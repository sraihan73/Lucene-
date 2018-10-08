#pragma once
#include "../PointsReader.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"

#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include  "core/src/java/org/apache/lucene/util/bkd/BKDReader.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"

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
namespace org::apache::lucene::codecs::lucene60
{

using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointValues = org::apache::lucene::index::PointValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using BKDReader = org::apache::lucene::util::bkd::BKDReader;

/** Reads point values previously written with {@link Lucene60PointsWriter} */
class Lucene60PointsReader : public PointsReader
{
  GET_CLASS_NAME(Lucene60PointsReader)
public:
  const std::shared_ptr<IndexInput> dataIn;
  const std::shared_ptr<SegmentReadState> readState;
  const std::unordered_map<int, std::shared_ptr<BKDReader>> readers =
      std::unordered_map<int, std::shared_ptr<BKDReader>>();

  /** Sole constructor */
  Lucene60PointsReader(std::shared_ptr<SegmentReadState> readState) throw(
      IOException);

  /** Returns the underlying {@link BKDReader}.
   *
   * @lucene.internal */
  std::shared_ptr<PointValues>
  getValues(const std::wstring &fieldName) override;

  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  void checkIntegrity()  override;

  virtual ~Lucene60PointsReader();

protected:
  std::shared_ptr<Lucene60PointsReader> shared_from_this()
  {
    return std::static_pointer_cast<Lucene60PointsReader>(
        org.apache.lucene.codecs.PointsReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/lucene60/
