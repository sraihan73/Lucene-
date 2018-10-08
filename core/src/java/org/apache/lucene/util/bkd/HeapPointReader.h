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
namespace org::apache::lucene::util::bkd
{

/** Utility class to read buffered points from in-heap arrays.
 *
 * @lucene.internal */
class HeapPointReader final : public PointReader
{
  GET_CLASS_NAME(HeapPointReader)
private:
  int curRead = 0;

public:
  const std::deque<std::deque<char>> blocks;
  const int valuesPerBlock;
  const int packedBytesLength;
  std::deque<int64_t> const ordsLong;
  std::deque<int> const ords;
  std::deque<int> const docIDs;
  const int end;
  std::deque<char> const scratch;
  const bool singleValuePerDoc;

  HeapPointReader(std::deque<std::deque<char>> &blocks, int valuesPerBlock,
                  int packedBytesLength, std::deque<int> &ords,
                  std::deque<int64_t> &ordsLong, std::deque<int> &docIDs,
                  int start, int end, bool singleValuePerDoc);

  void writePackedValue(int index, std::deque<char> &bytes);

  void readPackedValue(int index, std::deque<char> &bytes);

  bool next() override;

  std::deque<char> packedValue() override;

  int docID() override;

  int64_t ord() override;

  virtual ~HeapPointReader();

protected:
  std::shared_ptr<HeapPointReader> shared_from_this()
  {
    return std::static_pointer_cast<HeapPointReader>(
        PointReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/bkd/
