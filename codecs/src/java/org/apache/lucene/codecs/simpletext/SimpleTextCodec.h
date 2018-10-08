#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"
#include "SimpleTextCompoundFormat.h"
#include "SimpleTextDocValuesFormat.h"
#include "SimpleTextFieldInfosFormat.h"
#include "SimpleTextLiveDocsFormat.h"
#include "SimpleTextNormsFormat.h"
#include "SimpleTextPointsFormat.h"
#include "SimpleTextPostingsFormat.h"
#include "SimpleTextSegmentInfoFormat.h"
#include "SimpleTextStoredFieldsFormat.h"
#include "SimpleTextTermVectorsFormat.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/PostingsFormat.h"

#include  "core/src/java/org/apache/lucene/codecs/StoredFieldsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/SegmentInfoFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldInfosFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/TermVectorsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/NormsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/LiveDocsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/CompoundFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsFormat.h"

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

using Codec = org::apache::lucene::codecs::Codec;
using CompoundFormat = org::apache::lucene::codecs::CompoundFormat;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using LiveDocsFormat = org::apache::lucene::codecs::LiveDocsFormat;
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using SegmentInfoFormat = org::apache::lucene::codecs::SegmentInfoFormat;
using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;

/**
 * plain text index format.
 * <p>
 * <b>FOR RECREATIONAL USE ONLY</b>
 * @lucene.experimental
 */
class SimpleTextCodec final : public Codec
{
  GET_CLASS_NAME(SimpleTextCodec)
private:
  const std::shared_ptr<PostingsFormat> postings =
      std::make_shared<SimpleTextPostingsFormat>();
  const std::shared_ptr<StoredFieldsFormat> storedFields =
      std::make_shared<SimpleTextStoredFieldsFormat>();
  const std::shared_ptr<SegmentInfoFormat> segmentInfos =
      std::make_shared<SimpleTextSegmentInfoFormat>();
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<FieldInfosFormat> fieldInfosFormat_ =
      std::make_shared<SimpleTextFieldInfosFormat>();
  const std::shared_ptr<TermVectorsFormat> vectorsFormat =
      std::make_shared<SimpleTextTermVectorsFormat>();
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<NormsFormat> normsFormat_ =
      std::make_shared<SimpleTextNormsFormat>();
  const std::shared_ptr<LiveDocsFormat> liveDocs =
      std::make_shared<SimpleTextLiveDocsFormat>();
  const std::shared_ptr<DocValuesFormat> dvFormat =
      std::make_shared<SimpleTextDocValuesFormat>();
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<CompoundFormat> compoundFormat_ =
      std::make_shared<SimpleTextCompoundFormat>();
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<PointsFormat> pointsFormat_ =
      std::make_shared<SimpleTextPointsFormat>();

public:
  SimpleTextCodec();

  std::shared_ptr<PostingsFormat> postingsFormat() override;

  std::shared_ptr<StoredFieldsFormat> storedFieldsFormat() override;

  std::shared_ptr<TermVectorsFormat> termVectorsFormat() override;

  std::shared_ptr<FieldInfosFormat> fieldInfosFormat() override;

  std::shared_ptr<SegmentInfoFormat> segmentInfoFormat() override;

  std::shared_ptr<NormsFormat> normsFormat() override;

  std::shared_ptr<LiveDocsFormat> liveDocsFormat() override;

  std::shared_ptr<DocValuesFormat> docValuesFormat() override;

  std::shared_ptr<CompoundFormat> compoundFormat() override;

  std::shared_ptr<PointsFormat> pointsFormat() override;

protected:
  std::shared_ptr<SimpleTextCodec> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextCodec>(
        org.apache.lucene.codecs.Codec::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/simpletext/
