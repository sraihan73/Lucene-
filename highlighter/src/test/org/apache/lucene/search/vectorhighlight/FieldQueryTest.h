#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::search::vectorhighlight
{

using Query = org::apache::lucene::search::Query;

class FieldQueryTest : public AbstractTestCase
{
  GET_CLASS_NAME(FieldQueryTest)
private:
  float boost = 0;

  /**
   * Set boost to a random value each time it is called.
   */
  void initBoost();

public:
  virtual void testFlattenBoolean() ;

  virtual void testFlattenDisjunctionMaxQuery() ;

  virtual void testFlattenTermAndPhrase() ;

  virtual void testFlattenTermAndPhrase2gram() ;

  virtual void testFlatten1TermPhrase() ;

  virtual void testExpand() ;

  virtual void testNoExpand() ;

  virtual void testExpandNotFieldMatch() ;

  virtual void testGetFieldTermMap() ;

  virtual void testGetRootMap() ;

  virtual void testGetRootMapNotFieldMatch() ;

  virtual void testGetTermSet() ;

  virtual void testQueryPhraseMap1Term() ;

  virtual void testQueryPhraseMap1Phrase() ;

  virtual void testQueryPhraseMap1PhraseAnother() ;

  virtual void testQueryPhraseMap2Phrases() ;

  virtual void testQueryPhraseMap2PhrasesFields() ;

  /*
   * <t>...terminal
   *
   * a-b-c-<t>
   *     +-d-<t>
   * b-c-d-<t>
   * +-d-<t>
   */
  virtual void testQueryPhraseMapOverlapPhrases() ;

  /*
   * <t>...terminal
   *
   * a-b-<t>
   *   +-c-<t>
   */
  virtual void testQueryPhraseMapOverlapPhrases2() ;

  /*
   * <t>...terminal
   *
   * a-a-a-<t>
   *     +-a-<t>
   *       +-a-<t>
   *         +-a-<t>
   */
  virtual void testQueryPhraseMapOverlapPhrases3() ;

  virtual void testQueryPhraseMapOverlap2gram() ;

  virtual void testSearchPhrase() ;

  virtual void testSearchPhraseSlop() ;

  virtual void testHighlightQuery() ;

  virtual void testPrefixQuery() ;

  virtual void testRegexpQuery() ;

  virtual void testRangeQuery() ;

private:
  void defgMultiTermQueryTest(std::shared_ptr<Query> query) ;

public:
  virtual void testStopRewrite() ;

private:
  class QueryAnonymousInnerClass : public Query
  {
    GET_CLASS_NAME(QueryAnonymousInnerClass)
  private:
    std::shared_ptr<FieldQueryTest> outerInstance;

  public:
    QueryAnonymousInnerClass(std::shared_ptr<FieldQueryTest> outerInstance);

    // C++ TODO: There is no native C++ equivalent to 'toString':
    std::wstring Term::toString(const std::wstring &field) override;
    virtual bool equals(std::any o);

    virtual int hashCode();

  protected:
    std::shared_ptr<QueryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<QueryAnonymousInnerClass>(
          org.apache.lucene.search.Query::shared_from_this());
    }
  };

public:
  virtual void testFlattenConstantScoreQuery() ;

  virtual void testFlattenToParentBlockJoinQuery() ;

protected:
  std::shared_ptr<FieldQueryTest> shared_from_this()
  {
    return std::static_pointer_cast<FieldQueryTest>(
        AbstractTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
