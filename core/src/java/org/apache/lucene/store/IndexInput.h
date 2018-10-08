#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class RandomAccessInput;
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
 * Abstract base class for input from a file in a {@link Directory}.  A
 * random-access input stream.  Used for all Lucene index input operations.
 *
 * <p>{@code IndexInput} may only be used from one thread, because it is not
 * thread safe (it keeps internal state like file position). To allow
 * multithreaded use, every {@code IndexInput} instance must be cloned before
 * it is used in another thread. Subclasses must therefore implement {@link
#clone()},
 * returning a new {@code IndexInput} which operates on the same underlying
GET_CLASS_NAME(es)
 * resource, but positioned independently.
 *
 * <p><b>Warning:</b> Lucene never closes cloned
 * {@code IndexInput}s, it will only call {@link #close()} on the original
object.
 *
 * <p>If you access the cloned IndexInput after closing the original object,
 * any <code>readXXX</code> methods will throw {@link AlreadyClosedException}.
 *
 * @see Directory
 */
class IndexInput : public DataInput, public Cloneable
{
  GET_CLASS_NAME(IndexInput)

private:
  const std::wstring resourceDescription;

  /** resourceDescription should be a non-null, opaque string
   *  describing this resource; it's returned from
   *  {@link #toString}. */
protected:
  IndexInput(const std::wstring &resourceDescription);

  /** Closes the stream to further operations. */
public:
  void close() = 0;
  override

      /** Returns the current position in this file, where the next read will
       * occur.
       * @see #seek(long)
       */
      virtual int64_t
      getFilePointer() = 0;

  /** Sets current position in this file, where the next read will occur.  If
   * this is beyond the end of the file then this will throw {@code
   * EOFException} and then the stream is in an undetermined state.
   *
   * @see #getFilePointer()
   */
  virtual void seek(int64_t pos) = 0;

  /** The number of bytes in the file. */
  virtual int64_t length() = 0;

  virtual std::wstring toString();

  /** {@inheritDoc}
   *
   * <p><b>Warning:</b> Lucene never closes cloned
   * {@code IndexInput}s, it will only call {@link #close()} on the original
   * object.
   *
   * <p>If you access the cloned IndexInput after closing the original object,
   * any <code>readXXX</code> methods will throw {@link AlreadyClosedException}.
   *
   * <p>This method is NOT thread safe, so if the current {@code IndexInput}
   * is being used by one thread while {@code clone} is called by another,
   * disaster could strike.
   */
  std::shared_ptr<IndexInput> clone() override;

  /**
   * Creates a slice of this index input, with the given description, offset,
   * and length. The slice is seeked to the beginning.
   */
  virtual std::shared_ptr<IndexInput>
  slice(const std::wstring &sliceDescription, int64_t offset,
        int64_t length) = 0;

  /** Subclasses call this to get the std::wstring for resourceDescription of a slice
   * of this {@code IndexInput}. */
protected:
  virtual std::wstring
  getFullSliceDescription(const std::wstring &sliceDescription);

  /**
   * Creates a random-access slice of this index input, with the given offset
   * and length. <p> The default implementation calls {@link #slice}, and it
   * doesn't support random access, it implements absolute reads as seek+read.
   */
public:
  virtual std::shared_ptr<RandomAccessInput>
  randomAccessSlice(int64_t offset, int64_t length) ;

private:
  class RandomAccessInputAnonymousInnerClass
      : public std::enable_shared_from_this<
            RandomAccessInputAnonymousInnerClass>,
        public RandomAccessInput
  {
    GET_CLASS_NAME(RandomAccessInputAnonymousInnerClass)
  private:
    std::shared_ptr<IndexInput> outerInstance;

    std::shared_ptr<org::apache::lucene::store::IndexInput> slice;

  public:
    RandomAccessInputAnonymousInnerClass(
        std::shared_ptr<IndexInput> outerInstance,
        std::shared_ptr<org::apache::lucene::store::IndexInput> slice);

    char readByte(int64_t pos)  override;

    short readShort(int64_t pos)  override;

    int readInt(int64_t pos)  override;

    int64_t readLong(int64_t pos)  override;

    std::wstring toString();
  };

protected:
  std::shared_ptr<IndexInput> shared_from_this()
  {
    return std::static_pointer_cast<IndexInput>(DataInput::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
