#include <cstring>		// bzero()
#include <pthread.h>
#include <stdio.h>
#include <float.h>
#include "global_data.h"
#include "multisource.h"
#include "message_relay.h"

#include <Rcpp.h>
#define VWCOUT Rcpp::Rcout

using namespace std;

global_data global;
string version = "6.0";

pthread_mutex_t output_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t output_done = PTHREAD_COND_INITIALIZER;

void binary_print_result(int f, float res, float weight, v_array<char> tag)
{
  if (f >= 0)
    {
      global_prediction ps = {res, weight};
      send_global_prediction(f, ps);
    }
}

void print_result(int f, float res, float weight, v_array<char> tag)
{
  if (f >= 0)
    {
      char temp[30];
      int num = snprintf(temp, 30, "%f", res);
      ssize_t t;
      t = write(f, temp, num);
      if (t != num) 
	VWCOUT << "write error" << endl;
      if (tag.begin != tag.end){
	temp[0] = ' ';
	t = write(f, temp, 1);
	if (t != 1)
	  VWCOUT << "write error" << endl;
	t = write(f, tag.begin, sizeof(char)*tag.index());
	if (t != (ssize_t) (sizeof(char)*tag.index()))
	  VWCOUT << "write error" << endl;
      }
      if(global.active && weight >= 0)
	{
	  num = snprintf(temp, 30, " %f", weight);
	  t = write(f, temp, num);
	  if (t != num)
	    VWCOUT << "write error" << endl;
	}
      temp[0] = '\n';
      t = write(f, temp, 1);     
      if (t != 1) 
	VWCOUT << "write error" << endl;
    }
}

void print_lda_result(int f, float* res, float weight, v_array<char> tag)
{
  if (f >= 0)
    {
      char temp[30];
      ssize_t t;
      int num;
      for (size_t k = 0; k < global.lda; k++)
	{
	  num = snprintf(temp, 30, "%f ", res[k]);
	  t = write(f, temp, num);
	  if (t != num)
	    VWCOUT << "write error" << endl;
	}
      if (tag.begin != tag.end){
	temp[0] = ' ';
	t = write(f, temp, 1);
	if (t != 1)
	  VWCOUT << "write error" << endl;
	t = write(f, tag.begin, sizeof(char)*tag.index());
	if (t != (ssize_t) (sizeof(char)*tag.index()))
	  VWCOUT << "write error" << endl;
      }
      if(global.active && weight >= 0)
	{
	  num = snprintf(temp, 30, " %f", weight);
	  t = write(f, temp, num);
	  if (t != num)
	    VWCOUT << "write error" << endl;
	}
      temp[0] = '\n';
      t = write(f, temp, 1);
      if (t != 1)
	VWCOUT << "write error" << endl;
    }
}

void set_mm(double label)
{
  global.min_label = min(global.min_label, label);
  if (label != FLT_MAX)
    global.max_label = max(global.max_label, label);
}

void noop_mm(double label)
{}

void (*set_minmax)(double label) = set_mm;

