// Copyright 2025 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "memory_block.h"

#include <numaif.h>
#include <sys/mman.h>

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <ostream>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"

namespace verbsmarks {

constexpr int kBitsPerByte = 8;

absl::StatusOr<MemoryBlock> MemoryBlock::Create(size_t size, int numa_node) {
  if (size == 0) {
    return MemoryBlock(nullptr, 0);
  }

  // Validate NUMA node if provided.
  if (numa_node >= 0) {
    if (numa_node >= (sizeof(uint64_t) * kBitsPerByte)) {
      return absl::InvalidArgumentError(
          absl::StrCat("NUMA node ID ", numa_node, " is out of range."));
    }
  }

  // Allocate page-aligned memory using mmap.
  void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ptr == MAP_FAILED) {
    return absl::InternalError(absl::StrCat("mmap failed with errno: ", errno));
  }

  // Bind the memory to the specified NUMA node if provided.
  if (numa_node >= 0) {
    uint64_t nodemask_bits;
    std::cout << "MemoryBlock::Create on numa_node: " << numa_node << std::endl;
    nodemask_bits = 1ULL << numa_node;
    if (mbind(ptr, size, MPOL_BIND, &nodemask_bits,
              sizeof(nodemask_bits) * kBitsPerByte, 0) != 0) {
      int err = errno;
      munmap(ptr, size);  // Clean up on failure
      return absl::InternalError(absl::StrCat("mbind to node ", numa_node,
                                              " failed with errno: ", err));
    }
  }
  // If numa_node is not provided, the memory remains allocated via mmap
  // without specific NUMA affinity, allowing the OS to manage placement.

  return MemoryBlock(ptr, size);
}

MemoryBlock::MemoryBlock(void* ptr, size_t size) : ptr_(ptr), size_(size) {}

MemoryBlock::MemoryBlock(MemoryBlock&& other) noexcept
    : ptr_(other.ptr_), size_(other.size_) {
  other.ptr_ = nullptr;
  other.size_ = 0;
}

MemoryBlock& MemoryBlock::operator=(MemoryBlock&& other) noexcept {
  if (this != &other) {
    if (ptr_) {
      munmap(ptr_, size_);
    }
    ptr_ = other.ptr_;
    size_ = other.size_;
    other.ptr_ = nullptr;
    other.size_ = 0;
  }
  return *this;
}

MemoryBlock::~MemoryBlock() {
  if (ptr_) {
    munmap(ptr_, size_);
  }
}

}  // namespace verbsmarks
