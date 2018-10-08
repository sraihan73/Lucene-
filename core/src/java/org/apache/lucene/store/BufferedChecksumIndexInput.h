#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
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
 * Simple implementation of {@link ChecksumIndexInput} that wraps
 * another input and delegates calls.
 */
class BufferedChecksumIndexInput : public ChecksumIndexInput
{
  GET_CLASS_NAME(BufferedChecksumIndexInput)
public:
  const std::shared_ptr<IndexInput> main;
  const std::shared_ptr<Checksum> digest;

  /** Creates a new BufferedChecksumIndexInput */
  BufferedChecksumIndexInput(std::shared_ptr<IndexInput> main);

  char readByte()  override;

  void readBytes(std::deque<char> &b, int offset,
                 int len)  override;

  int64_t getChecksum() override;

  virtual ~BufferedChecksumIndexInput();

  int64_t getFilePointer() override;

  int64_t length() override;

  std::shared_ptr<IndexInput> clone() override;

  std::shared_ptr<IndexInput>
  slice(const std::wstring &sliceDescription, int64_t offset,
        int64_t length)  override;

protected:
  std::shared_ptr<BufferedChecksumIndexInput> shared_from_this()
  {
    return std::static_pointer_cast<BufferedChecksumIndexInput>(
        ChecksumIndexInput::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
