#pragma once
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
namespace org::apache::lucene::codecs
{

using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

/**
 * Controls the format of term vectors
 */
class TermVectorsFormat : public std::enable_shared_from_this<TermVectorsFormat>
{
  GET_CLASS_NAME(TermVectorsFormat)
  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  TermVectorsFormat();

  /** Returns a {@link TermVectorsReader} to read term
   *  vectors. */
public:
  virtual std::shared_ptr<TermVectorsReader>
  vectorsReader(std::shared_ptr<Directory> directory,
                std::shared_ptr<SegmentInfo> segmentInfo,
                std::shared_ptr<FieldInfos> fieldInfos,
                std::shared_ptr<IOContext> context) = 0;

  /** Returns a {@link TermVectorsWriter} to write term
   *  vectors. */
  virtual std::shared_ptr<TermVectorsWriter>
  vectorsWriter(std::shared_ptr<Directory> directory,
                std::shared_ptr<SegmentInfo> segmentInfo,
                std::shared_ptr<IOContext> context) = 0;
};

} // #include  "core/src/java/org/apache/lucene/codecs/
