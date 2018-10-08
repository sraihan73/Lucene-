#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/store/IndexInput.h"

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

using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexInput = org::apache::lucene::store::IndexInput;

/** Represents a logical byte[] as a series of pages.  You
 *  can write-once into the logical byte[] (append only),
 *  using copy, and then retrieve slices (BytesRef) into it
 *  using fill.
 *
 * @lucene.internal
 **/
// TODO: refactor this, byteblockpool, fst.bytestore, and any
// other "shift/mask big arrays". there are too many of these classes!
class PagedBytes final : public std::enable_shared_from_this<PagedBytes>,
                         public Accountable
{
private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(PagedBytes::typeid);
  std::deque<std::deque<char>> blocks = std::deque<std::deque<char>>(16);
  int numBlocks = 0;
  // TODO: these are unused?
  const int blockSize;
  const int blockBits;
  const int blockMask;
  bool didSkipBytes = false;
  bool frozen = false;
  int upto = 0;
  std::deque<char> currentBlock;
  const int64_t bytesUsedPerBlock;

  static std::deque<char> const EMPTY_BYTES;

  /** Provides methods to read BytesRefs from a frozen
   *  PagedBytes.
   *
   * @see #freeze */
public:
  class Reader final : public std::enable_shared_from_this<Reader>,
                       public Accountable
  {
    GET_CLASS_NAME(Reader)
  private:
    static const int64_t BASE_RAM_BYTES_USED =
        RamUsageEstimator::shallowSizeOfInstance(Reader::typeid);
    std::deque<std::deque<char>> const blocks;
    const int blockBits;
    const int blockMask;
    const int blockSize;
    const int64_t bytesUsedPerBlock;

    Reader(std::shared_ptr<PagedBytes> pagedBytes);

    /**
     * Gets a slice out of {@link PagedBytes} starting at <i>start</i> with a
     * given length. Iff the slice spans across a block border this method will
     * allocate sufficient resources and copy the paged data.
     * <p>
     * Slices spanning more than two blocks are not supported.
     * </p>
     * @lucene.internal
     **/
  public:
    void fillSlice(std::shared_ptr<BytesRef> b, int64_t start, int length);

    /**
     * Reads length as 1 or 2 byte vInt prefix, starting at <i>start</i>.
     * <p>
     * <b>Note:</b> this method does not support slices spanning across block
     * borders.
     * </p>
     *
     * @lucene.internal
     **/
    // TODO: this really needs to be refactored into fieldcacheimpl
    void fill(std::shared_ptr<BytesRef> b, int64_t start);

    int64_t ramBytesUsed() override;

    virtual std::wstring toString();
  };

  /** 1&lt;&lt;blockBits must be bigger than biggest single
   *  BytesRef slice that will be pulled */
public:
  PagedBytes(int blockBits);

private:
  void addBlock(std::deque<char> &block);

  /** Read this many bytes from in */
public:
  void copy(std::shared_ptr<IndexInput> in_,
            int64_t byteCount) ;

  /** Copy BytesRef in, setting BytesRef out to the result.
   * Do not use this if you will use freeze(true).
   * This only supports bytes.length &lt;= blockSize */
  void copy(std::shared_ptr<BytesRef> bytes, std::shared_ptr<BytesRef> out);

  /** Commits final byte[], trimming it if necessary and if trim=true */
  std::shared_ptr<Reader> freeze(bool trim);

  int64_t getPointer();

  int64_t ramBytesUsed() override;

  /** Copy bytes in, writing the length as a 1 or 2 byte
   *  vInt prefix. */
  // TODO: this really needs to be refactored into fieldcacheimpl!
  int64_t copyUsingLengthPrefix(std::shared_ptr<BytesRef> bytes);

public:
  class PagedBytesDataInput final : public DataInput
  {
    GET_CLASS_NAME(PagedBytesDataInput)
  private:
    std::shared_ptr<PagedBytes> outerInstance;

    int currentBlockIndex = 0;
    int currentBlockUpto = 0;
    std::deque<char> currentBlock;

  public:
    PagedBytesDataInput(std::shared_ptr<PagedBytes> outerInstance);

    std::shared_ptr<PagedBytesDataInput> clone() override;

    /** Returns the current byte position. */
    int64_t getPosition();

    /** Seek to a position previously obtained from
     *  {@link #getPosition}. */
    void setPosition(int64_t pos);

    char readByte() override;

    void readBytes(std::deque<char> &b, int offset, int len) override;

  private:
    void nextBlock();

  protected:
    std::shared_ptr<PagedBytesDataInput> shared_from_this()
    {
      return std::static_pointer_cast<PagedBytesDataInput>(
          org.apache.lucene.store.DataInput::shared_from_this());
    }
  };

public:
  class PagedBytesDataOutput final : public DataOutput
  {
    GET_CLASS_NAME(PagedBytesDataOutput)
  private:
    std::shared_ptr<PagedBytes> outerInstance;

  public:
    PagedBytesDataOutput(std::shared_ptr<PagedBytes> outerInstance);

    void writeByte(char b) override;

    void writeBytes(std::deque<char> &b, int offset, int length) override;

    /** Return the current byte position. */
    int64_t getPosition();

  protected:
    std::shared_ptr<PagedBytesDataOutput> shared_from_this()
    {
      return std::static_pointer_cast<PagedBytesDataOutput>(
          org.apache.lucene.store.DataOutput::shared_from_this());
    }
  };

  /** Returns a DataInput to read values from this
   *  PagedBytes instance. */
public:
  std::shared_ptr<PagedBytesDataInput> getDataInput();

  /** Returns a DataOutput that you may use to write into
   *  this PagedBytes instance.  If you do this, you should
   *  not call the other writing methods (eg, copy);
   *  results are undefined. */
  std::shared_ptr<PagedBytesDataOutput> getDataOutput();
};

} // #include  "core/src/java/org/apache/lucene/util/
