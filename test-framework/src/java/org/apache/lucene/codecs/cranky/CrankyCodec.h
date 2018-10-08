#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class Codec;
}

namespace org::apache::lucene::codecs
{
class DocValuesFormat;
}
namespace org::apache::lucene::codecs
{
class FieldInfosFormat;
}
namespace org::apache::lucene::codecs
{
class LiveDocsFormat;
}
namespace org::apache::lucene::codecs
{
class NormsFormat;
}
namespace org::apache::lucene::codecs
{
class PostingsFormat;
}
namespace org::apache::lucene::codecs
{
class SegmentInfoFormat;
}
namespace org::apache::lucene::codecs
{
class StoredFieldsFormat;
}
namespace org::apache::lucene::codecs
{
class TermVectorsFormat;
}
namespace org::apache::lucene::codecs
{
class CompoundFormat;
}
namespace org::apache::lucene::codecs
{
class PointsFormat;
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
namespace org::apache::lucene::codecs::cranky
{

using Codec = org::apache::lucene::codecs::Codec;
using CompoundFormat = org::apache::lucene::codecs::CompoundFormat;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using LiveDocsFormat = org::apache::lucene::codecs::LiveDocsFormat;
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using SegmentInfoFormat = org::apache::lucene::codecs::SegmentInfoFormat;
using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;

/** Codec for testing that throws random IOExceptions */
class CrankyCodec : public FilterCodec
{
  GET_CLASS_NAME(CrankyCodec)
public:
  const std::shared_ptr<Random> random;

  /**
   * Wrap the provided codec with crankiness.
   * Try passing Asserting for the most fun.
   */
  CrankyCodec(std::shared_ptr<Codec> delegate_, std::shared_ptr<Random> random);

  std::shared_ptr<DocValuesFormat> docValuesFormat() override;

  std::shared_ptr<FieldInfosFormat> fieldInfosFormat() override;

  std::shared_ptr<LiveDocsFormat> liveDocsFormat() override;

  std::shared_ptr<NormsFormat> normsFormat() override;

  std::shared_ptr<PostingsFormat> postingsFormat() override;

  std::shared_ptr<SegmentInfoFormat> segmentInfoFormat() override;

  std::shared_ptr<StoredFieldsFormat> storedFieldsFormat() override;

  std::shared_ptr<TermVectorsFormat> termVectorsFormat() override;

  std::shared_ptr<CompoundFormat> compoundFormat() override;

  std::shared_ptr<PointsFormat> pointsFormat() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<CrankyCodec> shared_from_this()
  {
    return std::static_pointer_cast<CrankyCodec>(
        org.apache.lucene.codecs.FilterCodec::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::cranky