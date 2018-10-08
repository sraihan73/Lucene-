#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexOutput;
}

namespace org::apache::lucene::store
{
class Directory;
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

/** Corrupts on bit of a file after close */
class CorruptingIndexOutput : public IndexOutput
{
  GET_CLASS_NAME(CorruptingIndexOutput)
protected:
  const std::shared_ptr<IndexOutput> out;

public:
  const std::shared_ptr<Directory> dir;
  const int64_t byteToCorrupt;

private:
  bool closed = false;

public:
  CorruptingIndexOutput(std::shared_ptr<Directory> dir, int64_t byteToCorrupt,
                        std::shared_ptr<IndexOutput> out);

  std::wstring getName() override;

  virtual ~CorruptingIndexOutput();

protected:
  virtual void corruptFile() ;

public:
  int64_t getFilePointer() override;

  int64_t getChecksum()  override;

  virtual std::wstring toString();

  void writeByte(char b)  override;

  void writeBytes(std::deque<char> &b, int offset,
                  int length)  override;

protected:
  std::shared_ptr<CorruptingIndexOutput> shared_from_this()
  {
    return std::static_pointer_cast<CorruptingIndexOutput>(
        IndexOutput::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
