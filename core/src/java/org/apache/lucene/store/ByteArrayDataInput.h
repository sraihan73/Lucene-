#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::store
{

/**
 * DataInput backed by a byte array.
 * <b>WARNING:</b> This class omits all low-level checks.
 * @lucene.experimental
 */
class ByteArrayDataInput final : public DataInput
{
  GET_CLASS_NAME(ByteArrayDataInput)

private:
  std::deque<char> bytes;

  int pos = 0;
  int limit = 0;

public:
  ByteArrayDataInput(std::deque<char> &bytes);

  ByteArrayDataInput(std::deque<char> &bytes, int offset, int len);

  ByteArrayDataInput();

  void reset(std::deque<char> &bytes);

  // NOTE: sets pos to 0, which is not right if you had
  // called reset w/ non-zero offset!!
  void rewind();

  int getPosition();

  void setPosition(int pos);

  void reset(std::deque<char> &bytes, int offset, int len);

  int length();

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

protected:
  std::shared_ptr<ByteArrayDataInput> shared_from_this()
  {
    return std::static_pointer_cast<ByteArrayDataInput>(
        DataInput::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
