#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/MockDirectoryWrapper.h"

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
namespace org::apache::lucene::store
{

/**
 * Used by MockDirectoryWrapper to create an input stream that
 * keeps track of when it's been closed.
 */

class MockIndexInputWrapper : public IndexInput
{
  GET_CLASS_NAME(MockIndexInputWrapper)
private:
  std::shared_ptr<MockDirectoryWrapper> dir;

public:
  const std::wstring name;

private:
  std::shared_ptr<IndexInput> delegate_;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool closed;
  bool closed = false;

  // Which MockIndexInputWrapper we were cloned from, or null if we are not a
  // clone:
  const std::shared_ptr<MockIndexInputWrapper> parent;

  /** Sole constructor */
public:
  MockIndexInputWrapper(std::shared_ptr<MockDirectoryWrapper> dir,
                        const std::wstring &name,
                        std::shared_ptr<IndexInput> delegate_,
                        std::shared_ptr<MockIndexInputWrapper> parent);

  virtual ~MockIndexInputWrapper();

private:
  void ensureOpen();

public:
  std::shared_ptr<MockIndexInputWrapper> clone() override;

  std::shared_ptr<IndexInput>
  slice(const std::wstring &sliceDescription, int64_t offset,
        int64_t length)  override;

  int64_t getFilePointer() override;

  void seek(int64_t pos)  override;

  int64_t length() override;

  char readByte()  override;

  void readBytes(std::deque<char> &b, int offset,
                 int len)  override;

  void readBytes(std::deque<char> &b, int offset, int len,
                 bool useBuffer)  override;

  short readShort()  override;

  int readInt()  override;

  int64_t readLong()  override;

  std::wstring readString()  override;

  int readVInt()  override;

  int64_t readVLong()  override;

  int readZInt()  override;

  int64_t readZLong()  override;

  void skipBytes(int64_t numBytes)  override;

  std::unordered_map<std::wstring, std::wstring>
  readMapOfStrings()  override;

  std::shared_ptr<Set<std::wstring>>
  readSetOfStrings()  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<MockIndexInputWrapper> shared_from_this()
  {
    return std::static_pointer_cast<MockIndexInputWrapper>(
        IndexInput::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
