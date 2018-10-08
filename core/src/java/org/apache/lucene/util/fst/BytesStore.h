#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class DataInput;
}

namespace org::apache::lucene::store
{
class DataOutput;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
namespace org::apache::lucene::util::fst
{
class BytesReader;
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
namespace org::apache::lucene::util::fst
{

using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Accountable = org::apache::lucene::util::Accountable;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

// TODO: merge with PagedBytes, except PagedBytes doesn't
// let you read while writing which FST needs

class BytesStore : public DataOutput, public Accountable
{
  GET_CLASS_NAME(BytesStore)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(BytesStore::typeid) +
      RamUsageEstimator::shallowSizeOfInstance(std::deque::typeid);

  const std::deque<std::deque<char>> blocks =
      std::deque<std::deque<char>>();

  const int blockSize;
  const int blockBits;
  const int blockMask;

  std::deque<char> current;
  int nextWrite = 0;

public:
  BytesStore(int blockBits);

  /** Pulls bytes from the provided IndexInput.  */
  BytesStore(std::shared_ptr<DataInput> in_, int64_t numBytes,
             int maxBlockSize) ;

  /** Absolute write byte; you must ensure dest is &lt; max
   *  position written so far. */
  virtual void writeByte(int dest, char b);

  void writeByte(char b) override;

  void writeBytes(std::deque<char> &b, int offset, int len) override;

  virtual int getBlockBits();

  /** Absolute writeBytes without changing the current
   *  position.  Note: this cannot "grow" the bytes, so you
   *  must only call it on already written parts. */
  virtual void writeBytes(int64_t dest, std::deque<char> &b, int offset,
                          int len);

  /** Absolute copy bytes self to self, without changing the
   *  position. Note: this cannot "grow" the bytes, so must
   *  only call it on already written parts. */
  virtual void copyBytes(int64_t src, int64_t dest, int len);

  /** Writes an int at the absolute position without
   *  changing the current pointer. */
  virtual void writeInt(int64_t pos, int value);

  /** Reverse from srcPos, inclusive, to destPos, inclusive. */
  virtual void reverse(int64_t srcPos, int64_t destPos);

  virtual void skipBytes(int len);

  virtual int64_t getPosition();

  /** Pos must be less than the max position written so far!
   *  Ie, you cannot "grow" the file with this! */
  virtual void truncate(int64_t newLen);

  virtual void finish();

  /** Writes all of our bytes to the target {@link DataOutput}. */
  virtual void writeTo(std::shared_ptr<DataOutput> out) ;

  virtual std::shared_ptr<FST::BytesReader> getForwardReader();

private:
  class BytesReaderAnonymousInnerClass : public FST::BytesReader
  {
    GET_CLASS_NAME(BytesReaderAnonymousInnerClass)
  private:
    std::shared_ptr<BytesStore> outerInstance;

  public:
    BytesReaderAnonymousInnerClass(std::shared_ptr<BytesStore> outerInstance);

  private:
    std::deque<char> outerInstance->current;
    int nextBuffer = 0;
    int nextRead = 0;

  public:
    char readByte() override;

    void skipBytes(int64_t count) override;

    void readBytes(std::deque<char> &b, int offset, int len) override;

    int64_t getPosition() override;

    void setPosition(int64_t pos) override;

    bool reversed() override;

  protected:
    std::shared_ptr<BytesReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<BytesReaderAnonymousInnerClass>(
          FST.BytesReader::shared_from_this());
    }
  };

public:
  virtual std::shared_ptr<FST::BytesReader> getReverseReader();

  virtual std::shared_ptr<FST::BytesReader> getReverseReader(bool allowSingle);

private:
  class BytesReaderAnonymousInnerClass2 : public FST::BytesReader
  {
    GET_CLASS_NAME(BytesReaderAnonymousInnerClass2)
  private:
    std::shared_ptr<BytesStore> outerInstance;

  public:
    BytesReaderAnonymousInnerClass2(std::shared_ptr<BytesStore> outerInstance);

  private:
    std::deque<char> outerInstance->current;
    int nextBuffer = 0;
    int nextRead = 0;

  public:
    char readByte() override;

    void skipBytes(int64_t count) override;

    void readBytes(std::deque<char> &b, int offset, int len) override;

    int64_t getPosition() override;

    void setPosition(int64_t pos) override;

    bool reversed() override;

  protected:
    std::shared_ptr<BytesReaderAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<BytesReaderAnonymousInnerClass2>(
          FST.BytesReader::shared_from_this());
    }
  };

public:
  int64_t ramBytesUsed() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<BytesStore> shared_from_this()
  {
    return std::static_pointer_cast<BytesStore>(
        org.apache.lucene.store.DataOutput::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::fst
