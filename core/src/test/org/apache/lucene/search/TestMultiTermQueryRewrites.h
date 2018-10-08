#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/search/MultiTermQuery.h"
#include  "core/src/java/org/apache/lucene/search/RewriteMethod.h"
#include  "core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"
#include  "core/src/java/org/apache/lucene/search/BoostAttribute.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestMultiTermQueryRewrites : public LuceneTestCase
{
  GET_CLASS_NAME(TestMultiTermQueryRewrites)

public:
  static std::shared_ptr<Directory> dir, sdir1, sdir2;
  static std::shared_ptr<IndexReader> reader, multiReader, multiReaderDupls;
  static std::shared_ptr<IndexSearcher> searcher, multiSearcher,
      multiSearcherDupls;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

private:
  std::shared_ptr<Query> extractInnerQuery(std::shared_ptr<Query> q);

  std::shared_ptr<Term> extractTerm(std::shared_ptr<Query> q);

  void checkBooleanQueryOrder(std::shared_ptr<Query> q);

  void checkDuplicateTerms(std::shared_ptr<MultiTermQuery::RewriteMethod>
                               method) ;

public:
  virtual void testRewritesWithDuplicateTerms() ;

private:
  void checkBooleanQueryBoosts(std::shared_ptr<BooleanQuery> bq);

  void checkBoosts(std::shared_ptr<MultiTermQuery::RewriteMethod> method) throw(
      std::runtime_error);

private:
  class MultiTermQueryAnonymousInnerClass : public MultiTermQuery
  {
    GET_CLASS_NAME(MultiTermQueryAnonymousInnerClass)
  private:
    std::shared_ptr<TestMultiTermQueryRewrites> outerInstance;

  public:
    MultiTermQueryAnonymousInnerClass(
        std::shared_ptr<TestMultiTermQueryRewrites> outerInstance);

  protected:
    std::shared_ptr<TermsEnum> getTermsEnum(
        std::shared_ptr<Terms> terms,
        std::shared_ptr<AttributeSource> atts)  override;

  private:
    class FilteredTermsEnumAnonymousInnerClass : public FilteredTermsEnum
    {
      GET_CLASS_NAME(FilteredTermsEnumAnonymousInnerClass)
    private:
      std::shared_ptr<MultiTermQueryAnonymousInnerClass> outerInstance;

    public:
      FilteredTermsEnumAnonymousInnerClass(
          std::shared_ptr<MultiTermQueryAnonymousInnerClass> outerInstance,
          std::shared_ptr<TermsEnum> iterator);

      const std::shared_ptr<BoostAttribute> boostAtt;

    protected:
      AcceptStatus accept(std::shared_ptr<BytesRef> term) override;

    protected:
      std::shared_ptr<FilteredTermsEnumAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilteredTermsEnumAnonymousInnerClass>(
            org.apache.lucene.index.FilteredTermsEnum::shared_from_this());
      }
    };

  public:
    // C++ TODO: There is no native C++ equivalent to 'toString':
    std::wstring Term::toString(const std::wstring &field) override;

  protected:
    std::shared_ptr<MultiTermQueryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<MultiTermQueryAnonymousInnerClass>(
          MultiTermQuery::shared_from_this());
    }
  };

public:
  virtual void testBoosts() ;

private:
  void checkMaxClauseLimitation(std::shared_ptr<MultiTermQuery::RewriteMethod>
                                    method) ;

  void checkNoMaxClauseLimitation(std::shared_ptr<MultiTermQuery::RewriteMethod>
                                      method) ;

public:
  virtual void testMaxClauseLimitations() ;

protected:
  std::shared_ptr<TestMultiTermQueryRewrites> shared_from_this()
  {
    return std::static_pointer_cast<TestMultiTermQueryRewrites>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
