#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/LongBitSet.h"

#include  "core/src/java/org/apache/lucene/util/bkd/PointWriter.h"

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

using LongBitSet = org::apache::lucene::util::LongBitSet;

/** One pass iterator through all points previously written with a
 *  {@link PointWriter}, abstracting away whether points a read
 *  from (offline) disk or simple arrays in heap.
 *
 * @lucene.internal */
class PointReader : public std::enable_shared_from_this<PointReader>
{
  GET_CLASS_NAME(PointReader)

  /** Returns false once iteration is done, else true. */
public:
  virtual bool next() = 0;

  /** Returns the packed byte[] value */
  virtual std::deque<char> packedValue() = 0;

  /** Point ordinal */
  virtual int64_t ord() = 0;

  /** DocID for this point */
  virtual int docID() = 0;

  /** Iterates through the next {@code count} ords, marking them in the provided
   * {@code ordBitSet}. */
  virtual void
  markOrds(int64_t count,
           std::shared_ptr<LongBitSet> ordBitSet) ;

  /** Splits this reader into left and right partitions */
  virtual int64_t split(int64_t count,
                          std::shared_ptr<LongBitSet> rightTree,
                          std::shared_ptr<PointWriter> left,
                          std::shared_ptr<PointWriter> right,
                          bool doClearBits) ;
};

} // #include  "core/src/java/org/apache/lucene/util/bkd/
