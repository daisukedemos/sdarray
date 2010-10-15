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

#include <gtest/gtest.h>
#include <algorithm>
#include "../src/SDArray.hpp"

using namespace std;
using namespace SDS;

TEST(sdaode, trivial){
  SDArray sda;
  sda.build();
  ASSERT_EQ(0, sda.size());
  ASSERT_EQ(0, sda.allocSize());
  
  sda.build(); // again
  ASSERT_EQ(0, sda.size());
  ASSERT_EQ(0, sda.allocSize());
}

TEST(sdaode, zeros){
  SDArray sda;
  int N = 10000;
  for (int i = 0; i < N; ++i){
    sda.add(0);
  }
  sda.build();

  ASSERT_EQ(N, sda.size());
  for (int i = 0; i < N; ++i){

    ASSERT_EQ(0, sda.prefixSum(i));
    uint64_t val = 0;
    ASSERT_EQ(0, sda.prefixSumLookup(i, val));
    ASSERT_EQ(0, val);
  }
  
  ASSERT_EQ(SDArray::NOTFOUND, sda.find(0));
}

TEST(sdaode, ones){
  SDArray sda;
  int N = 10000;
  for (int i = 0; i < N; ++i){
    sda.add(1);
  }
  sda.build();

  ASSERT_EQ(N, sda.size());
  for (int i = 0; i < N; ++i){
    ASSERT_EQ(i, sda.prefixSum(i));
    uint64_t val = 0;    
    ASSERT_EQ(i, sda.prefixSumLookup(i, val));
    ASSERT_EQ(1, val);
  }

  for (int i = 0; i < N; ++i){
    ASSERT_EQ(i, sda.find(i));
  }
}

TEST(sdaode, increasing){
  SDArray sda;
  uint64_t N = 1000;
  vector<uint64_t> vals(N);
  vector<uint64_t> cums(N+1);
  uint64_t sum = 0;
  for (uint64_t i = 0; i < N; ++i){
    cums[i] = sum;
    sda.add(i);
    sum += i;
  }
  cums[N] = sum;
  sda.build();

  ASSERT_EQ(N, sda.size());
  for (uint64_t i = 0; i < N; ++i){
    ASSERT_EQ(cums[i], sda.prefixSum(i));
    uint64_t val = 0;
    ASSERT_EQ(cums[i], sda.prefixSumLookup(i, val));
    ASSERT_EQ(i, val);
  }

  for (uint64_t i = 0 ;i < sum; i += 17){
    vector<uint64_t>::iterator it = upper_bound(cums.begin(), cums.end(), i);
    size_t ind = it - cums.begin() - 1;

    ASSERT_GE(i, cums[ind]);
    if (ind+1 < cums.size()){
      ASSERT_LE(i, cums[ind+1]);
    }
    
    ASSERT_EQ(ind, sda.find(i));
  }
}



TEST(sdaode, random){
  SDArray sda;
  uint64_t N = 1000;
  vector<uint64_t> vals(N);
  vector<uint64_t> cums(N+1);
  uint64_t sum = 0;
  for (uint64_t i = 0; i < N; ++i){
    cums[i] = sum;
    vals[i] = rand();
    sda.add(vals[i]);
    sum += vals[i];
  }
  cums[N] = sum;
  sda.build();

  ASSERT_EQ(N, sda.size());
  sum = 0;
  for (uint64_t i = 0; i < N; ++i){
    ASSERT_EQ(sum, sda.prefixSum(i));
    uint64_t val = 0;
    ASSERT_EQ(sum, sda.prefixSumLookup(i, val));
    ASSERT_EQ(vals[i], val);
    sum += vals[i];
  }

  uint64_t M = 100;
  for (uint64_t i = 0; i < M; ++i){
    uint64_t val = rand() % sum;

    vector<uint64_t>::iterator it = upper_bound(cums.begin(), cums.end(), val);
    size_t ind = it - cums.begin() - 1;

    ASSERT_GE(val, cums[ind]);
    if (ind+1 < cums.size()){
      ASSERT_LE(val, cums[ind+1]);
    }
    
    ASSERT_EQ(ind, sda.find(val));
  }
}

