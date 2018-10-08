#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
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

/** Appends many points, and then at the end provides a {@link PointReader} to
 * iterate those points.  This abstracts away whether we write to disk, or use
 * simple arrays in heap.
 *
 *  @lucene.internal */
class PointWriter : public Closeable
{
  GET_CLASS_NAME(PointWriter)
  /** Add a new point */
public:
  virtual void append(std::deque<char> &packedValue, int64_t ord,
                      int docID) = 0;

  /** Returns a {@link PointReader} iterator to step through all previously
   * added points */
  virtual std::shared_ptr<PointReader> getReader(int64_t startPoint,
                                                 int64_t length) = 0;

  /** Returns the single shared reader, used at multiple times during the
   * recursion, to read previously added points */
  virtual std::shared_ptr<PointReader> getSharedReader(
      int64_t startPoint, int64_t length,
      std::deque<std::shared_ptr<Closeable>> &toCloseHeroically) = 0;

  /** Removes any temp files behind this writer */
  virtual void destroy() = 0;
};

} // #include  "core/src/java/org/apache/lucene/util/bkd/
