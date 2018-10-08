#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/util/bkd/PointReader.h"

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

using BytesRef = org::apache::lucene::util::BytesRef;

/** Utility class to write new points into in-heap arrays.
 *
 *  @lucene.internal */
class HeapPointWriter final
    : public std::enable_shared_from_this<HeapPointWriter>,
      public PointWriter
{
  GET_CLASS_NAME(HeapPointWriter)
public:
  std::deque<int> docIDs;
  std::deque<int64_t> ordsLong;
  std::deque<int> ords;

private:
  int nextWrite = 0;
  bool closed = false;

public:
  const int maxSize;
  const int valuesPerBlock;
  const int packedBytesLength;
  const bool singleValuePerDoc;
  // NOTE: can't use ByteBlockPool because we need random-write access when
  // sorting in heap
  const std::deque<std::deque<char>> blocks =
      std::deque<std::deque<char>>();

  HeapPointWriter(int initSize, int maxSize, int packedBytesLength,
                  bool longOrds, bool singleValuePerDoc);

  void copyFrom(std::shared_ptr<HeapPointWriter> other);

  void readPackedValue(int index, std::deque<char> &bytes);

  /** Returns a reference, in <code>result</code>, to the byte[] slice holding
   * this value */
  void getPackedValueSlice(int index, std::shared_ptr<BytesRef> result);

  void writePackedValue(int index, std::deque<char> &bytes);

  void append(std::deque<char> &packedValue, int64_t ord,
              int docID) override;

  std::shared_ptr<PointReader> getReader(int64_t start,
                                         int64_t length) override;

  std::shared_ptr<PointReader> getSharedReader(
      int64_t start, int64_t length,
      std::deque<std::shared_ptr<Closeable>> &toCloseHeroically) override;

  virtual ~HeapPointWriter();

  void destroy() override;

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/util/bkd/
