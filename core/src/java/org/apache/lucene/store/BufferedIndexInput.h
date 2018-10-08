#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IOContext;
}

namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::store
{
class IndexOutput;
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

/** Base implementation class for buffered {@link IndexInput}. */
class BufferedIndexInput : public IndexInput, public RandomAccessInput
{

  /** Default buffer size set to {@value #BUFFER_SIZE}. */
public:
  static constexpr int BUFFER_SIZE = 1024;

  /** Minimum buffer size allowed */
  static constexpr int MIN_BUFFER_SIZE = 8;

  // The normal read buffer size defaults to 1024, but
  // increasing this during merging seems to yield
  // performance gains.  However we don't want to increase
  // it too much because there are quite a few
  // BufferedIndexInputs created during merging.  See
  // LUCENE-888 for details.
  /**
   * A buffer size for merges set to {@value #MERGE_BUFFER_SIZE}.
   */
  static constexpr int MERGE_BUFFER_SIZE = 4096;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  int bufferSize_ = BUFFER_SIZE;

protected:
  std::deque<char> buffer;

private:
  int64_t bufferStart = 0; // position in file of buffer
  int bufferLength = 0;      // end of valid bytes
  int bufferPosition = 0;    // next byte to read

public:
  char readByte()  override final;

  BufferedIndexInput(const std::wstring &resourceDesc);

  BufferedIndexInput(const std::wstring &resourceDesc,
                     std::shared_ptr<IOContext> context);

  /** Inits BufferedIndexInput with a specific bufferSize */
  BufferedIndexInput(const std::wstring &resourceDesc, int bufferSize);

  /** Change the buffer size used by this IndexInput */
  void setBufferSize(int newSize);

protected:
  virtual void newBuffer(std::deque<char> &newBuffer);

  /** Returns buffer size.  @see #setBufferSize */
public:
  int getBufferSize();

private:
  void checkBufferSize(int bufferSize);

public:
  void readBytes(std::deque<char> &b, int offset,
                 int len)  override final;

  void readBytes(std::deque<char> &b, int offset, int len,
                 bool useBuffer)  override final;

  short readShort()  override final;

  int readInt()  override final;

  int64_t readLong()  override final;

  int readVInt()  override final;

  int64_t readVLong()  override final;

  char readByte(int64_t pos)  override;

  short readShort(int64_t pos)  override;

  int readInt(int64_t pos)  override;

  int64_t readLong(int64_t pos)  override;

private:
  void refill() ;

  /** Expert: implements buffer refill.  Reads bytes from the current position
   * in the input.
   * @param b the array to read bytes into
   * @param offset the offset in the array to start storing bytes
   * @param length the number of bytes to read
   */
protected:
  virtual void readInternal(std::deque<char> &b, int offset, int length) = 0;

public:
  int64_t getFilePointer() override final;
  void seek(int64_t pos)  override final;

  /** Expert: implements seek.  Sets current position in this file, where the
   * next {@link #readInternal(byte[],int,int)} will occur.
   * @see #readInternal(byte[],int,int)
   */
protected:
  virtual void seekInternal(int64_t pos) = 0;

public:
  std::shared_ptr<BufferedIndexInput> clone() override;

  std::shared_ptr<IndexInput>
  slice(const std::wstring &sliceDescription, int64_t offset,
        int64_t length)  override;

  /**
   * Flushes the in-memory buffer to the given output, copying at most
   * <code>numBytes</code>.
   * <p>
   * <b>NOTE:</b> this method does not refill the buffer, however it does
   * advance the buffer position.
   *
   * @return the number of bytes actually flushed from the in-memory buffer.
   */
protected:
  int flushBuffer(std::shared_ptr<IndexOutput> out,
                  int64_t numBytes) ;

  /**
   * Returns default buffer sizes for the given {@link IOContext}
   */
public:
  static int bufferSize(std::shared_ptr<IOContext> context);

  /**
   * Wraps a portion of another IndexInput with buffering.
   * <p><b>Please note:</b> This is in most cases ineffective, because it may
   * double buffer!
   */
  static std::shared_ptr<BufferedIndexInput>
  wrap(const std::wstring &sliceDescription, std::shared_ptr<IndexInput> other,
       int64_t offset, int64_t length);

  /**
   * Implementation of an IndexInput that reads from a portion of a file.
   */
private:
  class SlicedIndexInput;

protected:
  std::shared_ptr<BufferedIndexInput> shared_from_this()
  {
    return std::static_pointer_cast<BufferedIndexInput>(
        IndexInput::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
class BufferedIndexInput::SlicedIndexInput final : public BufferedIndexInput
{
  GET_CLASS_NAME(BufferedIndexInput::SlicedIndexInput)
public:
  std::shared_ptr<IndexInput> base;
  int64_t fileOffset = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  int64_t length_ = 0;

  SlicedIndexInput(const std::wstring &sliceDescription,
                   std::shared_ptr<IndexInput> base, int64_t offset,
                   int64_t length);

  std::shared_ptr<SlicedIndexInput> clone() override;

protected:
  void readInternal(std::deque<char> &b, int offset,
                    int len)  override;

  void seekInternal(int64_t pos) override;

public:
  virtual ~SlicedIndexInput();

  int64_t length() override;

protected:
  std::shared_ptr<SlicedIndexInput> shared_from_this()
  {
    return std::static_pointer_cast<SlicedIndexInput>(
        BufferedIndexInput::shared_from_this());
  }
};
