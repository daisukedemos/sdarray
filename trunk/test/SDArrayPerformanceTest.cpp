/* 
 *  Copyright (c) 2010 Daisuke Okanohara
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 * 
 *   1. Redistributions of source code must retain the above Copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above Copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 */

#include <iostream>
#include <cassert>
#include <stdlib.h>
#include <sys/time.h>
#include "../src/SDArray.hpp"

using namespace std;
using namespace SDS;

struct __bench__ {
  double start;
  __bench__(int dummy) {
    start = sec();
  }
  ~__bench__() {
    fprintf(stderr, ": %.6f sec\n", sec() - start);
  }
  double sec() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
  }
  operator bool() { return false; }
};

#define benchmark(...) \
  if (__bench__ __b__ = __bench__(fprintf(stderr, __VA_ARGS__))); else

int main(int argc, char* argv[]){
  const int N = 10000000;

  vector<uint64_t>  vals;
  SDArray sda;
  uint64_t sum = 0;
  for (int i = 0; i < N; ++i){
    uint64_t val = rand() % 100;
    vals.push_back(val);
    sum += val;
  }
  
  benchmark("build"){
    for (int i = 0; i < N; ++i){
      sda.add(vals[i]);
    }
    sda.build();
  }

  cout << sda.allocSize() << " " << (float)sda.allocSize() / N << endl;
  uint64_t dummy = 0;
  benchmark("prefixsum"){ 
    for (int i = 0; i < N; ++i){
      dummy += sda.prefixSum(i);
    }
  }

  benchmark("prefixsum_lookup"){ 
    for (int i = 0; i < N; ++i){
      uint64_t val = 0;
      dummy += sda.prefixSumLookup(i, val);
    }
  }

  benchmark("find"){ 
    for (int i = 0; i < N; ++i){
      uint64_t val = rand() % sum;
      dummy += sda.find(val);
    }
  }


  if (dummy == 0){
    cout << "luckey" << endl; 
  }
  return 0;
}
