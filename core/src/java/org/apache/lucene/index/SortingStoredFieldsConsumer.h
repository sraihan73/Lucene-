#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/TrackingTmpOutputDirectoryWrapper.h"

#include  "core/src/java/org/apache/lucene/index/DocumentsWriterPerThread.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/index/Sorter.h"
#include  "core/src/java/org/apache/lucene/index/DocMap.h"
#include  "core/src/java/org/apache/lucene/codecs/StoredFieldsWriter.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/IndexableFieldType.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using BytesRef = org::apache::lucene::util::BytesRef;

class SortingStoredFieldsConsumer final : public StoredFieldsConsumer
{
  GET_CLASS_NAME(SortingStoredFieldsConsumer)
public:
  std::shared_ptr<TrackingTmpOutputDirectoryWrapper> tmpDirectory;

  SortingStoredFieldsConsumer(
      std::shared_ptr<DocumentsWriterPerThread> docWriter);

protected:
  void initStoredFieldsWriter()  override;

public:
  void
  flush(std::shared_ptr<SegmentWriteState> state,
        std::shared_ptr<Sorter::DocMap> sortMap)  override;

  void abort() override;

  /**
   * A visitor that copies every field it sees in the provided {@link
   * StoredFieldsWriter}.
   */
private:
  class CopyVisitor : public StoredFieldVisitor, public IndexableField
  {
    GET_CLASS_NAME(CopyVisitor)
  public:
    const std::shared_ptr<StoredFieldsWriter> writer;
    // C++ NOTE: Fields cannot have the same name as methods:
    std::shared_ptr<BytesRef> binaryValue_;
    // C++ NOTE: Fields cannot have the same name as methods:
    std::wstring stringValue_;
    // C++ NOTE: Fields cannot have the same name as methods:
    std::shared_ptr<Number> numericValue_;
    std::shared_ptr<FieldInfo> currentField;

    CopyVisitor(std::shared_ptr<StoredFieldsWriter> writer);

    void binaryField(std::shared_ptr<FieldInfo> fieldInfo,
                     std::deque<char> &value)  override;

    void stringField(std::shared_ptr<FieldInfo> fieldInfo,
                     std::deque<char> &value)  override;

    void intField(std::shared_ptr<FieldInfo> fieldInfo,
                  int value)  override;

    void longField(std::shared_ptr<FieldInfo> fieldInfo,
                   int64_t value)  override;

    void floatField(std::shared_ptr<FieldInfo> fieldInfo,
                    float value)  override;

    void doubleField(std::shared_ptr<FieldInfo> fieldInfo,
                     double value)  override;

    Status needsField(std::shared_ptr<FieldInfo> fieldInfo) throw(
        IOException) override;

    std::wstring name() override;

    std::shared_ptr<IndexableFieldType> fieldType() override;

    std::shared_ptr<BytesRef> binaryValue() override;

    std::wstring stringValue() override;

    std::shared_ptr<Number> numericValue() override;

    std::shared_ptr<Reader> readerValue() override;

    std::shared_ptr<TokenStream>
    tokenStream(std::shared_ptr<Analyzer> analyzer,
                std::shared_ptr<TokenStream> reuse) override;

    virtual void reset(std::shared_ptr<FieldInfo> field);

    virtual void write() ;

  protected:
    std::shared_ptr<CopyVisitor> shared_from_this()
    {
      return std::static_pointer_cast<CopyVisitor>(
          StoredFieldVisitor::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SortingStoredFieldsConsumer> shared_from_this()
  {
    return std::static_pointer_cast<SortingStoredFieldsConsumer>(
        StoredFieldsConsumer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
