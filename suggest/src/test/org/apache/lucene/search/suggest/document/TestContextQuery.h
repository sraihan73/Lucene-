#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
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
namespace org::apache::lucene::search::suggest::document
{

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.TestSuggestField.Entry;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.TestSuggestField.assertSuggestions;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.TestSuggestField.iwcWithSuggestField;

class TestContextQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestContextQuery)
public:
  std::shared_ptr<Directory> dir;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void before() throws Exception
  virtual void before() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @After public void after() throws Exception
  virtual void after() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testIllegalInnerQuery() throws Exception
  virtual void testIllegalInnerQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSimpleContextQuery() throws Exception
  virtual void testSimpleContextQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testContextQueryOnSuggestField() throws
  // Exception
  virtual void testContextQueryOnSuggestField() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testNonExactContextQuery() throws
  // Exception
  virtual void testNonExactContextQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testContextPrecedenceBoost() throws
  // Exception
  virtual void testContextPrecedenceBoost() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEmptyContext() throws Exception
  virtual void testEmptyContext() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEmptyContextWithBoosts() throws
  // Exception
  virtual void testEmptyContextWithBoosts() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSameSuggestionMultipleContext() throws
  // Exception
  virtual void testSameSuggestionMultipleContext() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMixedContextQuery() throws Exception
  virtual void testMixedContextQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFilteringContextQuery() throws
  // Exception
  virtual void testFilteringContextQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testContextQueryRewrite() throws Exception
  virtual void testContextQueryRewrite() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMultiContextQuery() throws Exception
  virtual void testMultiContextQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAllContextQuery() throws Exception
  virtual void testAllContextQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRandomContextQueryScoring() throws
  // Exception
  virtual void testRandomContextQueryScoring() ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<Entry>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<TestContextQuery> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<TestContextQuery> outerInstance);

    int compare(std::shared_ptr<Entry> o1, std::shared_ptr<Entry> o2);
  };

protected:
  std::shared_ptr<TestContextQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestContextQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::document
