#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/TermVectorsWriter.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/DocumentsWriterPerThread.h"
#include  "core/src/java/org/apache/lucene/index/ByteSliceReader.h"
#include  "core/src/java/org/apache/lucene/index/TermVectorsConsumerPerField.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/index/Sorter.h"
#include  "core/src/java/org/apache/lucene/index/DocMap.h"
#include  "core/src/java/org/apache/lucene/index/TermsHashPerField.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/FieldInvertState.h"

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
using BytesRef = org::apache::lucene::util::BytesRef;

class TermVectorsConsumer : public TermsHash
{
  GET_CLASS_NAME(TermVectorsConsumer)
public:
  std::shared_ptr<TermVectorsWriter> writer;

  /** Scratch term used by TermVectorsConsumerPerField.finishDocument. */
  const std::shared_ptr<BytesRef> flushTerm = std::make_shared<BytesRef>();

  const std::shared_ptr<DocumentsWriterPerThread> docWriter;

  /** Used by TermVectorsConsumerPerField when serializing
   *  the term vectors. */
  const std::shared_ptr<ByteSliceReader> vectorSliceReaderPos =
      std::make_shared<ByteSliceReader>();
  const std::shared_ptr<ByteSliceReader> vectorSliceReaderOff =
      std::make_shared<ByteSliceReader>();

  bool hasVectors = false;
  int numVectorFields = 0;
  int lastDocID = 0;

private:
  std::deque<std::shared_ptr<TermVectorsConsumerPerField>> perFields =
      std::deque<std::shared_ptr<TermVectorsConsumerPerField>>(1);

public:
  TermVectorsConsumer(std::shared_ptr<DocumentsWriterPerThread> docWriter);

  void
  flush(std::unordered_map<std::wstring, std::shared_ptr<TermsHashPerField>>
            &fieldsToFlush,
        std::shared_ptr<SegmentWriteState> state,
        std::shared_ptr<Sorter::DocMap> sortMap)  override;

  /** Fills in no-term-vectors for all docs we haven't seen
   *  since the last doc that had term vectors. */
  virtual void fill(int docID) ;

  virtual void initTermVectorsWriter() ;

  void finishDocument()  override;

  void abort() override;

  virtual void resetFields();

  std::shared_ptr<TermsHashPerField>
  addField(std::shared_ptr<FieldInvertState> invertState,
           std::shared_ptr<FieldInfo> fieldInfo) override;

  virtual void
  addFieldToFlush(std::shared_ptr<TermVectorsConsumerPerField> fieldToFlush);

  void startDocument() override;

protected:
  std::shared_ptr<TermVectorsConsumer> shared_from_this()
  {
    return std::static_pointer_cast<TermVectorsConsumer>(
        TermsHash::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
