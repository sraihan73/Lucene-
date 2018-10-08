#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

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

/**
 * A pool for int blocks similar to {@link ByteBlockPool}
 * @lucene.internal
 */
class IntBlockPool final : public std::enable_shared_from_this<IntBlockPool>
{
  GET_CLASS_NAME(IntBlockPool)
public:
  static constexpr int INT_BLOCK_SHIFT = 13;
  static const int INT_BLOCK_SIZE = 1 << INT_BLOCK_SHIFT;
  static const int INT_BLOCK_MASK = INT_BLOCK_SIZE - 1;

  /** Abstract class for allocating and freeing int
   *  blocks. */
public:
  class Allocator : public std::enable_shared_from_this<Allocator>
  {
    GET_CLASS_NAME(Allocator)
  protected:
    const int blockSize;

  public:
    Allocator(int blockSize);

    virtual void recycleIntBlocks(std::deque<std::deque<int>> &blocks,
                                  int start, int end) = 0;

    virtual std::deque<int> getIntBlock();
  };

  /** A simple {@link Allocator} that never recycles. */
public:
  class DirectAllocator final : public Allocator
  {
    GET_CLASS_NAME(DirectAllocator)

    /**
     * Creates a new {@link DirectAllocator} with a default block size
     */
  public:
    DirectAllocator();

    void recycleIntBlocks(std::deque<std::deque<int>> &blocks, int start,
                          int end) override;

  protected:
    std::shared_ptr<DirectAllocator> shared_from_this()
    {
      return std::static_pointer_cast<DirectAllocator>(
          Allocator::shared_from_this());
    }
  };

  /** array of buffers currently used in the pool. Buffers are allocated if
   * needed don't modify this outside of this class */
public:
  std::deque<std::deque<int>> buffers = std::deque<std::deque<int>>(10);

  /** index into the buffers array pointing to the current buffer used as the
   * head */
private:
  int bufferUpto = -1;
  /** Pointer to the current position in head buffer */
public:
  int intUpto = INT_BLOCK_SIZE;
  /** Current head buffer */
  std::deque<int> buffer;
  /** Current head offset */
  int intOffset = -INT_BLOCK_SIZE;

private:
  const std::shared_ptr<Allocator> allocator;

  /**
   * Creates a new {@link IntBlockPool} with a default {@link Allocator}.
   * @see IntBlockPool#nextBuffer()
   */
public:
  IntBlockPool();

  /**
   * Creates a new {@link IntBlockPool} with the given {@link Allocator}.
   * @see IntBlockPool#nextBuffer()
   */
  IntBlockPool(std::shared_ptr<Allocator> allocator);

  /**
   * Resets the pool to its initial state reusing the first buffer. Calling
   * {@link IntBlockPool#nextBuffer()} is not needed after reset.
   */
  void reset();

  /**
   * Expert: Resets the pool to its initial state reusing the first buffer.
   * @param zeroFillBuffers if <code>true</code> the buffers are filled with
   * <tt>0</tt>. This should be set to <code>true</code> if this pool is used
   * with
   *        {@link SliceWriter}.
   * @param reuseFirst if <code>true</code> the first buffer will be reused and
   * calling
   *        {@link IntBlockPool#nextBuffer()} is not needed after reset iff the
   *        block pool was used before ie. {@link IntBlockPool#nextBuffer()} was
   * called before.
   */
  void reset(bool zeroFillBuffers, bool reuseFirst);

  /**
   * Advances the pool to its next buffer. This method should be called once
   * after the constructor to initialize the pool. In contrast to the
   * constructor a {@link IntBlockPool#reset()} call will advance the pool to
   * its first buffer immediately.
   */
  void nextBuffer();

  /**
   * Creates a new int slice with the given starting size and returns the slices
   * offset in the pool.
   * @see SliceReader
   */
private:
  int newSlice(int const size);

  static bool assertSliceBuffer(std::deque<int> &buffer);

  // no need to make this public unless we support different sizes
  // TODO make the levels and the sizes configurable
  /**
   * An array holding the offset into the {@link IntBlockPool#LEVEL_SIZE_ARRAY}
   * to quickly navigate to the next slice level.
   */
  static std::deque<int> const NEXT_LEVEL_ARRAY;

  /**
   * An array holding the level sizes for int slices.
   */
  static std::deque<int> const LEVEL_SIZE_ARRAY;

  /**
   * The first level size for new slices
   */
  static const int FIRST_LEVEL_SIZE = LEVEL_SIZE_ARRAY[0];

  /**
   * Allocates a new slice from the given offset
   */
  int allocSlice(std::deque<int> &slice, int const sliceOffset);

  /**
   * A {@link SliceWriter} that allows to write multiple integer slices into a
   * given {@link IntBlockPool}.
   *
   *  @see SliceReader
   *  @lucene.internal
   */
public:
  class SliceWriter : public std::enable_shared_from_this<SliceWriter>
  {
    GET_CLASS_NAME(SliceWriter)

  private:
    int offset = 0;
    const std::shared_ptr<IntBlockPool> pool;

  public:
    SliceWriter(std::shared_ptr<IntBlockPool> pool);
    /**
     *
     */
    virtual void reset(int sliceOffset);

    /**
     * Writes the given value into the slice and resizes the slice if needed
     */
    virtual void writeInt(int value);

    /**
     * starts a new slice and returns the start offset. The returned value
     * should be used as the start offset to initialize a {@link SliceReader}.
     */
    virtual int startNewSlice();

    /**
     * Returns the offset of the currently written slice. The returned value
     * should be used as the end offset to initialize a {@link SliceReader} once
     * this slice is fully written or to reset the this writer if another slice
     * needs to be written.
     */
    virtual int getCurrentOffset();
  };

  /**
   * A {@link SliceReader} that can read int slices written by a {@link
   * SliceWriter}
   * @lucene.internal
   */
public:
  class SliceReader final : public std::enable_shared_from_this<SliceReader>
  {
    GET_CLASS_NAME(SliceReader)

  private:
    const std::shared_ptr<IntBlockPool> pool;
    int upto = 0;
    int bufferUpto = 0;
    int bufferOffset = 0;
    std::deque<int> buffer;
    int limit = 0;
    int level = 0;
    int end = 0;

    /**
     * Creates a new {@link SliceReader} on the given pool
     */
  public:
    SliceReader(std::shared_ptr<IntBlockPool> pool);

    /**
     * Resets the reader to a slice give the slices absolute start and end
     * offset in the pool
     */
    void reset(int startOffset, int endOffset);

    /**
     * Returns <code>true</code> iff the current slice is fully read. If this
     * method returns <code>true</code> {@link SliceReader#readInt()} should not
     * be called again on this slice.
     */
    bool endOfSlice();

    /**
     * Reads the next int from the current slice and returns it.
     * @see SliceReader#endOfSlice()
     */
    int readInt();

  private:
    void nextSlice();
  };
};

} // namespace org::apache::lucene::util
