#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PointsFormat.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class PointsWriter;
}

namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class PointsReader;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
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
namespace org::apache::lucene::codecs::simpletext
{

using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

/** For debugging, curiosity, transparency only!!  Do not
 *  use this codec in production.
 *
 *  <p>This codec stores all dimensional data in a single
 *  human-readable text file (_N.dim).  You can view this in
 *  any text editor, and even edit it to alter your index.
 *
 *  @lucene.experimental */
class SimpleTextPointsFormat final : public PointsFormat
{
  GET_CLASS_NAME(SimpleTextPointsFormat)

public:
  std::shared_ptr<PointsWriter> fieldsWriter(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<PointsReader> fieldsReader(
      std::shared_ptr<SegmentReadState> state)  override;

  /** Extension of points data file */
  static const std::wstring POINT_EXTENSION;

  /** Extension of points index file */
  static const std::wstring POINT_INDEX_EXTENSION;

protected:
  std::shared_ptr<SimpleTextPointsFormat> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextPointsFormat>(
        org.apache.lucene.codecs.PointsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::simpletext
