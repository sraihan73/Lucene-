#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

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

using SearchEquivalenceTestBase =
    org::apache::lucene::search::SearchEquivalenceTestBase;

using namespace org::apache::lucene::search::spans;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.spans.SpanTestUtil.*;

/**
 * Basic equivalence tests for span queries
 */
class TestSpanSearchEquivalence : public SearchEquivalenceTestBase
{
  GET_CLASS_NAME(TestSpanSearchEquivalence)

  // TODO: we could go a little crazy for a lot of these,
  // but these are just simple minimal cases in case something
  // goes horribly wrong. Put more intense tests elsewhere.

  /** SpanTermQuery(A) = TermQuery(A) */
public:
  virtual void testSpanTermVersusTerm() ;

  /** SpanOrQuery(A) = SpanTermQuery(A) */
  virtual void testSpanOrVersusTerm() ;

  /** SpanOrQuery(A, A) = SpanTermQuery(A) */
  virtual void testSpanOrDoubleVersusTerm() ;

  /** SpanOrQuery(A, B) = (A B) */
  virtual void testSpanOrVersusBooleanTerm() ;

  /** SpanOrQuery(SpanNearQuery[A B], SpanNearQuery[C D]) = (SpanNearQuery[A B],
   * SpanNearQuery[C D]) */
  virtual void testSpanOrVersusBooleanNear() ;

  /** SpanNotQuery(A, B) ⊆ SpanTermQuery(A) */
  virtual void testSpanNotVersusSpanTerm() ;

  /** SpanNotQuery(A, [B C]) ⊆ SpanTermQuery(A) */
  virtual void testSpanNotNearVersusSpanTerm() ;

  /** SpanNotQuery([A B], C) ⊆ SpanNearQuery([A B]) */
  virtual void testSpanNotVersusSpanNear() ;

  /** SpanNotQuery([A B], [C D]) ⊆ SpanNearQuery([A B]) */
  virtual void testSpanNotNearVersusSpanNear() ;

  /** SpanFirstQuery(A, 10) ⊆ SpanTermQuery(A) */
  virtual void testSpanFirstVersusSpanTerm() ;

  /** SpanNearQuery([A, B], 0, true) = "A B" */
  virtual void testSpanNearVersusPhrase() ;

  /** SpanNearQuery([A, B], ∞, false) = +A +B */
  virtual void testSpanNearVersusBooleanAnd() ;

  /** SpanNearQuery([A B], 0, false) ⊆ SpanNearQuery([A B], 1, false) */
  virtual void testSpanNearVersusSloppySpanNear() ;

  /** SpanNearQuery([A B], 3, true) ⊆ SpanNearQuery([A B], 3, false) */
  virtual void testSpanNearInOrderVersusOutOfOrder() ;

  /** SpanNearQuery([A B], N, false) ⊆ SpanNearQuery([A B], N+1, false) */
  virtual void testSpanNearIncreasingSloppiness() ;

  /** SpanNearQuery([A B C], N, false) ⊆ SpanNearQuery([A B C], N+1, false) */
  virtual void testSpanNearIncreasingSloppiness3() ;

  /** SpanNearQuery([A B], N, true) ⊆ SpanNearQuery([A B], N+1, true) */
  virtual void
  testSpanNearIncreasingOrderedSloppiness() ;

  /** SpanNearQuery([A B C], N, true) ⊆ SpanNearQuery([A B C], N+1, true) */
  virtual void
  testSpanNearIncreasingOrderedSloppiness3() ;

  /** SpanPositionRangeQuery(A, M, N) ⊆ TermQuery(A) */
  virtual void testSpanRangeTerm() ;

  /** SpanPositionRangeQuery(A, M, N) ⊆ SpanFirstQuery(A, M, N+1) */
  virtual void testSpanRangeTermIncreasingEnd() ;

  /** SpanPositionRangeQuery(A, 0, ∞) = TermQuery(A) */
  virtual void testSpanRangeTermEverything() ;

  /** SpanPositionRangeQuery([A B], M, N) ⊆ SpanNearQuery([A B]) */
  virtual void testSpanRangeNear() ;

  /** SpanPositionRangeQuery([A B], M, N) ⊆ SpanFirstQuery([A B], M, N+1) */
  virtual void testSpanRangeNearIncreasingEnd() ;

  /** SpanPositionRangeQuery([A B], ∞) = SpanNearQuery([A B]) */
  virtual void testSpanRangeNearEverything() ;

  /** SpanFirstQuery(A, N) ⊆ TermQuery(A) */
  virtual void testSpanFirstTerm() ;

  /** SpanFirstQuery(A, N) ⊆ SpanFirstQuery(A, N+1) */
  virtual void testSpanFirstTermIncreasing() ;

  /** SpanFirstQuery(A, ∞) = TermQuery(A) */
  virtual void testSpanFirstTermEverything() ;

  /** SpanFirstQuery([A B], N) ⊆ SpanNearQuery([A B]) */
  virtual void testSpanFirstNear() ;

  /** SpanFirstQuery([A B], N) ⊆ SpanFirstQuery([A B], N+1) */
  virtual void testSpanFirstNearIncreasing() ;

  /** SpanFirstQuery([A B], ∞) = SpanNearQuery([A B]) */
  virtual void testSpanFirstNearEverything() ;

  /** SpanWithinQuery(A, B) ⊆ SpanNearQuery(A) */
  virtual void testSpanWithinVsNear() ;

  /** SpanWithinQuery(A, B) = SpanContainingQuery(A, B) */
  virtual void testSpanWithinVsContaining() ;

  virtual void testSpanBoostQuerySimplification() ;

protected:
  std::shared_ptr<TestSpanSearchEquivalence> shared_from_this()
  {
    return std::static_pointer_cast<TestSpanSearchEquivalence>(
        org.apache.lucene.search.SearchEquivalenceTestBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
