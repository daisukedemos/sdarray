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

#ifndef VCODE_HPP__
#define VCODE_HPP__

#include <vector>
#include <stdint.h>
#include <iostream>

namespace SDS{

class SDArray{
  static const uint64_t BEGPOS_WIDTH;
  static const uint64_t BEGPOS_MASK;
  static const uint64_t BLOCK_SIZE;

public:
  enum {
    NOTFOUND = 0xFFFFFFFFFFFFFFFFLLU
  };

  SDArray();
  ~SDArray();

  /*
   * Add new item to the last
   */
  void add(uint64_t val);

  /*
   * Build an index. This build should be called before prefixSum(), prefixSumLookup(), and find().
   */
  void build();

  void clear();
  
  /*
   * @ret vals_[0]+vals_[1]+...+vals_[pos-1]
   */
  uint64_t prefixSum(uint64_t pos) const;
  
  /*
   * @ret vals_[0]+vals_[1]+...+vals_[pos-1] and set vals_[pos] to val 
   */
  uint64_t prefixSumLookup(uint64_t pos, uint64_t& val) const;  

  /*
   * @ret i s.t. vals_[0]+...+vals_[i] <= val < vals_[0]+...+vals_[i]+vals_[i+1] 
   */
  uint64_t find(uint64_t val) const;

  size_t size() const;
  size_t allocSize() const;

  void save(std::ostream& os) const;
  void load(std::istream& is);

private:
  void packHighs(uint64_t begPos, uint64_t width);
  void packLows(uint64_t begPos, uint64_t width);

  uint64_t selectBlock(uint64_t rank, uint64_t header) const;
  uint64_t rankBlock(uint64_t val, uint64_t header) const;

  uint64_t getLow(uint64_t begPos, uint64_t num, uint64_t width) const;

  uint64_t getBit(uint64_t pos) const;
  uint64_t getBits(uint64_t pos, uint64_t num) const;
  static uint64_t log2(uint64_t x);
  static uint64_t select(uint64_t x, uint64_t r) ;
  static uint64_t popCount(uint64_t x);
  static uint64_t getBits(uint64_t x, uint64_t beg, uint64_t num);
  std::vector<uint64_t> Ltable_; 
  std::vector<uint64_t> B_;
  size_t size_;

  std::vector<uint64_t> vals_;
  uint64_t sum_;
};

}

#endif // VCODE_HPP__
