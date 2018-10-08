#pragma once
#include "stringhelper.h"
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/PointsFormat.h"

#include  "core/src/java/org/apache/lucene/codecs/PointsWriter.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/index/MergeState.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsReader.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/search/SortField.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/index/FieldInvertState.h"
#include  "core/src/java/org/apache/lucene/search/CollectionStatistics.h"
#include  "core/src/java/org/apache/lucene/search/TermStatistics.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimWeight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimScorer.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/search/Sort.h"

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
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using Document = org::apache::lucene::document::Document;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

class TestIndexSorting : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexSorting)
public:
  class AssertingNeedsIndexSortCodec : public FilterCodec
  {
    GET_CLASS_NAME(AssertingNeedsIndexSortCodec)
  public:
    bool needsIndexSort = false;
    int numCalls = 0;

    AssertingNeedsIndexSortCodec();

    std::shared_ptr<PointsFormat> pointsFormat() override;

  private:
    class PointsFormatAnonymousInnerClass : public PointsFormat
    {
      GET_CLASS_NAME(PointsFormatAnonymousInnerClass)
    private:
      std::shared_ptr<AssertingNeedsIndexSortCodec> outerInstance;

      std::shared_ptr<PointsFormat> pf;

    public:
      PointsFormatAnonymousInnerClass(
          std::shared_ptr<AssertingNeedsIndexSortCodec> outerInstance,
          std::shared_ptr<PointsFormat> pf);

      std::shared_ptr<PointsWriter> fieldsWriter(
          std::shared_ptr<SegmentWriteState> state)  override;

    private:
      class PointsWriterAnonymousInnerClass : public PointsWriter
      {
        GET_CLASS_NAME(PointsWriterAnonymousInnerClass)
      private:
        std::shared_ptr<PointsFormatAnonymousInnerClass> outerInstance;

        std::shared_ptr<PointsWriter> writer;

      public:
        PointsWriterAnonymousInnerClass(
            std::shared_ptr<PointsFormatAnonymousInnerClass> outerInstance,
            std::shared_ptr<PointsWriter> writer);

        void merge(std::shared_ptr<MergeState> mergeState) throw(
            IOException) override;

        void writeField(
            std::shared_ptr<FieldInfo> fieldInfo,
            std::shared_ptr<PointsReader> values)  override;

        void finish()  override;

        virtual ~PointsWriterAnonymousInnerClass();

      protected:
        std::shared_ptr<PointsWriterAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<PointsWriterAnonymousInnerClass>(
              org.apache.lucene.codecs.PointsWriter::shared_from_this());
        }
      };

    public:
      std::shared_ptr<PointsReader> fieldsReader(
          std::shared_ptr<SegmentReadState> state)  override;

    protected:
      std::shared_ptr<PointsFormatAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<PointsFormatAnonymousInnerClass>(
            org.apache.lucene.codecs.PointsFormat::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<AssertingNeedsIndexSortCodec> shared_from_this()
    {
      return std::static_pointer_cast<AssertingNeedsIndexSortCodec>(
          org.apache.lucene.codecs.FilterCodec::shared_from_this());
    }
  };

private:
  static void assertNeedsIndexSortMerge(
      std::shared_ptr<SortField> sortField,
      std::function<void(Document *)> &defaultValueConsumer,
      std::function<void(Document *)>
          &randomValueConsumer) ;

public:
  virtual void testNumericAlreadySorted() ;

  virtual void testStringAlreadySorted() ;

  virtual void testMultiValuedNumericAlreadySorted() ;

  virtual void testMultiValuedStringAlreadySorted() ;

  virtual void testBasicString() ;

  virtual void testBasicMultiValuedString() ;

  virtual void testMissingStringFirst() ;

  virtual void testMissingMultiValuedStringFirst() ;

  virtual void testMissingStringLast() ;

  virtual void testMissingMultiValuedStringLast() ;

  virtual void testBasicLong() ;

  virtual void testBasicMultiValuedLong() ;

  virtual void testMissingLongFirst() ;

  virtual void testMissingMultiValuedLongFirst() ;

  virtual void testMissingLongLast() ;

  virtual void testMissingMultiValuedLongLast() ;

  virtual void testBasicInt() ;

  virtual void testBasicMultiValuedInt() ;

  virtual void testMissingIntFirst() ;

  virtual void testMissingMultiValuedIntFirst() ;

  virtual void testMissingIntLast() ;

  virtual void testMissingMultiValuedIntLast() ;

  virtual void testBasicDouble() ;

  virtual void testBasicMultiValuedDouble() ;

  virtual void testMissingDoubleFirst() ;

  virtual void testMissingMultiValuedDoubleFirst() ;

  virtual void testMissingDoubleLast() ;

  virtual void testMissingMultiValuedDoubleLast() ;

  virtual void testBasicFloat() ;

  virtual void testBasicMultiValuedFloat() ;

  virtual void testMissingFloatFirst() ;

  virtual void testMissingMultiValuedFloatFirst() ;

  virtual void testMissingFloatLast() ;

  virtual void testMissingMultiValuedFloatLast() ;

  virtual void testRandom1() ;

  virtual void testMultiValuedRandom1() ;

public:
  class UpdateRunnable : public std::enable_shared_from_this<UpdateRunnable>,
                         public Runnable
  {
    GET_CLASS_NAME(UpdateRunnable)

  private:
    const int numDocs;
    const std::shared_ptr<Random> random;
    const std::shared_ptr<AtomicInteger> updateCount;
    const std::shared_ptr<IndexWriter> w;
    const std::unordered_map<int, int64_t> values;
    const std::shared_ptr<CountDownLatch> latch;

  public:
    UpdateRunnable(int numDocs, std::shared_ptr<Random> random,
                   std::shared_ptr<CountDownLatch> latch,
                   std::shared_ptr<AtomicInteger> updateCount,
                   std::shared_ptr<IndexWriter> w,
                   std::unordered_map<int, int64_t> &values);

    void run() override;
  };

  // There is tricky logic to resolve deletes that happened while merging
public:
  virtual void testConcurrentUpdates() ;

  // docvalues fields involved in the index sort cannot be updated
  virtual void testBadDVUpdate() ;

public:
  class DVUpdateRunnable
      : public std::enable_shared_from_this<DVUpdateRunnable>,
        public Runnable
  {
    GET_CLASS_NAME(DVUpdateRunnable)

  private:
    const int numDocs;
    const std::shared_ptr<Random> random;
    const std::shared_ptr<AtomicInteger> updateCount;
    const std::shared_ptr<IndexWriter> w;
    const std::unordered_map<int, int64_t> values;
    const std::shared_ptr<CountDownLatch> latch;

  public:
    DVUpdateRunnable(int numDocs, std::shared_ptr<Random> random,
                     std::shared_ptr<CountDownLatch> latch,
                     std::shared_ptr<AtomicInteger> updateCount,
                     std::shared_ptr<IndexWriter> w,
                     std::unordered_map<int, int64_t> &values);

    void run() override;
  };

  // There is tricky logic to resolve dv updates that happened while merging
public:
  virtual void testConcurrentDVUpdates() ;

  virtual void testAddIndexes(bool withDeletes,
                              bool useReaders) ;

  virtual void testAddIndexes() ;

  virtual void testAddIndexesWithDeletions() ;

  virtual void testAddIndexesWithDirectory() ;

  virtual void
  testAddIndexesWithDeletionsAndDirectory() ;

  virtual void testBadSort() ;

  // you can't change the index sort on an existing index:
  virtual void testIllegalChangeSort() ;

public:
  class NormsSimilarity final : public Similarity
  {
    GET_CLASS_NAME(NormsSimilarity)

  private:
    const std::shared_ptr<Similarity> in_;

  public:
    NormsSimilarity(std::shared_ptr<Similarity> in_);

    int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override;

    std::shared_ptr<Similarity::SimWeight>
    computeWeight(float boost,
                  std::shared_ptr<CollectionStatistics> collectionStats,
                  std::deque<TermStatistics> &termStats) override;

    std::shared_ptr<Similarity::SimScorer> simScorer(
        std::shared_ptr<Similarity::SimWeight> weight,
        std::shared_ptr<LeafReaderContext> context)  override;

  protected:
    std::shared_ptr<NormsSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<NormsSimilarity>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

public:
  class PositionsTokenStream final : public TokenStream
  {
    GET_CLASS_NAME(PositionsTokenStream)

  private:
    const std::shared_ptr<CharTermAttribute> term;
    const std::shared_ptr<PayloadAttribute> payload;
    const std::shared_ptr<OffsetAttribute> offset;

    int pos = 0, off = 0;

  public:
    PositionsTokenStream();

    bool incrementToken()  override;

    void setId(int id);

  protected:
    std::shared_ptr<PositionsTokenStream> shared_from_this()
    {
      return std::static_pointer_cast<PositionsTokenStream>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

public:
  virtual void testRandom2() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexSorting> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestIndexSorting> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class RandomDoc final : public std::enable_shared_from_this<RandomDoc>
  {
    GET_CLASS_NAME(RandomDoc)
  public:
    const int id;
    const int intValue;
    std::deque<int> const intValues;
    const int64_t longValue;
    std::deque<int64_t> const longValues;
    const float floatValue;
    std::deque<float> const floatValues;
    const double doubleValue;
    std::deque<double> const doubleValues;
    std::deque<char> const bytesValue;
    std::deque<std::deque<char>> const bytesValues;

    RandomDoc(int id);
  };

private:
  static std::shared_ptr<SortField> randomIndexSortField();

  static std::shared_ptr<Sort> randomSort();

  // pits index time sorting against query time sorting
public:
  virtual void testRandom3() ;

  virtual void testTieBreak() ;

  virtual void testIndexSortWithSparseField() ;

  virtual void testIndexSortOnSparseField() ;

protected:
  std::shared_ptr<TestIndexSorting> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexSorting>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
