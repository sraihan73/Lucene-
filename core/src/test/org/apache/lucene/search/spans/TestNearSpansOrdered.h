#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanNearQuery.h"
#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"

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
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.spans.SpanTestUtil.assertFinished;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.spans.SpanTestUtil.assertNext;

class TestNearSpansOrdered : public LuceneTestCase
{
  GET_CLASS_NAME(TestNearSpansOrdered)
protected:
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<Directory> directory;
  std::shared_ptr<IndexReader> reader;

public:
  static const std::wstring FIELD;

  void tearDown()  override;

  void setUp()  override;

protected:
  std::deque<std::wstring> docFields = {
      L"w1 w2 w3 w4 w5",       L"w1 w3 w2 w3 zz", L"w1 xx w2 yy w3",
      L"w1 w3 xx w2 yy w3 zz", L"t1 t2 t2 t1",    L"g x x g g x x x g g x x g",
      L"go to webpage"};

  virtual std::shared_ptr<SpanNearQuery> makeQuery(const std::wstring &s1,
                                                   const std::wstring &s2,
                                                   const std::wstring &s3,
                                                   int slop, bool inOrder);
  virtual std::shared_ptr<SpanNearQuery> makeQuery();

  virtual std::shared_ptr<SpanNearQuery>
  makeOverlappedQuery(const std::wstring &sqt1, const std::wstring &sqt2,
                      bool sqOrdered, const std::wstring &t3, bool ordered);

public:
  virtual void testSpanNearQuery() ;

  virtual std::wstring s(std::shared_ptr<Spans> span);
  virtual std::wstring s(int doc, int start, int end);

  virtual void testNearSpansNext() ;

  /**
   * test does not imply that skipTo(doc+1) should work exactly the
   * same as next -- it's only applicable in this case since we know doc
   * does not contain more than one span
   */
  virtual void testNearSpansAdvanceLikeNext() ;

  virtual void testNearSpansNextThenAdvance() ;

  virtual void testNearSpansNextThenAdvancePast() ;

  virtual void testNearSpansAdvancePast() ;

  virtual void testNearSpansAdvanceTo0() ;

  virtual void testNearSpansAdvanceTo1() ;

  /**
   * not a direct test of NearSpans, but a demonstration of how/when
   * this causes problems
   */
  virtual void testSpanNearScorerSkipTo1() ;

  virtual void testOverlappedOrderedSpan() ;

  virtual void testOverlappedNonOrderedSpan() ;

  virtual void testNonOverlappedOrderedSpan() ;

  virtual void testOrderedSpanIteration() ;

  virtual void testOrderedSpanIterationSameTerms1() ;

  virtual void testOrderedSpanIterationSameTerms2() ;

  /**
   * not a direct test of NearSpans, but a demonstration of how/when
   * this causes problems
   */
  virtual void testSpanNearScorerExplain() ;

  virtual void testGaps() ;

  virtual void testMultipleGaps() ;

  virtual void testNestedGaps() ;

  /*
    protected std::wstring[] docFields = {
    "w1 w2 w3 w4 w5",
    "w1 w3 w2 w3 zz",
    "w1 xx w2 yy w3",
    "w1 w3 xx w2 yy w3 zz",
    "t1 t2 t2 t1",
    "g x x g g x x x g g x x g",
    "go to webpage"
  };
   */

protected:
  std::shared_ptr<TestNearSpansOrdered> shared_from_this()
  {
    return std::static_pointer_cast<TestNearSpansOrdered>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
