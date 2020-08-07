#include <cstring>		// bzero()
#include <float.h>

#include <Rcpp.h>
#define VWCOUT Rcpp::Rcout

#include "simple_label.h"
#include "cache.h"

using namespace std;

char* bufread_simple_label(label_data* ld, char* c)
{
  memcpy((void*) &ld->label, (const void*) c, sizeof(float));//  ld->label = *(float *)c;
  c += sizeof(ld->label);
  memcpy((void*) &ld->weight, (const void*) c, sizeof(float));//  ld->weight = *(float *)c;
  c += sizeof(ld->weight);
  memcpy((void*) &ld->initial, (const void*) c, sizeof(float));//  ld->initial = *(float *)c;
  c += sizeof(ld->initial);
  return c;
}

size_t read_cached_simple_label(void* v, io_buf& cache)
{
  label_data* ld = (label_data*) v;
  char *c;
  size_t total = sizeof(ld->label)+sizeof(ld->weight)+sizeof(ld->initial);
  if (buf_read(cache, c, total) < total) 
    return 0;
  c = bufread_simple_label(ld,c);

  return total;
}

float get_weight(void* v)
{
  label_data* ld = (label_data*) v;
  return ld->weight;
}

float get_initial(void* v)
{
  label_data* ld = (label_data*) v;
  return ld->initial;
}

char* bufcache_simple_label(label_data* ld, char* c)
{
  memcpy((void*) c, (const void*) &ld->label, sizeof(float));//  *(float *)c = ld->label;
  c += sizeof(ld->label);
  memcpy((void*) c, (const void*) &ld->weight, sizeof(float));//  *(float *)c = ld->weight;
  c += sizeof(ld->weight);
  memcpy((void*) c, (const void*) &ld->initial, sizeof(float));//  *(float *)c = ld->initial;
  c += sizeof(ld->initial);
  return c;
}

void cache_simple_label(void* v, io_buf& cache)
{
  char *c;
  label_data* ld = (label_data*) v;
  buf_write(cache, c, sizeof(ld->label)+sizeof(ld->weight)+sizeof(ld->initial));
  c = bufcache_simple_label(ld,c);
}

void default_simple_label(void* v)
{
  label_data* ld = (label_data*) v;
  ld->label = FLT_MAX;
  ld->weight = 1.;
  ld->initial = 0.;
}

void delete_simple_label(void* v)
{
}

void parse_simple_label(void* v, v_array<substring>& words)
{
  label_data* ld = (label_data*)v;

  switch(words.index()) {
  case 0:
    break;
  case 1:
    ld->label = float_of_substring(words[0]);
    break;
  case 2:
    ld->label = float_of_substring(words[0]);
    ld->weight = float_of_substring(words[1]);
    break;
  case 3:
    ld->label = float_of_substring(words[0]);
    ld->weight = float_of_substring(words[1]);
    ld->initial = float_of_substring(words[2]);
    break;
  default:
    VWCOUT << "malformed example!\n";
    VWCOUT << "words.index() = " << words.index() << endl;
  }
}

