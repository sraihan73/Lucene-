#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"

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

using namespace org::apache::lucene::analysis;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

using namespace org::apache::lucene::search::spans;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.spans.SpanTestUtil.*;

/**
 * Tests basic search capabilities.
 *
 * <p>Uses a collection of 1000 documents, each the english rendition of their
 * document number.  For example, the document numbered 333 has text "three
 * hundred thirty three".
 *
 * <p>Tests are each a single query, and its hits are checked to ensure that
 * all and only the correct documents are returned, thus providing end-to-end
 * testing of the indexing and search code.
 *
 */
class TestBasics : public LuceneTestCase
{
  GET_CLASS_NAME(TestBasics)
private:
  static std::shared_ptr<IndexSearcher> searcher;
  static std::shared_ptr<IndexReader> reader;
  static std::shared_ptr<Directory> directory;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

  virtual void testTerm() ;

  virtual void testTerm2() ;

  virtual void testPhrase() ;

  virtual void testPhrase2() ;

  virtual void testBoolean() ;

  virtual void testBoolean2() ;

  virtual void testSpanNearExact() ;

  virtual void testSpanTermQuery() ;

  virtual void testSpanNearUnordered() ;

  virtual void testSpanNearOrdered() ;

  virtual void testSpanNot() ;

  virtual void testSpanNotNoOverflowOnLargeSpans() ;

  virtual void testSpanWithMultipleNotSingle() ;

  virtual void testSpanWithMultipleNotMany() ;

  virtual void testNpeInSpanNearWithSpanNot() ;

  virtual void
  testNpeInSpanNearInSpanFirstInSpanNot() ;

  virtual void testSpanNotWindowOne() ;

  virtual void testSpanNotWindowTwoBefore() ;

  virtual void testSpanNotWindowNegPost() ;

  virtual void testSpanNotWindowNegPre() ;

  virtual void
  testSpanNotWindowDoubleExcludesBefore() ;

  virtual void testSpanFirst() ;

  virtual void testSpanPositionRange() ;

  virtual void testSpanOr() ;

  virtual void testSpanExactNested() ;

  virtual void testSpanNearOr() ;

  virtual void testSpanComplex1() ;

private:
  void checkHits(std::shared_ptr<Query> query,
                 std::deque<int> &results) ;

protected:
  std::shared_ptr<TestBasics> shared_from_this()
  {
    return std::static_pointer_cast<TestBasics>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
