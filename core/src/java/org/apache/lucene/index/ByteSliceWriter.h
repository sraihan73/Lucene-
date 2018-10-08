#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/ByteBlockPool.h"

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
namespace org::apache::lucene::index
{

using DataOutput = org::apache::lucene::store::DataOutput;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;

/**
 * Class to write byte streams into slices of shared
 * byte[].  This is used by DocumentsWriter to hold the
 * posting deque for many terms in RAM.
 */

class ByteSliceWriter final : public DataOutput
{
  GET_CLASS_NAME(ByteSliceWriter)

private:
  std::deque<char> slice;
  int upto = 0;
  const std::shared_ptr<ByteBlockPool> pool;

public:
  int offset0 = 0;

  ByteSliceWriter(std::shared_ptr<ByteBlockPool> pool);

  /**
   * Set up the writer to write at address.
   */
  void init(int address);

  /** Write byte into byte slice stream */
  void writeByte(char b) override;

  void writeBytes(std::deque<char> &b, int offset, int const len) override;

  int getAddress();

protected:
  std::shared_ptr<ByteSliceWriter> shared_from_this()
  {
    return std::static_pointer_cast<ByteSliceWriter>(
        org.apache.lucene.store.DataOutput::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/index/
