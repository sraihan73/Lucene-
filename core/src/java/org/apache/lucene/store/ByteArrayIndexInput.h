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
class IndexInput;
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
 * DataInput backed by a byte array.
 * <b>WARNING:</b> This class omits all low-level checks.
 * @lucene.experimental
 */
class ByteArrayIndexInput final : public IndexInput
{
  GET_CLASS_NAME(ByteArrayIndexInput)

private:
  std::deque<char> bytes;

  int pos = 0;
  int limit = 0;

public:
  ByteArrayIndexInput(const std::wstring &description,
                      std::deque<char> &bytes);

  int64_t getFilePointer() override;

  void seek(int64_t pos) override;

  void reset(std::deque<char> &bytes, int offset, int len);

  int64_t length() override;

  bool eof();

  void skipBytes(int64_t count) override;

  short readShort() override;

  int readInt() override;

  int64_t readLong() override;

  int readVInt() override;

  int64_t readVLong() override;

  // NOTE: AIOOBE not EOF if you read too much
  char readByte() override;

  // NOTE: AIOOBE not EOF if you read too much
  void readBytes(std::deque<char> &b, int offset, int len) override;

  virtual ~ByteArrayIndexInput();

  std::shared_ptr<IndexInput>
  slice(const std::wstring &sliceDescription, int64_t offset,
        int64_t length)  override;

protected:
  std::shared_ptr<ByteArrayIndexInput> shared_from_this()
  {
    return std::static_pointer_cast<ByteArrayIndexInput>(
        IndexInput::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
