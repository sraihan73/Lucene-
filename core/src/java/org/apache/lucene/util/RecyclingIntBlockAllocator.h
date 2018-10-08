#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/Counter.h"

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::util
{

using Allocator = org::apache::lucene::util::IntBlockPool::Allocator;

/**
 * A {@link Allocator} implementation that recycles unused int
 * blocks in a buffer and reuses them in subsequent calls to
 * {@link #getIntBlock()}.
 * <p>
 * Note: This class is not thread-safe
 * </p>
 * @lucene.internal
 */
class RecyclingIntBlockAllocator final : public Allocator
{
  GET_CLASS_NAME(RecyclingIntBlockAllocator)
private:
  std::deque<std::deque<int>> freeByteBlocks;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int maxBufferedBlocks_;
  // C++ NOTE: Fields cannot have the same name as methods:
  int freeBlocks_ = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<Counter> bytesUsed_;

public:
  static constexpr int DEFAULT_BUFFERED_BLOCKS = 64;

  /**
   * Creates a new {@link RecyclingIntBlockAllocator}
   *
   * @param blockSize
   *          the block size in bytes
   * @param maxBufferedBlocks
   *          maximum number of buffered int block
   * @param bytesUsed
   *          {@link Counter} reference counting internally allocated bytes
   */
  RecyclingIntBlockAllocator(int blockSize, int maxBufferedBlocks,
                             std::shared_ptr<Counter> bytesUsed);

  /**
   * Creates a new {@link RecyclingIntBlockAllocator}.
   *
   * @param blockSize
   *          the size of each block returned by this allocator
   * @param maxBufferedBlocks
   *          maximum number of buffered int blocks
   */
  RecyclingIntBlockAllocator(int blockSize, int maxBufferedBlocks);

  /**
   * Creates a new {@link RecyclingIntBlockAllocator} with a block size of
   * {@link IntBlockPool#INT_BLOCK_SIZE}, upper buffered docs limit of
   * {@link #DEFAULT_BUFFERED_BLOCKS} ({@value #DEFAULT_BUFFERED_BLOCKS}).
   *
   */
  RecyclingIntBlockAllocator();

  std::deque<int> getIntBlock() override;

  void recycleIntBlocks(std::deque<std::deque<int>> &blocks, int start,
                        int end) override;

  /**
   * @return the number of currently buffered blocks
   */
  int numBufferedBlocks();

  /**
   * @return the number of bytes currently allocated by this {@link Allocator}
   */
  int64_t bytesUsed();

  /**
   * @return the maximum number of buffered byte blocks
   */
  int maxBufferedBlocks();

  /**
   * Removes the given number of int blocks from the buffer if possible.
   *
   * @param num
   *          the number of int blocks to remove
   * @return the number of actually removed buffers
   */
  int freeBlocks(int num);

protected:
  std::shared_ptr<RecyclingIntBlockAllocator> shared_from_this()
  {
    return std::static_pointer_cast<RecyclingIntBlockAllocator>(
        org.apache.lucene.util.IntBlockPool.Allocator::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/util/
