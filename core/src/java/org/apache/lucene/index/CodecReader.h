#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/StoredFieldsReader.h"

#include  "core/src/java/org/apache/lucene/codecs/TermVectorsReader.h"
#include  "core/src/java/org/apache/lucene/codecs/NormsProducer.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsReader.h"
#include  "core/src/java/org/apache/lucene/index/StoredFieldVisitor.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
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
namespace org::apache::lucene::index
{

using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using Accountable = org::apache::lucene::util::Accountable;

/**
 * LeafReader implemented by codec APIs.
 */
class CodecReader : public LeafReader, public Accountable
{
  GET_CLASS_NAME(CodecReader)

  /** Sole constructor. (For invocation by subclass
   * constructors, typically implicit.) */
protected:
  CodecReader();

  /**
   * Expert: retrieve thread-private StoredFieldsReader
   * @lucene.internal
   */
public:
  virtual std::shared_ptr<StoredFieldsReader> getFieldsReader() = 0;

  /**
   * Expert: retrieve thread-private TermVectorsReader
   * @lucene.internal
   */
  virtual std::shared_ptr<TermVectorsReader> getTermVectorsReader() = 0;

  /**
   * Expert: retrieve underlying NormsProducer
   * @lucene.internal
   */
  virtual std::shared_ptr<NormsProducer> getNormsReader() = 0;

  /**
   * Expert: retrieve underlying DocValuesProducer
   * @lucene.internal
   */
  virtual std::shared_ptr<DocValuesProducer> getDocValuesReader() = 0;

  /**
   * Expert: retrieve underlying FieldsProducer
   * @lucene.internal
   */
  virtual std::shared_ptr<FieldsProducer> getPostingsReader() = 0;

  /**
   * Expert: retrieve underlying PointsReader
   * @lucene.internal
   */
  virtual std::shared_ptr<PointsReader> getPointsReader() = 0;

  void document(int docID, std::shared_ptr<StoredFieldVisitor> visitor) throw(
      IOException) override final;

  std::shared_ptr<Fields>
  getTermVectors(int docID)  override final;

private:
  void checkBounds(int docID);

public:
  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override final;

  // returns the FieldInfo that corresponds to the given field and type, or
  // null if the field does not exist, or not indexed as the requested
  // DovDocValuesType.
private:
  std::shared_ptr<FieldInfo> getDVField(const std::wstring &field,
                                        DocValuesType type);

public:
  std::shared_ptr<NumericDocValues> getNumericDocValues(
      const std::wstring &field)  override final;

  std::shared_ptr<BinaryDocValues> getBinaryDocValues(
      const std::wstring &field)  override final;

  std::shared_ptr<SortedDocValues> getSortedDocValues(
      const std::wstring &field)  override final;

  std::shared_ptr<SortedNumericDocValues> getSortedNumericDocValues(
      const std::wstring &field)  override final;

  std::shared_ptr<SortedSetDocValues> getSortedSetDocValues(
      const std::wstring &field)  override final;

  std::shared_ptr<NumericDocValues>
  getNormValues(const std::wstring &field)  override final;

  std::shared_ptr<PointValues>
  getPointValues(const std::wstring &field)  override final;

protected:
  void doClose()  override;

public:
  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  void checkIntegrity()  override;

protected:
  std::shared_ptr<CodecReader> shared_from_this()
  {
    return std::static_pointer_cast<CodecReader>(
        LeafReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
