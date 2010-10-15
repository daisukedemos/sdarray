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

#include <cassert>
#include "SDArray.hpp"

using namespace std;

namespace SDS{

const uint64_t SDArray::BLOCK_SIZE = 64;

uint64_t SDArray::log2(uint64_t x){
  uint64_t r = 0;
  while (x >> r){
    r++;
  }
  return r;
} 

uint64_t SDArray::select(uint64_t x, uint64_t r){
  assert(r != 0);
  uint64_t x1 = x - ((x & 0xAAAAAAAAAAAAAAAALLU) >> 1);
  uint64_t x2 = (x1 & 0x3333333333333333LLU) + ((x1 >> 2) & 0x3333333333333333LLU);
  uint64_t x3 = (x2 + (x2 >> 4)) & 0x0F0F0F0F0F0F0F0FLLU;
  
  uint64_t pos = 0;
  for (;;  pos += 8){
    uint64_t b = (x3 >> pos) & 0xFFLLU;
    if (r <= b) break;
    r -= b;
  }

  uint64_t v2 = (x2 >> pos) & 0xFLLU;
  if (r > v2) {
    r -= v2;
    pos += 4;
  }

  uint64_t v1 = (x1 >> pos) & 0x3LLU;
  if (r > v1){
    r -= v1;
    pos += 2;
  }

  uint64_t v0  = (x >> pos) & 0x1LLU;
  if (v0 < r){
    r -= v0;
    pos += 1;
  }

  return pos;
}

uint64_t SDArray::popCount(uint64_t x) {
  x = x - ((x & 0xAAAAAAAAAAAAAAAALLU) >> 1);
  x = (x & 0x3333333333333333LLU) + ((x >> 2) & 0x3333333333333333LLU);
  x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FLLU;
  return ((x * 0x0101010101010101LLU) >> 56) & 0x7FLLU;
}

uint64_t SDArray::getBits(uint64_t x, uint64_t beg, uint64_t num){
  return (x >> beg) & ((1LLU << num) - 1);
}

SDArray::SDArray() : size_(0), sum_(0){
}

SDArray::~SDArray(){
}

void SDArray::add(uint64_t val){
  vals_.push_back(val);
  size_++;
  if (vals_.size() == BLOCK_SIZE) {
    build();
  }
}

void SDArray::build(){
  assert(vals_.size() <= BLOCK_SIZE); 
  if (vals_.size() == 0) return;

  for (size_t i = 1; i < vals_.size(); ++i){
    vals_[i] += vals_[i-1];
  }

  uint64_t begPos  = B_.size(); 

  Ltable_.push_back(sum_);
  sum_ += vals_.back();

  // header 
  // |-- begPos   (48) --|
  // |-- allZero  ( 1) --|
  // |-- width    ( 7) --|
  // |-- firstSum ( 8) --|
  assert(begPos < 1LLU << 48); 
  uint64_t header = (uint64_t)begPos; // use first 48 bit only

  if (vals_.back() == 0){
    header |= (1LLU << 48);
  } else {
    uint64_t width = log2(vals_.back() / vals_.size());    
    assert(width < (1LLU << 7));

    // All zero special case
    B_.resize(begPos + 2 + (vals_.size() * width + 63) / 64);
    packHighs(begPos, width);
    packLows(begPos, width);

    header |= (width << 49);
    uint64_t firstSum_ = popCount(B_[begPos]);
    assert(firstSum_ < (1LLU << 8));
    
    header |= firstSum_ << 56;
  }
  Ltable_.push_back(header);
  vals_.clear();
}

void SDArray::clear(){
  vector<uint64_t>().swap(Ltable_);
  vector<uint64_t>().swap(B_);
  size_ = 0;
  sum_  = 0;
}

uint64_t SDArray::prefixSum(const uint64_t pos) const {
  uint64_t bpos   = pos / BLOCK_SIZE;
  uint64_t offset = pos % BLOCK_SIZE;
  uint64_t sum    = Ltable_[bpos * 2];
  if (offset == 0) {
    return sum;
  }

  return sum + selectBlock(offset, Ltable_[bpos * 2 + 1]);
}

uint64_t SDArray::prefixSumLookup(const uint64_t pos, uint64_t& val) const {
  uint64_t bpos   = pos / BLOCK_SIZE;
  uint64_t offset = pos % BLOCK_SIZE;
  uint64_t sum    = Ltable_[bpos * 2];
  uint64_t prev   = 0;
  if (offset == 0) {
    prev = 0;
  } else {
    prev = selectBlock(offset, Ltable_[bpos * 2 + 1]);
  }
  uint64_t cur = selectBlock(offset+1, Ltable_[bpos * 2 + 1]);
  val = cur - prev;
  return sum + prev;
}

uint64_t SDArray::find(const uint64_t val) const{
  if (sum_ <= val) {
    cout << "come0" << endl;
    return NOTFOUND;
  }
  uint64_t low  = 0;
  uint64_t high = Ltable_.size() / 2;
  while (low < high){
    uint64_t dif = high - low;
    uint64_t mid = low + dif/2;
    if (val < Ltable_[mid*2]){
      high = mid;
    } else {
      low = mid+1;
    }
  }
  if (low*2 > Ltable_.size()) {
    cout << "come1" << endl;
    return NOTFOUND;
  }
  if (low == 0) return 0;
  uint64_t bpos = low-1;
  assert(Ltable_[bpos*2] <= val);
  if ((bpos+1)*2 < Ltable_.size()){
    assert(val < Ltable_[(bpos+1)*2]);
  }
  
  return bpos * BLOCK_SIZE + rankBlock(val - Ltable_[bpos*2], Ltable_[bpos*2+1]);
} 

size_t SDArray::size() const {
  return size_;
}

size_t SDArray::allocSize() const{
  return sizeof(uint64_t) * (Ltable_.size() + B_.size());  
}

void SDArray::save(ostream& os) const{
  os.write((const char*)&size_, sizeof(size_));
  size_t Bsize = B_.size();
  os.write((const char*)&Bsize, sizeof(Bsize));
  os.write((const char*)&B_[0],  sizeof(B_[0])*B_.size());
  os.write((const char*)&Ltable_[0],  sizeof(Ltable_[0])*Ltable_.size());
}

void SDArray::load(istream& is) {
  clear();
  is.read((char*)&size_, sizeof(size_));
  size_t Bsize = 0;
  is.read((char*)&Bsize, sizeof(Bsize));
  B_.resize(Bsize);
  is.read((char*)&B_[0],  sizeof(B_[0])*B_.size());
  size_t Lsize = (size_ + BLOCK_SIZE - 1) / BLOCK_SIZE * 2;
  Ltable_.resize(Lsize);
  is.read((char*)Ltable_[0], sizeof(Ltable_[0]) * Ltable_.size());
}

void SDArray::packHighs(uint64_t begPos, uint64_t width){
  for (size_t i = 0; i < vals_.size(); ++i){
    uint64_t pos    = (vals_[i] >> width) + i;
    B_[begPos + (pos / BLOCK_SIZE)] |= (1LLU << (pos % BLOCK_SIZE));
  }
}

void SDArray::packLows(uint64_t begPos, uint64_t width){
  if (width == 0) return;
  begPos += 2;
  uint64_t mask   = (1LLU << width) - 1;
  for (size_t i = 0; i < vals_.size(); ++i){
    uint64_t val    = vals_[i] & mask;
    uint64_t pos    = i * width;
    uint64_t bpos   = pos / BLOCK_SIZE;
    uint64_t offset = pos % BLOCK_SIZE;
    B_[begPos + bpos] |= val << offset;
    if (offset + width > BLOCK_SIZE){
      B_[begPos + bpos + 1] |= (val >> (BLOCK_SIZE - offset));
    }
  }
}


uint64_t SDArray::selectBlock(const uint64_t offset, const uint64_t header) const {
  if (getBits(header, 48, 1)){
    // all zero
    return 0;
  } 
  uint64_t begPos   = getBits(header,  0, 48);
  uint64_t width    = getBits(header, 49,  7);
  uint64_t firstSum = getBits(header, 56,  8);
  
  uint64_t high = 0;
  if (offset <= firstSum) {
    high = (select(B_[begPos], offset) + 1 - offset) << width;
  } else {
    high = (select(B_[begPos+1], offset - firstSum) + 1 - offset + BLOCK_SIZE) << width;
  }

  return high + getLow(begPos, offset-1, width);
}

uint64_t SDArray::rankBlock(const uint64_t val, uint64_t header) const {
  if (getBits(header, 48, 1)){
    // all zero
    return BLOCK_SIZE-1; 
  } 
  uint64_t begPos      = getBits(header,  0, 48);
  uint64_t width       = getBits(header, 49,  7);
  uint64_t firstOneSum = getBits(header, 56,  8);

  uint64_t high = val >> width;
  uint64_t low  = getBits(val,  0, width);

  uint64_t firstZeroSum = BLOCK_SIZE - firstOneSum;
  uint64_t valNum = 0;
  uint64_t highPos = begPos * BLOCK_SIZE;
  if (high > firstZeroSum){
    valNum += firstOneSum;
    high -= firstZeroSum;
    highPos += BLOCK_SIZE;
  }
  if (high > 0){
    uint64_t skipNum = select(~B_[highPos / BLOCK_SIZE], high)+ 1;
    highPos += skipNum;
    assert(skipNum >= high);  
    valNum += skipNum - high;
  }

  for ( ; ;  highPos++, valNum++){
    if (highPos >= (begPos + 2) * BLOCK_SIZE){
      return valNum;
    }
    if (!getBit(highPos)){
      return valNum;
    }
    uint64_t cur = getLow(begPos, valNum, width);

    if (cur == low) {
      return valNum + 1;
    } else if (low < cur){
      return valNum;
    }
  } 
  return valNum;
}

uint64_t SDArray::getLow(uint64_t begPos, uint64_t num, uint64_t width) const{
  return getBits((begPos + 2) * BLOCK_SIZE + num * width, width);  
}

uint64_t SDArray::getBit(const uint64_t pos) const{
  return (B_[pos / BLOCK_SIZE] >> (pos % BLOCK_SIZE)) & 1LLU;
}

uint64_t SDArray::getBits(const uint64_t pos, const uint64_t num) const {
  uint64_t bpos   = pos / BLOCK_SIZE;
  uint64_t offset = pos % BLOCK_SIZE;
  uint64_t mask   = (1LLU << num) - 1;
  if (offset + num <= BLOCK_SIZE){
    return (B_[bpos] >> pos) & mask;
  } else {
    return ((B_[bpos] >> pos) + (B_[bpos + 1] << (BLOCK_SIZE - offset))) & mask; 
  }
}

}
