/*
 * Copyright 2025 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VERBSMARKS_MEMORY_BLOCK_H_
#define VERBSMARKS_MEMORY_BLOCK_H_

#include <cstddef>
#include <cstdint>

#include "absl/status/statusor.h"

namespace verbsmarks {

class MemoryBlock {
 public:
  MemoryBlock() : MemoryBlock(nullptr, 0) {}

  // If numa_node is not provided, memory is allocated using mmap but not bound
  // to a specific NUMA node.
  static absl::StatusOr<MemoryBlock> Create(size_t size, int numa_node = -1);

  MemoryBlock(MemoryBlock&& other) noexcept;

  MemoryBlock& operator=(MemoryBlock&& other) noexcept;

  ~MemoryBlock();

  MemoryBlock(const MemoryBlock&) = delete;
  MemoryBlock& operator=(const MemoryBlock&) = delete;

  void* Data() const { return ptr_; }
  size_t Size() const { return size_; }
  uint8_t* DataAsUint8() const { return static_cast<uint8_t*>(ptr_); }

 private:
  MemoryBlock(void* ptr, size_t size);

  void* ptr_ = nullptr;
  size_t size_ = 0;
};

}  // namespace verbsmarks

#endif  // VERBSMARKS_MEMORY_BLOCK_H_
