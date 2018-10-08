#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class ByteBufferGuard;
}

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
namespace org::apache::lucene::store
{

/**
 * Base IndexInput implementation that uses an array
 * of ByteBuffers to represent a file.
 * <p>
 * Because Java's ByteBuffer uses an int to address the
 * values, it's necessary to access a file greater
 * Integer.MAX_VALUE in size using multiple byte buffers.
 * <p>
 * For efficiency, this class requires that the buffers
 * are a power-of-two (<code>chunkSizePower</code>).
 */
class ByteBufferIndexInput : public IndexInput, public RandomAccessInput
{
  GET_CLASS_NAME(ByteBufferIndexInput)
protected:
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t length_;
  const int64_t chunkSizeMask;
  const int chunkSizePower;
  const std::shared_ptr<ByteBufferGuard> guard;

  std::deque<std::shared_ptr<ByteBuffer>> buffers;
  int curBufIndex = -1;
  std::shared_ptr<ByteBuffer>
      curBuf; // redundant for speed: buffers[curBufIndex]

  bool isClone = false;

public:
  static std::shared_ptr<ByteBufferIndexInput>
  newInstance(const std::wstring &resourceDescription,
              std::deque<std::shared_ptr<ByteBuffer>> &buffers,
              int64_t length, int chunkSizePower,
              std::shared_ptr<ByteBufferGuard> guard);

  ByteBufferIndexInput(const std::wstring &resourceDescription,
                       std::deque<std::shared_ptr<ByteBuffer>> &buffers,
                       int64_t length, int chunkSizePower,
                       std::shared_ptr<ByteBufferGuard> guard);

  char readByte()  override final;

  void readBytes(std::deque<char> &b, int offset,
                 int len)  override final;

  short readShort()  override final;

  int readInt()  override final;

  int64_t readLong()  override final;

  int64_t getFilePointer() override;

  void seek(int64_t pos)  override;

  char readByte(int64_t pos)  override;

  // used only by random access methods to handle reads across boundaries
private:
  void setPos(int64_t pos, int bi) ;

public:
  short readShort(int64_t pos)  override;

  int readInt(int64_t pos)  override;

  int64_t readLong(int64_t pos)  override;

  int64_t length() override final;

  std::shared_ptr<ByteBufferIndexInput> clone() override final;

  /**
   * Creates a slice of this index input, with the given description, offset,
   * and length. The slice is seeked to the beginning.
   */
  std::shared_ptr<ByteBufferIndexInput>
  slice(const std::wstring &sliceDescription, int64_t offset,
        int64_t length) override final;

  /** Builds the actual sliced IndexInput (may apply extra offset in
   * subclasses). **/
protected:
  virtual std::shared_ptr<ByteBufferIndexInput>
  buildSlice(const std::wstring &sliceDescription, int64_t offset,
             int64_t length);

  /** Factory method that creates a suitable implementation of this class for
   * the given ByteBuffers. */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("resource") protected ByteBufferIndexInput
  // newCloneInstance(std::wstring newResourceDescription, java.nio.ByteBuffer[]
  // newBuffers, int offset, long length)
  virtual std::shared_ptr<ByteBufferIndexInput>
  newCloneInstance(const std::wstring &newResourceDescription,
                   std::deque<std::shared_ptr<ByteBuffer>> &newBuffers,
                   int offset, int64_t length);

  /** Returns a sliced view from a set of already-existing buffers:
   *  the last buffer's limit() will be correct, but
   *  you must deal with offset separately (the first buffer will not be
   * adjusted) */
private:
  std::deque<std::shared_ptr<ByteBuffer>>
  buildSlice(std::deque<std::shared_ptr<ByteBuffer>> &buffers,
             int64_t offset, int64_t length);

public:
  ~ByteBufferIndexInput();

  /**
   * Called to remove all references to byte buffers, so we can throw
   * AlreadyClosed on NPE.
   */
private:
  void unsetBuffers();

  /** Optimization of ByteBufferIndexInput for when there is only one buffer */
public:
  class SingleBufferImpl;

  /** This class adds offset support to ByteBufferIndexInput, which is needed
   * for slices. */
public:
  class MultiBufferImpl;

protected:
  std::shared_ptr<ByteBufferIndexInput> shared_from_this()
  {
    return std::static_pointer_cast<ByteBufferIndexInput>(
        IndexInput::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
class ByteBufferIndexInput::SingleBufferImpl final : public ByteBufferIndexInput
{
  GET_CLASS_NAME(ByteBufferIndexInput::SingleBufferImpl)

public:
  SingleBufferImpl(const std::wstring &resourceDescription,
                   std::shared_ptr<ByteBuffer> buffer, int64_t length,
                   int chunkSizePower, std::shared_ptr<ByteBufferGuard> guard);

  // TODO: investigate optimizing readByte() & Co?

  void seek(int64_t pos)  override;

  int64_t getFilePointer() override;

  char readByte(int64_t pos)  override;

  short readShort(int64_t pos)  override;

  int readInt(int64_t pos)  override;

  int64_t readLong(int64_t pos)  override;

protected:
  std::shared_ptr<SingleBufferImpl> shared_from_this()
  {
    return std::static_pointer_cast<SingleBufferImpl>(
        ByteBufferIndexInput::shared_from_this());
  }
};
class ByteBufferIndexInput::MultiBufferImpl final : public ByteBufferIndexInput
{
  GET_CLASS_NAME(ByteBufferIndexInput::MultiBufferImpl)
private:
  const int offset;

public:
  MultiBufferImpl(const std::wstring &resourceDescription,
                  std::deque<std::shared_ptr<ByteBuffer>> &buffers, int offset,
                  int64_t length, int chunkSizePower,
                  std::shared_ptr<ByteBufferGuard> guard);

  void seek(int64_t pos)  override;

  int64_t getFilePointer() override;

  char readByte(int64_t pos)  override;

  short readShort(int64_t pos)  override;

  int readInt(int64_t pos)  override;

  int64_t readLong(int64_t pos)  override;

protected:
  std::shared_ptr<ByteBufferIndexInput>
  buildSlice(const std::wstring &sliceDescription, int64_t ofs,
             int64_t length) override;

protected:
  std::shared_ptr<MultiBufferImpl> shared_from_this()
  {
    return std::static_pointer_cast<MultiBufferImpl>(
        ByteBufferIndexInput::shared_from_this());
  }
};
