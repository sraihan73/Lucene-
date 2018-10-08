#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/ByteBlockPool.h"

#include  "core/src/java/org/apache/lucene/store/DataOutput.h"

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

using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;

/* IndexInput that knows how to read the byte slices written
 * by Posting and PostingVector.  We read the bytes in
 * each slice until we hit the end of that slice at which
 * point we read the forwarding address of the next slice
 * and then jump to it.*/
class ByteSliceReader final : public DataInput
{
  GET_CLASS_NAME(ByteSliceReader)
public:
  std::shared_ptr<ByteBlockPool> pool;
  int bufferUpto = 0;
  std::deque<char> buffer;
  int upto = 0;
  int limit = 0;
  int level = 0;
  int bufferOffset = 0;

  int endIndex = 0;

  void init(std::shared_ptr<ByteBlockPool> pool, int startIndex, int endIndex);

  bool eof();

  char readByte() override;

  int64_t writeTo(std::shared_ptr<DataOutput> out) ;

  void nextSlice();

  void readBytes(std::deque<char> &b, int offset, int len) override;

protected:
  std::shared_ptr<ByteSliceReader> shared_from_this()
  {
    return std::static_pointer_cast<ByteSliceReader>(
        org.apache.lucene.store.DataInput::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/index/
