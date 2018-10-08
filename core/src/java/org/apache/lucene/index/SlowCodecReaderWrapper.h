#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/CodecReader.h"

#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/codecs/TermVectorsReader.h"
#include  "core/src/java/org/apache/lucene/codecs/StoredFieldsReader.h"
#include  "core/src/java/org/apache/lucene/codecs/NormsProducer.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsReader.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/index/LeafMetaData.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include  "core/src/java/org/apache/lucene/index/StoredFieldVisitor.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"

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

/**
 * Wraps arbitrary readers for merging. Note that this can cause slow
 * and memory-intensive merges. Consider using {@link FilterCodecReader}
 * instead.
 */
class SlowCodecReaderWrapper final
    : public std::enable_shared_from_this<SlowCodecReaderWrapper>
{
  GET_CLASS_NAME(SlowCodecReaderWrapper)

  /** No instantiation */
private:
  SlowCodecReaderWrapper();

  /**
   * Returns a {@code CodecReader} view of reader.
   * <p>
   * If {@code reader} is already a {@code CodecReader}, it is returned
   * directly. Otherwise, a (slow) view is returned.
   */
public:
  static std::shared_ptr<CodecReader>
  wrap(std::shared_ptr<LeafReader> reader) ;

private:
  class CodecReaderAnonymousInnerClass : public CodecReader
  {
    GET_CLASS_NAME(CodecReaderAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::index::LeafReader> reader;

  public:
    CodecReaderAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::index::LeafReader> reader);

    std::shared_ptr<TermVectorsReader> getTermVectorsReader() override;

    std::shared_ptr<StoredFieldsReader> getFieldsReader() override;

    std::shared_ptr<NormsProducer> getNormsReader() override;

    std::shared_ptr<DocValuesProducer> getDocValuesReader() override;

    std::shared_ptr<FieldsProducer> getPostingsReader() override;

    std::shared_ptr<FieldInfos> getFieldInfos() override;

    std::shared_ptr<PointsReader> getPointsReader() override;

    std::shared_ptr<Bits> getLiveDocs() override;

    int numDocs() override;

    int maxDoc() override;

    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

    virtual std::wstring toString();

    std::shared_ptr<LeafMetaData> getMetaData() override;

  protected:
    std::shared_ptr<CodecReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CodecReaderAnonymousInnerClass>(
          CodecReader::shared_from_this());
    }
  };

private:
  static std::shared_ptr<PointsReader>
  pointValuesToReader(std::shared_ptr<LeafReader> reader);

private:
  class PointsReaderAnonymousInnerClass : public PointsReader
  {
    GET_CLASS_NAME(PointsReaderAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::index::LeafReader> reader;

  public:
    PointsReaderAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::index::LeafReader> reader);

    std::shared_ptr<PointValues>
    getValues(const std::wstring &field)  override;

    void checkIntegrity()  override;

    virtual ~PointsReaderAnonymousInnerClass();

    int64_t ramBytesUsed() override;

  protected:
    std::shared_ptr<PointsReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PointsReaderAnonymousInnerClass>(
          org.apache.lucene.codecs.PointsReader::shared_from_this());
    }
  };

private:
  static std::shared_ptr<NormsProducer>
  readerToNormsProducer(std::shared_ptr<LeafReader> reader);

private:
  class NormsProducerAnonymousInnerClass : public NormsProducer
  {
    GET_CLASS_NAME(NormsProducerAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::index::LeafReader> reader;

  public:
    NormsProducerAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::index::LeafReader> reader);

    std::shared_ptr<NumericDocValues>
    getNorms(std::shared_ptr<FieldInfo> field)  override;

    void checkIntegrity()  override;

    virtual ~NormsProducerAnonymousInnerClass();

    int64_t ramBytesUsed() override;

  protected:
    std::shared_ptr<NormsProducerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<NormsProducerAnonymousInnerClass>(
          org.apache.lucene.codecs.NormsProducer::shared_from_this());
    }
  };

private:
  static std::shared_ptr<DocValuesProducer>
  readerToDocValuesProducer(std::shared_ptr<LeafReader> reader);

private:
  class DocValuesProducerAnonymousInnerClass : public DocValuesProducer
  {
    GET_CLASS_NAME(DocValuesProducerAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::index::LeafReader> reader;

  public:
    DocValuesProducerAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::index::LeafReader> reader);

    std::shared_ptr<NumericDocValues>
    getNumeric(std::shared_ptr<FieldInfo> field)  override;

    std::shared_ptr<BinaryDocValues>
    getBinary(std::shared_ptr<FieldInfo> field)  override;

    std::shared_ptr<SortedDocValues>
    getSorted(std::shared_ptr<FieldInfo> field)  override;

    std::shared_ptr<SortedNumericDocValues> getSortedNumeric(
        std::shared_ptr<FieldInfo> field)  override;

    std::shared_ptr<SortedSetDocValues>
    getSortedSet(std::shared_ptr<FieldInfo> field)  override;

    void checkIntegrity()  override;

    virtual ~DocValuesProducerAnonymousInnerClass();

    int64_t ramBytesUsed() override;

  protected:
    std::shared_ptr<DocValuesProducerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocValuesProducerAnonymousInnerClass>(
          org.apache.lucene.codecs.DocValuesProducer::shared_from_this());
    }
  };

private:
  static std::shared_ptr<StoredFieldsReader>
  readerToStoredFieldsReader(std::shared_ptr<LeafReader> reader);

private:
  class StoredFieldsReaderAnonymousInnerClass : public StoredFieldsReader
  {
    GET_CLASS_NAME(StoredFieldsReaderAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::index::LeafReader> reader;

  public:
    StoredFieldsReaderAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::index::LeafReader> reader);

    void
    visitDocument(int docID, std::shared_ptr<StoredFieldVisitor> visitor) throw(
        IOException) override;

    std::shared_ptr<StoredFieldsReader> clone() override;

    void checkIntegrity()  override;

    virtual ~StoredFieldsReaderAnonymousInnerClass();

    int64_t ramBytesUsed() override;

  protected:
    std::shared_ptr<StoredFieldsReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<StoredFieldsReaderAnonymousInnerClass>(
          org.apache.lucene.codecs.StoredFieldsReader::shared_from_this());
    }
  };

private:
  static std::shared_ptr<TermVectorsReader>
  readerToTermVectorsReader(std::shared_ptr<LeafReader> reader);

private:
  class TermVectorsReaderAnonymousInnerClass : public TermVectorsReader
  {
    GET_CLASS_NAME(TermVectorsReaderAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::index::LeafReader> reader;

  public:
    TermVectorsReaderAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::index::LeafReader> reader);

    std::shared_ptr<Fields> get(int docID)  override;

    std::shared_ptr<TermVectorsReader> clone() override;

    void checkIntegrity()  override;

    virtual ~TermVectorsReaderAnonymousInnerClass();

    int64_t ramBytesUsed() override;

  protected:
    std::shared_ptr<TermVectorsReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TermVectorsReaderAnonymousInnerClass>(
          org.apache.lucene.codecs.TermVectorsReader::shared_from_this());
    }
  };

private:
  static std::shared_ptr<FieldsProducer>
  readerToFieldsProducer(std::shared_ptr<LeafReader> reader) ;

private:
  class FieldsProducerAnonymousInnerClass : public FieldsProducer
  {
    GET_CLASS_NAME(FieldsProducerAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::index::LeafReader> reader;
    std::deque<std::wstring> indexedFields;

  public:
    FieldsProducerAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::index::LeafReader> reader,
        std::deque<std::wstring> &indexedFields);

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

    int size() override;

    void checkIntegrity()  override;

    virtual ~FieldsProducerAnonymousInnerClass();

    int64_t ramBytesUsed() override;

  protected:
    std::shared_ptr<FieldsProducerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FieldsProducerAnonymousInnerClass>(
          org.apache.lucene.codecs.FieldsProducer::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/index/
