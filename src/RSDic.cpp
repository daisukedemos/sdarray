#include "RSDic.hpp"

using namespace std;

namespace SDS{

const uint64_t RSDic::BLOCKSIZE = 64;

RSDic::RSDic(){
}

RSDic::~RSDic(){
}

void RSDic::build(const vector<uint64_t>& bv){
  build(bv, bv.size() * BLOCKSIZE);
}
  
void RSDic::build(const vector<uint64_t>& bv, size_t size){
  sda_.clear();
  uint64_t prev = SDArray::NOTFOUND;
  for (size_t i = 0; i < size; ++i){
    if ((bv[i / BLOCKSIZE] >> (i % BLOCKSIZE)) & 1LLU){
      if (prev == SDArray::NOTFOUND){
	sda_.add(i+1);
      } else {
	sda_.add(i - prev);
      }
      prev = i;
    }
  }
  sda_.build();
}
  
uint64_t RSDic::rank(uint64_t pos, const bool bit) const{
  uint64_t val = sda_.find(pos);
  if (val == SDArray::NOTFOUND) {
    val = 0;
  }
  if (bit) return val;
  else     return pos - val; 
}

uint64_t RSDic::select(uint64_t num) const{
  return sda_.prefixSum(num+1);
}

size_t RSDic::size() const{
  return sda_.size();
}

size_t RSDic::allocSize() const {
  return sda_.allocSize();
}

}
