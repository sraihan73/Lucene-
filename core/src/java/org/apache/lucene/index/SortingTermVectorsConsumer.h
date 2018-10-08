#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/TrackingTmpOutputDirectoryWrapper.h"

#include  "core/src/java/org/apache/lucene/index/DocumentsWriterPerThread.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/index/Sorter.h"
#include  "core/src/java/org/apache/lucene/index/DocMap.h"
#include  "core/src/java/org/apache/lucene/index/TermsHashPerField.h"
#include  "core/src/java/org/apache/lucene/codecs/TermVectorsWriter.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"

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
namespace org::apache::lucene::index
{

using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;

class SortingTermVectorsConsumer final : public TermVectorsConsumer
{
  GET_CLASS_NAME(SortingTermVectorsConsumer)
public:
  std::shared_ptr<TrackingTmpOutputDirectoryWrapper> tmpDirectory;

  SortingTermVectorsConsumer(
      std::shared_ptr<DocumentsWriterPerThread> docWriter);

  void
  flush(std::unordered_map<std::wstring, std::shared_ptr<TermsHashPerField>>
            &fieldsToFlush,
        std::shared_ptr<SegmentWriteState> state,
        std::shared_ptr<Sorter::DocMap> sortMap)  override;

  void initTermVectorsWriter()  override;

  void abort() override;

  /** Safe (but, slowish) default method to copy every deque field in the
   * provided {@link TermVectorsWriter}. */
private:
  static void
  writeTermVectors(std::shared_ptr<TermVectorsWriter> writer,
                   std::shared_ptr<Fields> vectors,
                   std::shared_ptr<FieldInfos> fieldInfos) ;

protected:
  std::shared_ptr<SortingTermVectorsConsumer> shared_from_this()
  {
    return std::static_pointer_cast<SortingTermVectorsConsumer>(
        TermVectorsConsumer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
