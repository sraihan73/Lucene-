#pragma once
#include "stringbuilder.h"
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

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::search
{
class Query;
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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Simple base class for checking search equivalence.
 * Extend it, and write tests that create {@link #randomTerm()}s
GET_CLASS_NAME(for)
 * (all terms are single characters a-z), and use
 * {@link #assertSameSet(Query, Query)} and
 * {@link #assertSubsetOf(Query, Query)}
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs("SimpleText") public abstract class
// SearchEquivalenceTestBase extends org.apache.lucene.util.LuceneTestCase
class SearchEquivalenceTestBase : public LuceneTestCase
{
protected:
  static std::shared_ptr<IndexSearcher> s1, s2;
  static std::shared_ptr<Directory> directory;
  static std::shared_ptr<IndexReader> reader;
  static std::shared_ptr<Analyzer> analyzer;
  static std::wstring stopword; // we always pick a character as a stopword

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

  /**
   * populate a field with random contents.
   * terms should be single characters in lowercase (a-z)
   * tokenization can be assumed to be on whitespace.
   */
  static std::wstring randomFieldContents();

  /**
   * returns random character (a-z)
   */
  static wchar_t randomChar();

  /**
   * returns a term suitable for searching.
   * terms are single characters in lowercase (a-z)
   */
protected:
  virtual std::shared_ptr<Term> randomTerm();

  /**
   * Returns a random filter over the document set
   */
  virtual std::shared_ptr<Query> randomFilter();

  /**
   * Asserts that the documents returned by <code>q1</code>
   * are the same as of those returned by <code>q2</code>
   */
public:
  virtual void
  assertSameSet(std::shared_ptr<Query> q1,
                std::shared_ptr<Query> q2) ;

  /**
   * Asserts that the documents returned by <code>q1</code>
   * are a subset of those returned by <code>q2</code>
   */
  virtual void
  assertSubsetOf(std::shared_ptr<Query> q1,
                 std::shared_ptr<Query> q2) ;

  /**
   * Asserts that the documents returned by <code>q1</code>
   * are a subset of those returned by <code>q2</code>.
   *
   * Both queries will be filtered by <code>filter</code>
   */
protected:
  virtual void
  assertSubsetOf(std::shared_ptr<Query> q1, std::shared_ptr<Query> q2,
                 std::shared_ptr<Query> filter) ;

  /**
   * Assert that two queries return the same documents and with the same scores.
   */
  virtual void
  assertSameScores(std::shared_ptr<Query> q1,
                   std::shared_ptr<Query> q2) ;

  virtual void
  assertSameScores(std::shared_ptr<Query> q1, std::shared_ptr<Query> q2,
                   std::shared_ptr<Query> filter) ;

  virtual std::shared_ptr<Query> filteredQuery(std::shared_ptr<Query> query,
                                               std::shared_ptr<Query> filter);

protected:
  std::shared_ptr<SearchEquivalenceTestBase> shared_from_this()
  {
    return std::static_pointer_cast<SearchEquivalenceTestBase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
