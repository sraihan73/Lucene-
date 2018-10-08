#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/TermVectorsFormat.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/TermVectorsReader.h"

#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/codecs/TermVectorsWriter.h"

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

using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

/**
 * plain text term vectors format.
 * <p>
 * <b>FOR RECREATIONAL USE ONLY</b>
 * @lucene.experimental
 */
class SimpleTextTermVectorsFormat : public TermVectorsFormat
{
  GET_CLASS_NAME(SimpleTextTermVectorsFormat)

public:
  std::shared_ptr<TermVectorsReader>
  vectorsReader(std::shared_ptr<Directory> directory,
                std::shared_ptr<SegmentInfo> segmentInfo,
                std::shared_ptr<FieldInfos> fieldInfos,
                std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<TermVectorsWriter>
  vectorsWriter(std::shared_ptr<Directory> directory,
                std::shared_ptr<SegmentInfo> segmentInfo,
                std::shared_ptr<IOContext> context)  override;

protected:
  std::shared_ptr<SimpleTextTermVectorsFormat> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextTermVectorsFormat>(
        org.apache.lucene.codecs.TermVectorsFormat::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/simpletext/
