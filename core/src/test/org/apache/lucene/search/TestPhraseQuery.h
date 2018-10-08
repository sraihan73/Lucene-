#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/PhraseQuery.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"

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
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests {@link PhraseQuery}.
 *
 * @see TestPositionIncrement
 */
class TestPhraseQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestPhraseQuery)

  /** threshold for comparing floats */
public:
  static constexpr float SCORE_COMP_THRESH = 1e-6.0f;

private:
  static std::shared_ptr<IndexSearcher> searcher;
  static std::shared_ptr<IndexReader> reader;
  std::shared_ptr<PhraseQuery> query;
  static std::shared_ptr<Directory> directory;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  public:
    AnalyzerAnonymousInnerClass();

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

    int getPositionIncrementGap(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  void setUp()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

  virtual void testNotCloseEnough() ;

  virtual void testBarelyCloseEnough() ;

  /**
   * Ensures slop of 0 works for exact matches, but not reversed
   */
  virtual void testExact() ;

  virtual void testSlop1() ;

  /**
   * As long as slop is at least 2, terms can be reversed
   */
  virtual void testOrderDoesntMatter() ;

  /**
   * slop is the total number of positional moves allowed
   * to line up a phrase
   */
  virtual void testMultipleTerms() ;

  virtual void testPhraseQueryWithStopAnalyzer() ;

  virtual void testPhraseQueryInConjunctionScorer() ;

  virtual void testSlopScoring() ;

  virtual void testToString() ;

  virtual void testWrappedPhrase() ;

  // work on two docs like this: "phrase exist notexist exist found"
  virtual void testNonExistingPhrase() ;

  /**
   * Working on a 2 fields like this:
   *    Field("field", "one two three four five")
   *    Field("palindrome", "one two three two one")
   * Phrase of size 2 occuriong twice, once in order and once in reverse,
   * because doc is a palyndrome, is counted twice.
   * Also, in this case order in query does not matter.
   * Also, when an exact match is found, both sloppy scorer and exact scorer
   * scores the same.
   */
  virtual void testPalyndrome2() ;

  /**
   * Working on a 2 fields like this:
   *    Field("field", "one two three four five")
   *    Field("palindrome", "one two three two one")
   * Phrase of size 3 occuriong twice, once in order and once in reverse,
   * because doc is a palyndrome, is counted twice.
   * Also, in this case order in query does not matter.
   * Also, when an exact match is found, both sloppy scorer and exact scorer
   * scores the same.
   */
  virtual void testPalyndrome3() ;

  // LUCENE-1280
  virtual void testEmptyPhraseQuery() ;

  /* test that a single term is rewritten to a term query */
  virtual void testRewrite() ;

  /** Tests PhraseQuery with terms at the same position in the query. */
  virtual void testZeroPosIncr() ;

  virtual void testRandomPhrases() ;

  virtual void testNegativeSlop() ;

  virtual void testNegativePosition() ;

  virtual void testBackwardPositions() ;

protected:
  std::shared_ptr<TestPhraseQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestPhraseQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
