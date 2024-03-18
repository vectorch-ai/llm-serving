#pragma once
#include "block.h"
#include <glog/logging.h>

#include <cstdint>
#include <vector>

namespace llm {

// BlockAllocator is used to track memory blocks. It is not thread safe.
// Please note: The actual memory has been allocated outside of this class. 
// This class only manages the allocation and deallocation of block ids.
class BlockAllocator final {
 public:
  // block_size: number of slots per block
  BlockAllocator(uint32_t total_blocks, uint32_t block_size);

  // allocate a list of blocks
  std::vector<Block> allocate(uint32_t n_blocks);

  // allocate a block
  Block allocate();

  // get number of slots per block
  size_t block_size() const { return block_size_; }

  // get number of free blocks
  size_t free_block_count() const { return free_block_count_; }

  // N.B. should not be used by the user. Only Block should call this method.
  void free(int32_t block_id);
 private:
  // free block count
  size_t free_block_count_ = 0;

  // number of slots per block
  size_t block_size_ = 0;

  // free block list
  std::vector<int32_t> free_blocks_;
};

}  // namespace llm
