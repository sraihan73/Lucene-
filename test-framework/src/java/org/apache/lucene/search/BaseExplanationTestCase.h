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
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"

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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.spans.SpanTestUtil.*;

/**
 * Tests primitive queries (ie: that rewrite to themselves) to
 * insure they match the expected set of docs, and that the score of each
 * match is equal to the value of the scores explanation.
 *
 * <p>
 * The assumption is that if all of the "primitive" queries work well,
 * then anything that rewrites to a primitive will work well also.
 * </p>
 *
 */
class BaseExplanationTestCase : public LuceneTestCase
{
  GET_CLASS_NAME(BaseExplanationTestCase)
protected:
  static std::shared_ptr<IndexSearcher> searcher;
  static std::shared_ptr<IndexReader> reader;
  static std::shared_ptr<Directory> directory;
  static std::shared_ptr<Analyzer> analyzer;

public:
  static const std::wstring KEY;
  // boost on this field is the same as the iterator for the doc
  static const std::wstring FIELD;
  // same contents, but no field boost
  static const std::wstring ALTFIELD;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClassTestExplanations()
  // throws Exception
  static void afterClassTestExplanations() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void
  // beforeClassTestExplanations() throws Exception
  static void beforeClassTestExplanations() ;

  static std::shared_ptr<Document> createDoc(int index);

protected:
  static std::deque<std::wstring> const docFields;

  /**
   * check the expDocNrs match and have scores that match the explanations.
   * Query may be randomly wrapped in a BooleanQuery with a term that matches no
   * documents.
   */
public:
  virtual void qtest(std::shared_ptr<Query> q,
                     std::deque<int> &expDocNrs) ;

  /**
   * Tests a query using qtest after wrapping it with both optB and reqB
   * @see #qtest
   * @see #reqB
   * @see #optB
   */
  virtual void bqtest(std::shared_ptr<Query> q,
                      std::deque<int> &expDocNrs) ;

  /**
   * Convenience subclass of TermsQuery
   */
protected:
  virtual std::shared_ptr<Query> matchTheseItems(std::deque<int> &terms);

  /** helper for generating MultiPhraseQueries */
public:
  static std::deque<std::shared_ptr<Term>> ta(std::deque<std::wstring> &s);

  /** MACRO for SpanTermQuery */
  virtual std::shared_ptr<SpanQuery> st(const std::wstring &s);

  /** MACRO for SpanNotQuery */
  virtual std::shared_ptr<SpanQuery> snot(std::shared_ptr<SpanQuery> i,
                                          std::shared_ptr<SpanQuery> e);

  /** MACRO for SpanOrQuery containing two SpanTerm queries */
  virtual std::shared_ptr<SpanQuery> sor(const std::wstring &s,
                                         const std::wstring &e);

  /** MACRO for SpanOrQuery containing two SpanQueries */
  virtual std::shared_ptr<SpanQuery> sor(std::shared_ptr<SpanQuery> s,
                                         std::shared_ptr<SpanQuery> e);

  /** MACRO for SpanOrQuery containing three SpanTerm queries */
  virtual std::shared_ptr<SpanQuery>
  sor(const std::wstring &s, const std::wstring &m, const std::wstring &e);
  /** MACRO for SpanOrQuery containing two SpanQueries */
  virtual std::shared_ptr<SpanQuery> sor(std::shared_ptr<SpanQuery> s,
                                         std::shared_ptr<SpanQuery> m,
                                         std::shared_ptr<SpanQuery> e);

  /** MACRO for SpanNearQuery containing two SpanTerm queries */
  virtual std::shared_ptr<SpanQuery>
  snear(const std::wstring &s, const std::wstring &e, int slop, bool inOrder);

  /** MACRO for SpanNearQuery containing two SpanQueries */
  virtual std::shared_ptr<SpanQuery> snear(std::shared_ptr<SpanQuery> s,
                                           std::shared_ptr<SpanQuery> e,
                                           int slop, bool inOrder);

  /** MACRO for SpanNearQuery containing three SpanTerm queries */
  virtual std::shared_ptr<SpanQuery> snear(const std::wstring &s,
                                           const std::wstring &m,
                                           const std::wstring &e, int slop,
                                           bool inOrder);
  /** MACRO for SpanNearQuery containing three SpanQueries */
  virtual std::shared_ptr<SpanQuery> snear(std::shared_ptr<SpanQuery> s,
                                           std::shared_ptr<SpanQuery> m,
                                           std::shared_ptr<SpanQuery> e,
                                           int slop, bool inOrder);

  /** MACRO for SpanFirst(SpanTermQuery) */
  virtual std::shared_ptr<SpanQuery> sf(const std::wstring &s, int b);

  /**
   * MACRO: Wraps a Query in a BooleanQuery so that it is optional, along
   * with a second prohibited clause which will never match anything
   */
  virtual std::shared_ptr<Query>
  optB(std::shared_ptr<Query> q) ;

  /**
   * MACRO: Wraps a Query in a BooleanQuery so that it is required, along
   * with a second optional clause which will match everything
   */
  virtual std::shared_ptr<Query>
  reqB(std::shared_ptr<Query> q) ;

protected:
  std::shared_ptr<BaseExplanationTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseExplanationTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
