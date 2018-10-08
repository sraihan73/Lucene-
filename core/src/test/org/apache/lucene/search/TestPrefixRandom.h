#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"

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

using FilteredTermsEnum = org::apache::lucene::index::FilteredTermsEnum;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Create an index with random unicode terms
 * Generates random prefix queries, and validates against a simple impl.
 */
class TestPrefixRandom : public LuceneTestCase
{
  GET_CLASS_NAME(TestPrefixRandom)
private:
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<Directory> dir;

public:
  void setUp()  override;

  void tearDown()  override;

  /** a stupid prefix query that just blasts thru the terms */
private:
  class DumbPrefixQuery : public MultiTermQuery
  {
    GET_CLASS_NAME(DumbPrefixQuery)
  private:
    const std::shared_ptr<BytesRef> prefix;

  public:
    DumbPrefixQuery(std::shared_ptr<Term> term);

  protected:
    std::shared_ptr<TermsEnum> getTermsEnum(
        std::shared_ptr<Terms> terms,
        std::shared_ptr<AttributeSource> atts)  override;

  private:
    class SimplePrefixTermsEnum : public FilteredTermsEnum
    {
      GET_CLASS_NAME(SimplePrefixTermsEnum)
    private:
      const std::shared_ptr<BytesRef> prefix;

      SimplePrefixTermsEnum(std::shared_ptr<TermsEnum> tenum,
                            std::shared_ptr<BytesRef> prefix);

    protected:
      AcceptStatus
      accept(std::shared_ptr<BytesRef> term)  override;

    protected:
      std::shared_ptr<SimplePrefixTermsEnum> shared_from_this()
      {
        return std::static_pointer_cast<SimplePrefixTermsEnum>(
            org.apache.lucene.index.FilteredTermsEnum::shared_from_this());
      }
    };

  public:
    std::wstring toString(const std::wstring &field) override;

    bool equals(std::any obj) override;

  protected:
    std::shared_ptr<DumbPrefixQuery> shared_from_this()
    {
      return std::static_pointer_cast<DumbPrefixQuery>(
          MultiTermQuery::shared_from_this());
    }
  };

  /** test a bunch of random prefixes */
public:
  virtual void testPrefixes() ;

  /** check that the # of hits is the same as from a very
   * simple prefixquery implementation.
   */
private:
  void assertSame(const std::wstring &prefix) ;

protected:
  std::shared_ptr<TestPrefixRandom> shared_from_this()
  {
    return std::static_pointer_cast<TestPrefixRandom>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
