#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class ScoreDoc;
}
namespace org::apache::lucene::search
{
class TopDocs;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class Sort;
}
namespace org::apache::lucene::search
{
class TopFieldDocs;
}
namespace org::apache::lucene::search
{
class Collector;
}
namespace org::apache::lucene::search
{
class Weight;
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

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static junit.framework.Assert.assertNotNull;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertNull;

/**
 * Utility class for asserting expected hits in tests.
 */
class CheckHits : public std::enable_shared_from_this<CheckHits>
{
  GET_CLASS_NAME(CheckHits)

  /**
   * Some explains methods calculate their values though a slightly
   * different  order of operations from the actual scoring method ...
   * this allows for a small amount of relative variation
   */
public:
  static float EXPLAIN_SCORE_TOLERANCE_DELTA;

  /**
   * In general we use a relative epsilon, but some tests do crazy things
   * like boost documents with 0, creating tiny tiny scores where the
   * relative difference is large but the absolute difference is tiny.
   * we ensure the the epsilon is always at least this big.
   */
  static float EXPLAIN_SCORE_TOLERANCE_MINIMUM;

  /**
   * Tests that all documents up to maxDoc which are *not* in the
   * expected result set, have an explanation which indicates that
   * the document does not match
   */
  static void
  checkNoMatchExplanations(std::shared_ptr<Query> q,
                           const std::wstring &defaultFieldName,
                           std::shared_ptr<IndexSearcher> searcher,
                           std::deque<int> &results) ;

  /**
   * Tests that a query matches the an expected set of documents using a
   * HitCollector.
   * <p>
   * Note that when using the HitCollector API, documents will be collected
   * if they "match" regardless of what their score is.
   * </p>
   *
   * @param query            the query to test
   * @param searcher         the searcher to test the query against
   * @param defaultFieldName used for displaying the query in assertion messages
   * @param results          a deque of documentIds that must match the query
   * @see #checkHits
   */
  static void checkHitCollector(std::shared_ptr<Random> random,
                                std::shared_ptr<Query> query,
                                const std::wstring &defaultFieldName,
                                std::shared_ptr<IndexSearcher> searcher,
                                std::deque<int> &results) ;

  /**
   * Just collects document ids into a set.
   */
public:
  class SetCollector : public SimpleCollector
  {
    GET_CLASS_NAME(SetCollector)
  public:
    const std::shared_ptr<Set<int>> bag;

    SetCollector(std::shared_ptr<Set<int>> bag);

  private:
    int base = 0;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer)  override;

    void collect(int doc) override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    bool needsScores() override;

  protected:
    std::shared_ptr<SetCollector> shared_from_this()
    {
      return std::static_pointer_cast<SetCollector>(
          SimpleCollector::shared_from_this());
    }
  };

  /**
   * Tests that a query matches the an expected set of documents using Hits.
   * <p>
   * Note that when using the Hits API, documents will only be returned
   * if they have a positive normalized score.
   * </p>
   *
   * @param query            the query to test
   * @param searcher         the searcher to test the query against
   * @param defaultFieldName used for displaing the query in assertion messages
   * @param results          a deque of documentIds that must match the query
   * @see #checkHitCollector
   */
public:
  static void checkHits(std::shared_ptr<Random> random,
                        std::shared_ptr<Query> query,
                        const std::wstring &defaultFieldName,
                        std::shared_ptr<IndexSearcher> searcher,
                        std::deque<int> &results) ;

  /**
   * Tests that a Hits has an expected order of documents
   */
  static void checkDocIds(const std::wstring &mes, std::deque<int> &results,
                          std::deque<std::shared_ptr<ScoreDoc>> &hits);

  /**
   * Tests that two queries have an expected order of documents,
   * and that the two queries have the same score values.
   */
  static void checkHitsQuery(std::shared_ptr<Query> query,
                             std::deque<std::shared_ptr<ScoreDoc>> &hits1,
                             std::deque<std::shared_ptr<ScoreDoc>> &hits2,
                             std::deque<int> &results);

  static void checkEqual(std::shared_ptr<Query> query,
                         std::deque<std::shared_ptr<ScoreDoc>> &hits1,
                         std::deque<std::shared_ptr<ScoreDoc>> &hits2);

  static std::wstring hits2str(std::deque<std::shared_ptr<ScoreDoc>> &hits1,
                               std::deque<std::shared_ptr<ScoreDoc>> &hits2,
                               int start, int end);

  static std::wstring topdocsString(std::shared_ptr<TopDocs> docs, int start,
                                    int end);

  /**
   * Asserts that the explanation value for every document matching a
   * query corresponds with the true score.
   *
   * @param query            the query to test
   * @param searcher         the searcher to test the query against
   * @param defaultFieldName used for displaing the query in assertion messages
   * @see ExplanationAsserter
   * @see #checkExplanations(Query, std::wstring, IndexSearcher, bool) for a
   * "deep" testing of the explanation details.
   */
  static void
  checkExplanations(std::shared_ptr<Query> query,
                    const std::wstring &defaultFieldName,
                    std::shared_ptr<IndexSearcher> searcher) ;

  /**
   * Asserts that the explanation value for every document matching a
   * query corresponds with the true score.  Optionally does "deep"
   * testing of the explanation details.
   *
   * @param query            the query to test
   * @param searcher         the searcher to test the query against
   * @param defaultFieldName used for displaing the query in assertion messages
   * @param deep             indicates whether a deep comparison of
   * sub-Explanation details should be executed
   * @see ExplanationAsserter
   */
  static void checkExplanations(std::shared_ptr<Query> query,
                                const std::wstring &defaultFieldName,
                                std::shared_ptr<IndexSearcher> searcher,
                                bool deep) ;

  /**
   * returns a reasonable epsilon for comparing two floats,
   * where minor differences are acceptable such as score vs. explain
   */
  static float explainToleranceDelta(float f1, float f2);

  /**
   * Asserts that the result of calling {@link Weight#matches(LeafReaderContext,
   * int)} for every document matching a query returns a non-null {@link
   * Matches}
   *
   * @param query    the query to test
   * @param searcher the search to test against
   */
  static void
  checkMatches(std::shared_ptr<Query> query,
               std::shared_ptr<IndexSearcher> searcher) ;

  /**
   * Assert that an explanation has the expected score, and optionally that its
   * sub-details max/sum/factor match to that score.
   *
   * @param q     std::wstring representation of the query for assertion messages
   * @param doc   Document ID for assertion messages
   * @param score Real score value of doc with query q
   * @param deep  indicates whether a deep comparison of sub-Explanation details
   * should be executed
   * @param expl  The Explanation to match against score
   */
  static void verifyExplanation(const std::wstring &q, int doc, float score,
                                bool deep, std::shared_ptr<Explanation> expl);

  /**
   * an IndexSearcher that implicitly checks hte explanation of every match
   * whenever it executes a search.
   *
   * @see ExplanationAsserter
   */
public:
  class ExplanationAssertingSearcher : public IndexSearcher
  {
    GET_CLASS_NAME(ExplanationAssertingSearcher)
  public:
    ExplanationAssertingSearcher(std::shared_ptr<IndexReader> r);

  protected:
    virtual void checkExplanations(std::shared_ptr<Query> q) ;

  public:
    std::shared_ptr<TopFieldDocs>
    search(std::shared_ptr<Query> query, int n,
           std::shared_ptr<Sort> sort)  override;

    void search(std::shared_ptr<Query> query,
                std::shared_ptr<Collector> results)  override;

    std::shared_ptr<TopDocs> search(std::shared_ptr<Query> query,
                                    int n)  override;

  protected:
    std::shared_ptr<ExplanationAssertingSearcher> shared_from_this()
    {
      return std::static_pointer_cast<ExplanationAssertingSearcher>(
          IndexSearcher::shared_from_this());
    }
  };

  /**
   * Asserts that the score explanation for every document matching a
   * query corresponds with the true score.
   * <p>
   * NOTE: this HitCollector should only be used with the Query and Searcher
   * specified at when it is constructed.
   *
   * @see CheckHits#verifyExplanation
   */
public:
  class ExplanationAsserter : public SimpleCollector
  {
    GET_CLASS_NAME(ExplanationAsserter)

  public:
    std::shared_ptr<Query> q;
    std::shared_ptr<IndexSearcher> s;
    std::wstring d;
    bool deep = false;

    std::shared_ptr<Scorer> scorer;

  private:
    int base = 0;

    /**
     * Constructs an instance which does shallow tests on the Explanation
     */
  public:
    ExplanationAsserter(std::shared_ptr<Query> q,
                        const std::wstring &defaultFieldName,
                        std::shared_ptr<IndexSearcher> s);

    ExplanationAsserter(std::shared_ptr<Query> q,
                        const std::wstring &defaultFieldName,
                        std::shared_ptr<IndexSearcher> s, bool deep);

    void setScorer(std::shared_ptr<Scorer> scorer)  override;

    void collect(int doc)  override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    bool needsScores() override;

  protected:
    std::shared_ptr<ExplanationAsserter> shared_from_this()
    {
      return std::static_pointer_cast<ExplanationAsserter>(
          SimpleCollector::shared_from_this());
    }
  };

  /**
   * Asserts that the {@link Matches} from a query is non-null whenever
   * the document its created for is a hit.
   * <p>
   * Also checks that the previous non-matching document has a {@code null}
   * {@link Matches}
   */
public:
  class MatchesAsserter : public SimpleCollector
  {
    GET_CLASS_NAME(MatchesAsserter)

  private:
    const std::shared_ptr<Weight> weight;
    std::shared_ptr<LeafReaderContext> context;

  public:
    int lastCheckedDoc = -1;

    MatchesAsserter(std::shared_ptr<Query> query,
                    std::shared_ptr<IndexSearcher> searcher) ;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    void collect(int doc)  override;

    bool needsScores() override;

  protected:
    std::shared_ptr<MatchesAsserter> shared_from_this()
    {
      return std::static_pointer_cast<MatchesAsserter>(
          SimpleCollector::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::search
