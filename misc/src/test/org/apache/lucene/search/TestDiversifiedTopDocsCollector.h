#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SortedDocValues;
}

namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::index
{
class BinaryDocValues;
}
namespace org::apache::lucene::search
{
class LeafCollector;
}
namespace org::apache::lucene::search
{
class Record;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class DiversifiedTopDocsCollector;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class ScoreDoc;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::index
{
class FieldInvertState;
}
namespace org::apache::lucene::search
{
class CollectionStatistics;
}
namespace org::apache::lucene::search
{
class TermStatistics;
}
namespace org::apache::lucene::search::similarities
{
class SimWeight;
}
namespace org::apache::lucene::search::similarities
{
class SimScorer;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::search
{
class Explanation;
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
namespace org::apache::lucene::search
{

using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Demonstrates an application of the {@link DiversifiedTopDocsCollector} in
 * assembling a collection of top results but without over-representation of any
 * one source (in this case top-selling singles from the 60s without having them
 * all be Beatles records...). Results are ranked by the number of weeks a
 * single is top of the charts and de-duped by the artist name.
 *
 */
class TestDiversifiedTopDocsCollector : public LuceneTestCase
{
  GET_CLASS_NAME(TestDiversifiedTopDocsCollector)

public:
  virtual void testNonDiversifiedResults() ;

  virtual void testFirstPageDiversifiedResults() ;

  virtual void testSecondPageResults() ;

  virtual void testInvalidArguments() ;

  // Diversifying collector that looks up de-dup keys using SortedDocValues
  // from a top-level Reader
private:
  class DocValuesDiversifiedCollector final : public DiversifiedTopDocsCollector
  {
    GET_CLASS_NAME(DocValuesDiversifiedCollector)
  private:
    const std::shared_ptr<SortedDocValues> sdv;

  public:
    DocValuesDiversifiedCollector(int size, int maxHitsPerKey,
                                  std::shared_ptr<SortedDocValues> sdv);

  protected:
    std::shared_ptr<NumericDocValues>
    getKeys(std::shared_ptr<LeafReaderContext> context) override;

  private:
    class NumericDocValuesAnonymousInnerClass : public NumericDocValues
    {
      GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<DocValuesDiversifiedCollector> outerInstance;

      std::shared_ptr<LeafReaderContext> context;

    public:
      NumericDocValuesAnonymousInnerClass(
          std::shared_ptr<DocValuesDiversifiedCollector> outerInstance,
          std::shared_ptr<LeafReaderContext> context);

      int docID() override;

      int nextDoc()  override;

      int advance(int target)  override;

      bool advanceExact(int target)  override;

      int64_t cost() override;

      int64_t longValue()  override;

    protected:
      std::shared_ptr<NumericDocValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass>(
            org.apache.lucene.index.NumericDocValues::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<DocValuesDiversifiedCollector> shared_from_this()
    {
      return std::static_pointer_cast<DocValuesDiversifiedCollector>(
          DiversifiedTopDocsCollector::shared_from_this());
    }
  };

  // Alternative, faster implementation for converting std::wstring keys to longs
  // but with the potential for hash collisions
private:
  class HashedDocValuesDiversifiedCollector final
      : public DiversifiedTopDocsCollector
  {
    GET_CLASS_NAME(HashedDocValuesDiversifiedCollector)

  private:
    const std::wstring field;
    std::shared_ptr<BinaryDocValues> vals;

  public:
    HashedDocValuesDiversifiedCollector(int size, int maxHitsPerKey,
                                        const std::wstring &field);

  protected:
    std::shared_ptr<NumericDocValues>
    getKeys(std::shared_ptr<LeafReaderContext> context) override;

  private:
    class NumericDocValuesAnonymousInnerClass : public NumericDocValues
    {
      GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<HashedDocValuesDiversifiedCollector> outerInstance;

    public:
      NumericDocValuesAnonymousInnerClass(
          std::shared_ptr<HashedDocValuesDiversifiedCollector> outerInstance);

      int docID() override;
      int nextDoc()  override;
      int advance(int target)  override;
      bool advanceExact(int target)  override;
      int64_t cost() override;
      int64_t longValue()  override;

    protected:
      std::shared_ptr<NumericDocValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass>(
            org.apache.lucene.index.NumericDocValues::shared_from_this());
      }
    };

  public:
    std::shared_ptr<LeafCollector> getLeafCollector(
        std::shared_ptr<LeafReaderContext> context)  override;

  protected:
    std::shared_ptr<HashedDocValuesDiversifiedCollector> shared_from_this()
    {
      return std::static_pointer_cast<HashedDocValuesDiversifiedCollector>(
          DiversifiedTopDocsCollector::shared_from_this());
    }
  };

  // Test data - format is artist, song, weeks at top of charts
private:
  static std::deque<std::wstring> hitsOfThe60s;

  static const std::unordered_map<std::wstring, std::shared_ptr<Record>>
      parsedRecords;
  std::shared_ptr<Directory> dir;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<SortedDocValues> artistDocValues;

public:
  class Record : public std::enable_shared_from_this<Record>
  {
    GET_CLASS_NAME(Record)
  public:
    std::wstring year;
    std::wstring artist;
    std::wstring song;
    float weeks = 0;
    std::wstring id;

    // C++ TODO: No base class can be determined:
    Record(const std::wstring &id, const std::wstring &year,
           const std::wstring &artist, const std::wstring &song,
           float weeks); // super();

    virtual std::wstring toString();
  };

private:
  std::shared_ptr<DiversifiedTopDocsCollector>
  doDiversifiedSearch(int numResults,
                      int maxResultsPerArtist) ;

  std::shared_ptr<DiversifiedTopDocsCollector>
  doFuzzyDiversifiedSearch(int numResults,
                           int maxResultsPerArtist) ;

  std::shared_ptr<DiversifiedTopDocsCollector>
  doAccurateDiversifiedSearch(int numResults,
                              int maxResultsPerArtist) ;

  std::shared_ptr<Query> getTestQuery();

public:
  void setUp()  override;

  void tearDown()  override;

private:
  int getMaxNumRecordsPerArtist(
      std::deque<std::shared_ptr<ScoreDoc>> &sd) ;

  /**
   * Similarity that wraps another similarity and replaces the final score
   * according to whats in a docvalues field.
   *
   * @lucene.experimental
   */
public:
  class DocValueSimilarity : public Similarity
  {
    GET_CLASS_NAME(DocValueSimilarity)
  private:
    const std::shared_ptr<Similarity> sim;
    const std::wstring scoreValueField;

  public:
    DocValueSimilarity(std::shared_ptr<Similarity> sim,
                       const std::wstring &scoreValueField);

    int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override;

    std::shared_ptr<Similarity::SimWeight>
    computeWeight(float boost,
                  std::shared_ptr<CollectionStatistics> collectionStats,
                  std::deque<TermStatistics> &termStats) override;

    std::shared_ptr<Similarity::SimScorer> simScorer(
        std::shared_ptr<Similarity::SimWeight> stats,
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class SimScorerAnonymousInnerClass : public SimScorer
    {
      GET_CLASS_NAME(SimScorerAnonymousInnerClass)
    private:
      std::shared_ptr<DocValueSimilarity> outerInstance;

      std::shared_ptr<Similarity::SimScorer> sub;
      std::shared_ptr<NumericDocValues> values;

    public:
      SimScorerAnonymousInnerClass(
          std::shared_ptr<DocValueSimilarity> outerInstance,
          std::shared_ptr<Similarity::SimScorer> sub,
          std::shared_ptr<NumericDocValues> values);

      float score(int doc, float freq)  override;

      float computeSlopFactor(int distance) override;

      float computePayloadFactor(int doc, int start, int end,
                                 std::shared_ptr<BytesRef> payload) override;

      std::shared_ptr<Explanation>
      explain(int doc,
              std::shared_ptr<Explanation> freq)  override;

    protected:
      std::shared_ptr<SimScorerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SimScorerAnonymousInnerClass>(
            SimScorer::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<DocValueSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<DocValueSimilarity>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestDiversifiedTopDocsCollector> shared_from_this()
  {
    return std::static_pointer_cast<TestDiversifiedTopDocsCollector>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
