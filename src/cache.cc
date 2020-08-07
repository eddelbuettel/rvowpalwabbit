/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include <cstring>		// bzero()
#include "cache.h"
#include "unique_sort.h"

#include <Rcpp.h>
#define VWCOUT Rcpp::Rcout

using namespace std;

size_t neg_1 = 1;
size_t general = 2;

char* run_len_decode(char *p, size_t& i)
{// read an int 7 bits at a time.
  size_t count = 0;
  while(*p & 128)\
    i = i | ((*(p++) & 127) << 7*count++);
  i = i | (*(p++) << 7*count);
  return p;
}

size_t invocations = 0;

inline int32_t ZigZagDecode(uint32_t n) { return (n >> 1) ^ -static_cast<int32_t>(n & 1); }

size_t read_cached_tag(io_buf& cache, example* ae)
{
  char* c;
  size_t tag_size;
  if (buf_read(cache, c, sizeof(tag_size)) < sizeof(tag_size))
    return 0;
  memcpy((void*) &tag_size, (const void*)c, sizeof(size_t));  //tag_size = *(size_t*)c;
  c += sizeof(tag_size);
  cache.set(c);
  if (buf_read(cache, c, tag_size) < tag_size) 
    return 0;
  
  ae->tag.erase();
  push_many(ae->tag, c, tag_size);
  return tag_size+sizeof(tag_size);
}

struct one_float {
  float f;
} __attribute__((packed));

int read_cached_features(parser* p, void* ec)
{
  example* ae = (example*)ec;
  ae->sorted = p->sorted_cache;
  size_t mask = global.mask;
  io_buf* input = p->input;

  size_t total = p->lp->read_cached_label(ae->ld, *input);
  if (total == 0)
    return 0;
  if (read_cached_tag(*input,ae) == 0)
    return 0;

  char* c;
  unsigned char num_indices = 0;
  if (buf_read(*input, c, sizeof(num_indices)) < sizeof(num_indices)) 
    return 0;
  num_indices = *(unsigned char*)c;
  c += sizeof(num_indices);

  p->input->set(c);

  for (;num_indices > 0; num_indices--)
    {
      size_t temp;
      unsigned char index = 0;
      if((temp = buf_read(*input,c,sizeof(index) + sizeof(size_t))) < sizeof(index) + sizeof(size_t)) {
	VWCOUT << "truncated example! " << temp << " " << char_size + sizeof(size_t) << endl;
	return 0;
      }

      index = *(unsigned char*)c;
      c+= sizeof(index);
      push(ae->indices, (size_t)index);
      v_array<feature>* ours = ae->atomics+index;
      float* our_sum_feat_sq = ae->sum_feat_sq+index;
      size_t storage;
      memcpy((void*) &storage, (const void*) c, sizeof(size_t));//  size_t storage = *(size_t *)c;
      c += sizeof(size_t);
      p->input->set(c);
      total += storage; 
     if (buf_read(*input,c,storage) < storage) {
	VWCOUT << "truncated example! wanted: " << storage << " bytes" << endl;
	return 0;
      }

      char *end = c+storage;

      size_t last = 0;
      
      for (;c!= end;)
	{	  
	  feature f = {1., 0};
	  size_t temp = f.weight_index;
	  c = run_len_decode(c,temp);
	  f.weight_index = temp;
	  if (f.weight_index & neg_1) 
	    f.x = -1.;
	  else if (f.weight_index & general)	    {
	      f.x = ((one_float *)c)->f;
	      c += sizeof(float);
	    }
	  *our_sum_feat_sq += f.x*f.x;
          size_t diff = f.weight_index >> 2;

          int32_t s_diff = ZigZagDecode(diff);
	  if (s_diff < 0)
	    ae->sorted = false;
	  f.weight_index = last + s_diff;
	  last = f.weight_index;
	  f.weight_index = f.weight_index & mask;
	  push(*ours, f);
	}
      p->input->set(c);
    }

  return total;
}

char* run_len_encode(char *p, size_t i)
{// store an int 7 bits at a time.
  while (i >= 128)
    {
      *(p++) = (i & 127) | 128;
      i = i >> 7;
    }
  *(p++) = (i & 127);
  return p;
}

inline uint32_t ZigZagEncode(uint32_t n) {
  uint32_t ret = (n << 1) ^ (n >> 31);
  return ret;
}

void output_byte(io_buf& cache, unsigned char s)
{
  char *c;
  
  buf_write(cache, c, 1);
  *(c++) = s;
  cache.set(c);
}

void output_features(io_buf& cache, unsigned char index, feature* begin, feature* end)
{
  char* c;
  
  size_t storage = (end-begin) * int_size;
  for (feature* i = begin; i != end; i++)
    if (i->x != 1. && i->x != -1.)
      storage+=sizeof(float);
  
  buf_write(cache, c, sizeof(index) + storage + sizeof(size_t));
  *(unsigned char*)c = index;
  c += sizeof(index);

  char *storage_size_loc = c;
  c += sizeof(size_t);
  
  size_t last = 0;
  
  for (feature* i = begin; i != end; i++)
    {
      uint32_t s_diff = (i->weight_index - last);
      size_t diff = ZigZagEncode(s_diff) << 2;
      last = i->weight_index;
      if (i->x == 1.) 
	c = run_len_encode(c, diff);
      else if (i->x == -1.) 
	c = run_len_encode(c, diff | neg_1);
      else {
	c = run_len_encode(c, diff | general);
	memcpy((void*) c, (const void*) &(i->x), sizeof(float));//  *(float *)c = i->x;
	c += sizeof(float);
      }
    }
  cache.set(c);
  size_t tt = c - storage_size_loc - sizeof(size_t); memcpy((void*) &storage_size_loc, (const void*) &tt, sizeof(size_t));//  *(size_t*)storage_size_loc = c - storage_size_loc - sizeof(size_t);
}

void cache_tag(io_buf& cache, v_array<char> tag)
{
  char *c;
  buf_write(cache, c, sizeof(size_t)+tag.index());
  size_t tt = tag.index(); memcpy((void*) c, (const void*) &tt, sizeof(size_t));//  *(size_t*)c = tag.index();
  c += sizeof(size_t);
  if (tag.begin != nullptr) memcpy((void*) c, (const void*) tag.begin, tag.index());
  c += tag.index();
  cache.set(c);
}

void cache_features(io_buf& cache, example* ae)
{
  cache_tag(cache,ae->tag);
  output_byte(cache, ae->indices.index());
  for (size_t* b = ae->indices.begin; b != ae->indices.end; b++)
    output_features(cache, *b, ae->atomics[*b].begin,ae->atomics[*b].end);
}
