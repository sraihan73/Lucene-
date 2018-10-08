#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"
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
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

using namespace org::apache::lucene::search::spans;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.spans.SpanTestUtil.*;

class TestSpanContainQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestSpanContainQuery)
public:
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<Directory> directory;

  static const std::wstring field;

  void setUp()  override;

  void tearDown()  override;

  std::deque<std::wstring> docFields = {L"w1 w2 w3 w4 w5", L"w1 w3 w2 w3",
                                         L"w1 xx w2 yy w3",
                                         L"w1 w3 xx w2 yy w3"};

  virtual void checkHits(std::shared_ptr<Query> query,
                         std::deque<int> &results) ;

  virtual std::shared_ptr<Spans>
  makeSpans(std::shared_ptr<SpanQuery> sq) ;

  virtual void
  tstEqualSpans(const std::wstring &mes, std::shared_ptr<SpanQuery> expectedQ,
                std::shared_ptr<SpanQuery> actualQ) ;

  virtual void
  tstEqualSpans(const std::wstring &mes, std::shared_ptr<Spans> expected,
                std::shared_ptr<Spans> actual) ;

  virtual void testSpanContainTerm() ;

  virtual void testSpanContainPhraseBothWords() ;

  virtual void testSpanContainPhraseFirstWord() ;

  virtual void testSpanContainPhraseSecondWord() ;

protected:
  std::shared_ptr<TestSpanContainQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestSpanContainQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
