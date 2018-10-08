#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
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

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * TestWildcard tests the '*' and '?' wildcard characters.
 */
class TestWildcard : public LuceneTestCase
{
  GET_CLASS_NAME(TestWildcard)

public:
  virtual void testEquals();

  /**
   * Tests if a WildcardQuery that has no wildcard in the term is rewritten to a
   * single TermQuery. The boost should be preserved, and the rewrite should
   * return a ConstantScoreQuery if the WildcardQuery had a ConstantScore
   * rewriteMethod.
   */
  virtual void testTermWithoutWildcard() ;

  /**
   * Tests if a WildcardQuery with an empty term is rewritten to an empty
   * BooleanQuery
   */
  virtual void testEmptyTerm() ;

  /**
   * Tests if a WildcardQuery that has only a trailing * in the term is
   * rewritten to a single PrefixQuery. The boost and rewriteMethod should be
   * preserved.
   */
  virtual void testPrefixTerm() ;

  /**
   * Tests Wildcard queries with an asterisk.
   */
  virtual void testAsterisk() ;

  /**
   * Tests Wildcard queries with a question mark.
   *
   * @throws IOException if an error occurs
   */
  virtual void testQuestionmark() ;

  /**
   * Tests if wildcard escaping works
   */
  virtual void testEscapes() ;

private:
  std::shared_ptr<Directory>
  getIndexStore(const std::wstring &field,
                std::deque<std::wstring> &contents) ;

  void assertMatches(std::shared_ptr<IndexSearcher> searcher,
                     std::shared_ptr<Query> q,
                     int expectedMatches) ;

  /**
   * Test that wild card queries are parsed to the correct type and are searched
   * correctly. This test looks at both parsing and execution of wildcard
   * queries. Although placed here, it also tests prefix queries, verifying that
   * prefix queries are not parsed into wild card queries, and vice-versa.
   */
public:
  virtual void testParsingAndSearching() ;

protected:
  std::shared_ptr<TestWildcard> shared_from_this()
  {
    return std::static_pointer_cast<TestWildcard>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
