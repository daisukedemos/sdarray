#ifndef RSDIC_HPP__
#define RSDIC_HPP__

#include "SDArray.hpp"

namespace SDS{

class RSDic{
  static const uint64_t BLOCKSIZE;
  
public:
  RSDic();
  ~RSDic();

  void build(const std::vector<uint64_t>& bv);
  void build(const std::vector<uint64_t>& bv, size_t size);
  uint64_t rank(uint64_t pos, bool bit) const;
  uint64_t select(uint64_t num) const;
  size_t size() const;
  size_t allocSize() const;

private:
  SDArray sda_;

};

}


#endif // RSDIC_HPP__
