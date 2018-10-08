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

/** Implementation class for buffered {@link IndexOutput} that writes to an
 * {@link OutputStream}. */
class OutputStreamIndexOutput : public IndexOutput
{

private:
  const std::shared_ptr<CRC32> crc = std::make_shared<CRC32>();
  const std::shared_ptr<BufferedOutputStream> os;

  int64_t bytesWritten = 0LL;
  bool flushedOnClose = false;

  /**
   * Creates a new {@link OutputStreamIndexOutput} with the given buffer size.
   * @param bufferSize the buffer size in bytes used to buffer writes
   * internally.
   * @throws IllegalArgumentException if the given buffer size is less or equal
   * to <tt>0</tt>
   */
public:
  OutputStreamIndexOutput(const std::wstring &resourceDescription,
                          const std::wstring &name,
                          std::shared_ptr<OutputStream> out, int bufferSize);

  void writeByte(char b)  override final;

  void writeBytes(std::deque<char> &b, int offset,
                  int length)  override final;

  virtual ~OutputStreamIndexOutput();

  int64_t getFilePointer() override final;

  int64_t getChecksum()  override final;

protected:
  std::shared_ptr<OutputStreamIndexOutput> shared_from_this()
  {
    return std::static_pointer_cast<OutputStreamIndexOutput>(
        IndexOutput::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
