#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class Counter;
}

namespace org::apache::lucene::index
{
class DocumentsWriterPerThread;
}
namespace org::apache::lucene::index
{
class DocState;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class Builder;
}
namespace org::apache::lucene::index
{
class TermsHash;
}
namespace org::apache::lucene::index
{
class StoredFieldsConsumer;
}
namespace org::apache::lucene::index
{
class PerField;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::index
{
class Sorter;
}
namespace org::apache::lucene::index
{
class DocMap;
}
namespace org::apache::lucene::index
{
class IndexableField;
}
namespace org::apache::lucene::index
{
class IndexableFieldType;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::index
{
class FieldInvertState;
}
namespace org::apache::lucene::index
{
class TermsHashPerField;
}
namespace org::apache::lucene::index
{
class DocValuesWriter;
}
namespace org::apache::lucene::index
{
class PointValuesWriter;
}
namespace org::apache::lucene::index
{
class NormValuesWriter;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
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
namespace org::apache::lucene::index
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Counter = org::apache::lucene::util::Counter;

/** Default general purpose indexing chain, which handles
 *  indexing all types of fields. */
class DefaultIndexingChain final : public DocConsumer
{
  GET_CLASS_NAME(DefaultIndexingChain)
public:
  const std::shared_ptr<Counter> bytesUsed;
  const std::shared_ptr<DocumentsWriterPerThread::DocState> docState;
  const std::shared_ptr<DocumentsWriterPerThread> docWriter;
  const std::shared_ptr<FieldInfos::Builder> fieldInfos;

  // Writes postings and term vectors:
  const std::shared_ptr<TermsHash> termsHash;
  // Writes stored fields
  const std::shared_ptr<StoredFieldsConsumer> storedFieldsConsumer;

  // NOTE: I tried using Hash Map<std::wstring,PerField>
  // but it was ~2% slower on Wiki and Geonames with Java
  // 1.7.0_25:
private:
  std::deque<std::shared_ptr<PerField>> fieldHash =
      std::deque<std::shared_ptr<PerField>>(2);
  int hashMask = 1;

  int totalFieldCount = 0;
  int64_t nextFieldGen = 0;

  // Holds fields seen in each document
  std::deque<std::shared_ptr<PerField>> fields =
      std::deque<std::shared_ptr<PerField>>(1);

  const std::shared_ptr<Set<std::wstring>> finishedDocValues =
      std::unordered_set<std::wstring>();

public:
  DefaultIndexingChain(
      std::shared_ptr<DocumentsWriterPerThread> docWriter) ;

private:
  std::shared_ptr<Sorter::DocMap>
  maybeSortSegment(std::shared_ptr<SegmentWriteState> state) ;

public:
  std::shared_ptr<Sorter::DocMap>
  flush(std::shared_ptr<SegmentWriteState> state)  override;

  /** Writes all buffered points. */
private:
  void writePoints(std::shared_ptr<SegmentWriteState> state,
                   std::shared_ptr<Sorter::DocMap> sortMap) ;

  /** Writes all buffered doc values (called from {@link #flush}). */
  void
  writeDocValues(std::shared_ptr<SegmentWriteState> state,
                 std::shared_ptr<Sorter::DocMap> sortMap) ;

  void writeNorms(std::shared_ptr<SegmentWriteState> state,
                  std::shared_ptr<Sorter::DocMap> sortMap) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @SuppressWarnings("try") public void abort()
  // throws java.io.IOException
  void abort()  override;

private:
  void rehash();

  /** Calls StoredFieldsWriter.startDocument, aborting the
   *  segment if it hits any exception. */
  void startStoredFields(int docID) ;

  /** Calls StoredFieldsWriter.finishDocument, aborting the
   *  segment if it hits any exception. */
  void finishStoredFields() ;

public:
  void processDocument()  override;

private:
  int processField(std::shared_ptr<IndexableField> field, int64_t fieldGen,
                   int fieldCount) ;

  static void verifyUnIndexedFieldType(const std::wstring &name,
                                       std::shared_ptr<IndexableFieldType> ft);

  /** Called from processDocument to index one field's point */
  void indexPoint(std::shared_ptr<PerField> fp,
                  std::shared_ptr<IndexableField> field) ;

  /** Called from processDocument to index one field's doc value */
  void indexDocValue(std::shared_ptr<PerField> fp, DocValuesType dvType,
                     std::shared_ptr<IndexableField> field) ;

  /** Returns a previously created {@link PerField}, or null
   *  if this field name wasn't seen yet. */
  std::shared_ptr<PerField> getPerField(const std::wstring &name);

  /** Returns a previously created {@link PerField},
   *  absorbing the type information from {@link FieldType},
   *  and creates a new {@link PerField} if this field name
   *  wasn't seen yet. */
  std::shared_ptr<PerField>
  getOrAddField(const std::wstring &name,
                std::shared_ptr<IndexableFieldType> fieldType, bool invert);

  /** NOTE: not static: accesses at least docState, termsHash. */
private:
  class PerField final : public std::enable_shared_from_this<PerField>,
                         public Comparable<std::shared_ptr<PerField>>
  {
    GET_CLASS_NAME(PerField)
  private:
    std::shared_ptr<DefaultIndexingChain> outerInstance;

  public:
    const int indexCreatedVersionMajor;
    const std::shared_ptr<FieldInfo> fieldInfo;
    const std::shared_ptr<Similarity> similarity;

    std::shared_ptr<FieldInvertState> invertState;
    std::shared_ptr<TermsHashPerField> termsHashPerField;

    // Non-null if this field ever had doc values in this
    // segment:
    std::shared_ptr<DocValuesWriter> docValuesWriter;

    // Non-null if this field ever had points in this segment:
    std::shared_ptr<PointValuesWriter> pointValuesWriter;

    /** We use this to know when a PerField is seen for the
     *  first time in the current document. */
    int64_t fieldGen = -1;

    // Used by the hash table
    std::shared_ptr<PerField> next;

    // Lazy init'd:
    std::shared_ptr<NormValuesWriter> norms;

    // reused
    std::shared_ptr<TokenStream> tokenStream;

    PerField(std::shared_ptr<DefaultIndexingChain> outerInstance,
             int indexCreatedVersionMajor, std::shared_ptr<FieldInfo> fieldInfo,
             bool invert);

    void setInvertState();

    int compareTo(std::shared_ptr<PerField> other) override;

    void finish() ;

    /** Inverts one field for one document; first is true
     *  if this is the first time we are seeing this field
     *  name in this document. */
    void invert(std::shared_ptr<IndexableField> field,
                bool first) ;
  };

public:
  std::shared_ptr<DocIdSetIterator>
  getHasDocValues(const std::wstring &field) override;

protected:
  std::shared_ptr<DefaultIndexingChain> shared_from_this()
  {
    return std::static_pointer_cast<DefaultIndexingChain>(
        DocConsumer::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
