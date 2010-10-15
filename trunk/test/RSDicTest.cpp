#include <gtest/gtest.h>
#include "../src/RSDic.hpp"

using namespace std;
using namespace SDS;

TEST(rsdic, trivial){
  RSDic rds;
  vector<uint64_t> bv;
  rds.build(bv);
  ASSERT_EQ(0, rds.size());
  ASSERT_EQ(0, rds.allocSize());
}

TEST(rsdic, zeros){
  const int N = 100000;
  RSDic rds;
  vector<uint64_t> bv((N + 63) / 64);
  rds.build(bv, N);

  //ASSERT_EQ(N, bv.size());
  
  for (int i = 0; i < N; ++i){
    ASSERT_EQ(i, rds.rank(i, 0));
    ASSERT_EQ(0, rds.rank(i, 1));
  }
}


TEST(rsdic, ones){
  const int N = 100000;
  RSDic rds;
  vector<uint64_t> bv((N + 63) / 64, 0xFFFFFFFFFFFFFFFFLLU);
  rds.build(bv, N);

  for (int i = 0; i < N; ++i){
    ASSERT_EQ(0, rds.rank(i, 0));
    ASSERT_EQ(i, rds.rank(i, 1));
  }
}


