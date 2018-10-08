#pragma once
#include "stringhelper.h"
#include <iostream>
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class RandomIndexWriter;
}

namespace org::apache::lucene::analysis
{
class Token;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::index
{
class Term;
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

using Token = org::apache::lucene::analysis::Token;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * This class tests the MultiPhraseQuery class.
 *
 *
 */
class TestMultiPhraseQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestMultiPhraseQuery)

public:
  virtual void testPhrasePrefix() ;

  // LUCENE-2580
  virtual void testTall() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Ignore public void testMultiSloppyWithRepeats() throws
  // java.io.IOException
  virtual void testMultiSloppyWithRepeats() ;

  virtual void testMultiExactWithRepeats() ;

private:
  void add(const std::wstring &s,
           std::shared_ptr<RandomIndexWriter> writer) ;

public:
  virtual void
  testBooleanQueryContainingSingleTermPrefixQuery() ;

  virtual void testPhrasePrefixWithBooleanQuery() ;

  virtual void testNoDocs() ;

  virtual void testHashCodeAndEquals();

private:
  void add(const std::wstring &s, const std::wstring &type,
           std::shared_ptr<RandomIndexWriter> writer) ;

  // LUCENE-2526
public:
  virtual void testEmptyToString();

  virtual void testZeroPosIncr() ;

private:
  static std::shared_ptr<Token> makeToken(const std::wstring &text,
                                          int posIncr);

  static std::deque<std::shared_ptr<Token>> const INCR_0_DOC_TOKENS;

  static std::deque<std::shared_ptr<Token>> const INCR_0_QUERY_TOKENS_AND;

  static std::deque<std::deque<std::shared_ptr<Token>>> const
      INCR_0_QUERY_TOKENS_AND_OR_MATCH;

  static std::deque<std::deque<std::shared_ptr<Token>>> const
      INCR_0_QUERY_TOKENS_AND_OR_NO_MATCHN;

  /**
   * using query parser, MPQ will be created, and will not be strict about
   * having all query terms in each position - one of each position is
   * sufficient (OR logic)
   */
public:
  virtual void testZeroPosIncrSloppyParsedAnd() ;

private:
  void doTestZeroPosIncrSloppy(std::shared_ptr<Query> q,
                               int nExpected) ;

  /**
   * PQ AND Mode - Manually creating a phrase query
   */
public:
  virtual void testZeroPosIncrSloppyPqAnd() ;

  /**
   * MPQ AND Mode - Manually creating a multiple phrase query
   */
  virtual void testZeroPosIncrSloppyMpqAnd() ;

  /**
   * MPQ Combined AND OR Mode - Manually creating a multiple phrase query
   */
  virtual void testZeroPosIncrSloppyMpqAndOrMatch() ;

  /**
   * MPQ Combined AND OR Mode - Manually creating a multiple phrase query - with
   * no match
   */
  virtual void testZeroPosIncrSloppyMpqAndOrNoMatch() ;

private:
  std::deque<std::shared_ptr<Term>>
  tapTerms(std::deque<std::shared_ptr<Token>> &tap);

public:
  virtual void testNegativeSlop() ;

protected:
  std::shared_ptr<TestMultiPhraseQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestMultiPhraseQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
