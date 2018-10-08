#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search::spans
{
class SpanQuery;
}
namespace org::apache::lucene::search::spans
{
class Spans;
}
namespace org::apache::lucene::index
{
class IndexWriter;
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
namespace org::apache::lucene::search::spans
{

using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.spans.SpanTestUtil.assertFinished;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.spans.SpanTestUtil.assertNext;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.spans.SpanTestUtil.spanNearOrderedQuery;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.spans.SpanTestUtil.spanNearUnorderedQuery;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.spans.SpanTestUtil.spanNotQuery;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.spans.SpanTestUtil.spanOrQuery;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.spans.SpanTestUtil.spanTermQuery;

class TestSpans : public LuceneTestCase
{
  GET_CLASS_NAME(TestSpans)
private:
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<Directory> directory;

public:
  static const std::wstring field;

  void setUp()  override;

  void tearDown()  override;

private:
  std::deque<std::wstring> docFields = {
      L"w1 w2 w3 w4 w5",
      L"w1 w3 w2 w3",
      L"w1 xx w2 yy w3",
      L"w1 w3 xx w2 yy w3",
      L"u2 u2 u1",
      L"u2 xx u2 u1",
      L"u2 u2 xx u1",
      L"u2 xx u2 yy u1",
      L"u2 xx u1 u2",
      L"u2 u1 xx u2",
      L"u1 u2 xx u2",
      L"t1 t2 t1 t3 t2 t3",
      L"s2 s1 s1 xx xx s2 xx s2 xx s1 xx xx xx xx xx s2 xx",
      L"r1 s11",
      L"r1 s21"};

  void checkHits(std::shared_ptr<Query> query,
                 std::deque<int> &results) ;

  void orderedSlopTest3SQ(std::shared_ptr<SpanQuery> q1,
                          std::shared_ptr<SpanQuery> q2,
                          std::shared_ptr<SpanQuery> q3, int slop,
                          std::deque<int> &expectedDocs) ;

public:
  virtual void
  orderedSlopTest3(int slop, std::deque<int> &expectedDocs) ;

  virtual void
  orderedSlopTest3Equal(int slop,
                        std::deque<int> &expectedDocs) ;

  virtual void
  orderedSlopTest1Equal(int slop,
                        std::deque<int> &expectedDocs) ;

  virtual void testSpanNearOrdered01() ;

  virtual void testSpanNearOrdered02() ;

  virtual void testSpanNearOrdered03() ;

  virtual void testSpanNearOrdered04() ;

  virtual void testSpanNearOrdered05() ;

  virtual void testSpanNearOrderedEqual01() ;

  virtual void testSpanNearOrderedEqual02() ;

  virtual void testSpanNearOrderedEqual03() ;

  virtual void testSpanNearOrderedEqual04() ;

  virtual void testSpanNearOrderedEqual11() ;

  virtual void testSpanNearOrderedEqual12() ;

  virtual void testSpanNearOrderedEqual13() ;

  virtual void testSpanNearOrderedEqual14() ;

  virtual void testSpanNearOrderedEqual15() ;

  virtual void testSpanNearOrderedOverlap() ;

  virtual void testSpanNearUnOrdered() ;

private:
  std::shared_ptr<Spans>
  orSpans(std::deque<std::wstring> &terms) ;

public:
  virtual void testSpanOrEmpty() ;

  virtual void testSpanOrSingle() ;

  virtual void testSpanOrDouble() ;

  virtual void testSpanOrDoubleAdvance() ;

  virtual void testSpanOrUnused() ;

  virtual void testSpanOrTripleSameDoc() ;

  virtual void testSpanScorerZeroSloppyFreq() ;

private:
  class ClassicSimilarityAnonymousInnerClass : public ClassicSimilarity
  {
    GET_CLASS_NAME(ClassicSimilarityAnonymousInnerClass)
  private:
    std::shared_ptr<TestSpans> outerInstance;

  public:
    ClassicSimilarityAnonymousInnerClass(
        std::shared_ptr<TestSpans> outerInstance);

    float sloppyFreq(int distance) override;

  protected:
    std::shared_ptr<ClassicSimilarityAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ClassicSimilarityAnonymousInnerClass>(
          org.apache.lucene.search.similarities
              .ClassicSimilarity::shared_from_this());
    }
  };

  // LUCENE-1404
private:
  void addDoc(std::shared_ptr<IndexWriter> writer, const std::wstring &id,
              const std::wstring &text) ;

  // LUCENE-1404
  int64_t hitCount(std::shared_ptr<IndexSearcher> searcher,
                     const std::wstring &word) ;

  // LUCENE-1404
  std::shared_ptr<SpanQuery> createSpan(const std::wstring &value);

  // LUCENE-1404
  std::shared_ptr<SpanQuery>
  createSpan(int slop, bool ordered,
             std::deque<std::shared_ptr<SpanQuery>> &clauses);

  // LUCENE-1404
  std::shared_ptr<SpanQuery> createSpan(int slop, bool ordered,
                                        const std::wstring &term1,
                                        const std::wstring &term2);

  // LUCENE-1404
public:
  virtual void testNPESpanQuery() ;

  virtual void testSpanNotWithMultiterm() ;

  virtual void testSpanNots() ;

private:
  int spanCount(const std::wstring &include, int slop,
                const std::wstring &exclude, int pre,
                int post) ;

protected:
  std::shared_ptr<TestSpans> shared_from_this()
  {
    return std::static_pointer_cast<TestSpans>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::spans
