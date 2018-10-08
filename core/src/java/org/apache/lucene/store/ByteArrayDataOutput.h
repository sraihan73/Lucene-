#pragma once
#include "stringhelper.h"
#include <memory>
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
 * DataOutput backed by a byte array.
 * <b>WARNING:</b> This class omits most low-level checks,
 * so be sure to test heavily with assertions enabled.
 * @lucene.experimental
 */
class ByteArrayDataOutput : public DataOutput
{
  GET_CLASS_NAME(ByteArrayDataOutput)
private:
  std::deque<char> bytes;

  int pos = 0;
  int limit = 0;

public:
  ByteArrayDataOutput(std::deque<char> &bytes);

  ByteArrayDataOutput(std::deque<char> &bytes, int offset, int len);

  ByteArrayDataOutput();

  virtual void reset(std::deque<char> &bytes);

  virtual void reset(std::deque<char> &bytes, int offset, int len);

  virtual int getPosition();

  void writeByte(char b) override;

  void writeBytes(std::deque<char> &b, int offset, int length) override;

protected:
  std::shared_ptr<ByteArrayDataOutput> shared_from_this()
  {
    return std::static_pointer_cast<ByteArrayDataOutput>(
        DataOutput::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
