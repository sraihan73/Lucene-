#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
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

using DataOutput = org::apache::lucene::store::DataOutput;

/**
 * A {@link DataOutput} that can be used to build a byte[].
 *
 * @lucene.internal
 */
class GrowableByteArrayDataOutput final : public DataOutput
{
  GET_CLASS_NAME(GrowableByteArrayDataOutput)

  /** Minimum utf8 byte size of a string over which double pass over string is
   * to save memory during encode */
public:
  static constexpr int MIN_UTF8_SIZE_TO_ENABLE_DOUBLE_PASS_ENCODING = 65536;

  /** The bytes */
private:
  std::deque<char> bytes;

  /** The length */
  int length = 0;

  // scratch for utf8 encoding of small strings
  std::deque<char> scratchBytes;

  /** Create a {@link GrowableByteArrayDataOutput} with the given initial
   * capacity. */
public:
  GrowableByteArrayDataOutput(int cp);

  void writeByte(char b) override;

  void writeBytes(std::deque<char> &b, int off, int len) override;

  void writeString(const std::wstring &string)  override;

  std::deque<char> getBytes();

  int getPosition();

  void reset();

protected:
  std::shared_ptr<GrowableByteArrayDataOutput> shared_from_this()
  {
    return std::static_pointer_cast<GrowableByteArrayDataOutput>(
        org.apache.lucene.store.DataOutput::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
