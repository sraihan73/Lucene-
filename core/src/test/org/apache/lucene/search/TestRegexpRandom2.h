#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::util::automaton
{
class Automaton;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::util
{
class AttributeSource;
}
namespace org::apache::lucene::util::automaton
{
class CharacterRunAutomaton;
}
namespace org::apache::lucene::util
{
class CharsRefBuilder;
}
namespace org::apache::lucene::util
{
class BytesRef;
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

using FilteredTermsEnum = org::apache::lucene::index::FilteredTermsEnum;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using Automaton = org::apache::lucene::util::automaton::Automaton;

/**
 * Create an index with random unicode terms
 * Generates random regexps, and validates against a simple impl.
 */
class TestRegexpRandom2 : public LuceneTestCase
{
  GET_CLASS_NAME(TestRegexpRandom2)
protected:
  std::shared_ptr<IndexSearcher> searcher1;
  std::shared_ptr<IndexSearcher> searcher2;

private:
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<Directory> dir;

protected:
  std::wstring fieldName;

public:
  void setUp()  override;

  void tearDown()  override;

  /** a stupid regexp query that just blasts thru the terms */
private:
  class DumbRegexpQuery : public MultiTermQuery
  {
    GET_CLASS_NAME(DumbRegexpQuery)
  private:
    const std::shared_ptr<Automaton> automaton;

  public:
    DumbRegexpQuery(std::shared_ptr<Term> term, int flags);

  protected:
    std::shared_ptr<TermsEnum> getTermsEnum(
        std::shared_ptr<Terms> terms,
        std::shared_ptr<AttributeSource> atts)  override;

  private:
    class SimpleAutomatonTermsEnum : public FilteredTermsEnum
    {
      GET_CLASS_NAME(SimpleAutomatonTermsEnum)
    private:
      std::shared_ptr<TestRegexpRandom2::DumbRegexpQuery> outerInstance;

    public:
      std::shared_ptr<CharacterRunAutomaton> runAutomaton =
          std::make_shared<CharacterRunAutomaton>(outerInstance->automaton);
      std::shared_ptr<CharsRefBuilder> utf16 =
          std::make_shared<CharsRefBuilder>();

    private:
      SimpleAutomatonTermsEnum(
          std::shared_ptr<TestRegexpRandom2::DumbRegexpQuery> outerInstance,
          std::shared_ptr<TermsEnum> tenum);

    protected:
      AcceptStatus
      accept(std::shared_ptr<BytesRef> term)  override;

    protected:
      std::shared_ptr<SimpleAutomatonTermsEnum> shared_from_this()
      {
        return std::static_pointer_cast<SimpleAutomatonTermsEnum>(
            org.apache.lucene.index.FilteredTermsEnum::shared_from_this());
      }
    };

  public:
    std::wstring toString(const std::wstring &field) override;

    bool equals(std::any obj) override;

  protected:
    std::shared_ptr<DumbRegexpQuery> shared_from_this()
    {
      return std::static_pointer_cast<DumbRegexpQuery>(
          MultiTermQuery::shared_from_this());
    }
  };

  /** test a bunch of random regular expressions */
public:
  virtual void testRegexps() ;

  /** check that the # of hits is the same as from a very
   * simple regexpquery implementation.
   */
protected:
  virtual void assertSame(const std::wstring &regexp) ;

protected:
  std::shared_ptr<TestRegexpRandom2> shared_from_this()
  {
    return std::static_pointer_cast<TestRegexpRandom2>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
