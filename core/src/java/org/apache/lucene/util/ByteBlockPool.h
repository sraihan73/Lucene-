#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/Counter.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"

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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.RamUsageEstimator.NUM_BYTES_OBJECT_REF;

/**
 * Class that Posting and PostingVector use to write byte
 * streams into shared fixed-size byte[] arrays.  The idea
 * is to allocate slices of increasing lengths For
 * example, the first slice is 5 bytes, the next slice is
 * 14, etc.  We start by writing our bytes into the first
 * 5 bytes.  When we hit the end of the slice, we allocate
 * the next slice and then write the address of the new
 * slice into the last 4 bytes of the previous slice (the
 * "forwarding address").
 *
 * Each slice is filled with 0's initially, and we mark
 * the end with a non-zero byte.  This way the methods
 * that are writing into the slice don't need to record
 * its length and instead allocate a new slice once they
 * hit a non-zero byte.
 *
 * @lucene.internal
 **/
class ByteBlockPool final : public std::enable_shared_from_this<ByteBlockPool>
{
  GET_CLASS_NAME(ByteBlockPool)
public:
  static constexpr int BYTE_BLOCK_SHIFT = 15;
  static const int BYTE_BLOCK_SIZE = 1 << BYTE_BLOCK_SHIFT;
  static const int BYTE_BLOCK_MASK = BYTE_BLOCK_SIZE - 1;

  /** Abstract class for allocating and freeing byte
   *  blocks. */
public:
  class Allocator : public std::enable_shared_from_this<Allocator>
  {
    GET_CLASS_NAME(Allocator)
  protected:
    const int blockSize;

  public:
    Allocator(int blockSize);

    virtual void recycleByteBlocks(std::deque<std::deque<char>> &blocks,
                                   int start, int end) = 0;

    virtual void recycleByteBlocks(std::deque<std::deque<char>> &blocks);

    virtual std::deque<char> getByteBlock();
  };

  /** A simple {@link Allocator} that never recycles. */
public:
  class DirectAllocator final : public Allocator
  {
    GET_CLASS_NAME(DirectAllocator)

  public:
    DirectAllocator();

    DirectAllocator(int blockSize);

    void recycleByteBlocks(std::deque<std::deque<char>> &blocks, int start,
                           int end) override;

  protected:
    std::shared_ptr<DirectAllocator> shared_from_this()
    {
      return std::static_pointer_cast<DirectAllocator>(
          Allocator::shared_from_this());
    }
  };

  /** A simple {@link Allocator} that never recycles, but
   *  tracks how much total RAM is in use. */
public:
  class DirectTrackingAllocator : public Allocator
  {
    GET_CLASS_NAME(DirectTrackingAllocator)
  private:
    const std::shared_ptr<Counter> bytesUsed;

  public:
    DirectTrackingAllocator(std::shared_ptr<Counter> bytesUsed);

    DirectTrackingAllocator(int blockSize, std::shared_ptr<Counter> bytesUsed);

    std::deque<char> getByteBlock() override;

    void recycleByteBlocks(std::deque<std::deque<char>> &blocks, int start,
                           int end) override;

  protected:
    std::shared_ptr<DirectTrackingAllocator> shared_from_this()
    {
      return std::static_pointer_cast<DirectTrackingAllocator>(
          Allocator::shared_from_this());
    }
  };

  /**
   * array of buffers currently used in the pool. Buffers are allocated if
   * needed don't modify this outside of this class.
   */
public:
  std::deque<std::deque<char>> buffers = std::deque<std::deque<char>>(10);

  /** index into the buffers array pointing to the current buffer used as the
   * head */
private:
  int bufferUpto = -1; // Which buffer we are upto
                       /** Where we are in head buffer */
public:
  int byteUpto = BYTE_BLOCK_SIZE;

  /** Current head buffer */
  std::deque<char> buffer;
  /** Current head offset */
  int byteOffset = -BYTE_BLOCK_SIZE;

private:
  const std::shared_ptr<Allocator> allocator;

public:
  ByteBlockPool(std::shared_ptr<Allocator> allocator);

  /**
   * Resets the pool to its initial state reusing the first buffer and fills all
   * buffers with <tt>0</tt> bytes before they reused or passed to
   * {@link Allocator#recycleByteBlocks(byte[][], int, int)}. Calling
   * {@link ByteBlockPool#nextBuffer()} is not needed after reset.
   */
  void reset();

  /**
   * Expert: Resets the pool to its initial state reusing the first buffer.
   * Calling
   * {@link ByteBlockPool#nextBuffer()} is not needed after reset.
   * @param zeroFillBuffers if <code>true</code> the buffers are filled with
   * <tt>0</tt>. This should be set to <code>true</code> if this pool is used
   * with slices.
   * @param reuseFirst if <code>true</code> the first buffer will be reused and
   * calling
   *        {@link ByteBlockPool#nextBuffer()} is not needed after reset iff the
   *        block pool was used before ie. {@link ByteBlockPool#nextBuffer()}
   * was called before.
   */
  void reset(bool zeroFillBuffers, bool reuseFirst);

  /**
   * Advances the pool to its next buffer. This method should be called once
   * after the constructor to initialize the pool. In contrast to the
   * constructor a {@link ByteBlockPool#reset()} call will advance the pool to
   * its first buffer immediately.
   */
  void nextBuffer();

  /**
   * Allocates a new slice with the given size.
   * @see ByteBlockPool#FIRST_LEVEL_SIZE
   */
  int newSlice(int const size);

  // Size of each slice.  These arrays should be at most 16
  // elements (index is encoded with 4 bits).  First array
  // is just a compact way to encode X+1 with a max.  Second
  // array is the length of each slice, ie first slice is 5
  // bytes, next slice is 14 bytes, etc.

  /**
   * An array holding the offset into the {@link ByteBlockPool#LEVEL_SIZE_ARRAY}
   * to quickly navigate to the next slice level.
   */
  static std::deque<int> const NEXT_LEVEL_ARRAY;

  /**
   * An array holding the level sizes for byte slices.
   */
  static std::deque<int> const LEVEL_SIZE_ARRAY;

  /**
   * The first level size for new slices
   * @see ByteBlockPool#newSlice(int)
   */
  static const int FIRST_LEVEL_SIZE = LEVEL_SIZE_ARRAY[0];

  /**
   * Creates a new byte slice with the given starting size and
   * returns the slices offset in the pool.
   */
  int allocSlice(std::deque<char> &slice, int const upto);

  /** Fill the provided {@link BytesRef} with the bytes at the specified
   * offset/length slice. This will avoid copying the bytes, if the slice fits
   * into a single block; otherwise, it uses the provided {@link
   * BytesRefBuilder} to copy bytes over. */
  void setBytesRef(std::shared_ptr<BytesRefBuilder> builder,
                   std::shared_ptr<BytesRef> result, int64_t offset,
                   int length);

  // Fill in a BytesRef from term's length & bytes encoded in
  // byte block
  void setBytesRef(std::shared_ptr<BytesRef> term, int textStart);

  /**
   * Appends the bytes in the provided {@link BytesRef} at
   * the current position.
   */
  void append(std::shared_ptr<BytesRef> bytes);

  /**
   * Reads bytes bytes out of the pool starting at the given offset with the
   * given length into the given byte array at offset <tt>off</tt>. <p>Note:
   * this method allows to copy across block boundaries.</p>
   */
  void readBytes(int64_t const offset, std::deque<char> &bytes,
                 int bytesOffset, int bytesLength);

  /**
   * Set the given {@link BytesRef} so that its content is equal to the
   * {@code ref.length} bytes starting at {@code offset}. Most of the time this
   * method will set pointers to internal data-structures. However, in case a
   * value crosses a boundary, a fresh copy will be returned.
   * On the contrary to {@link #setBytesRef(BytesRef, int)}, this does not
   * expect the length to be encoded with the data.
   */
  void setRawBytesRef(std::shared_ptr<BytesRef> ref, int64_t const offset);

  /** Read a single byte at the given {@code offset}. */
  char readByte(int64_t offset);
};

} // #include  "core/src/java/org/apache/lucene/util/
