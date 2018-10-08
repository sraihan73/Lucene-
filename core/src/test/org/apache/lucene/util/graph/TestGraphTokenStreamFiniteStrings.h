#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Token.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
namespace org::apache::lucene::util::graph
{

using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * {@link GraphTokenStreamFiniteStrings} tests.
 */
class TestGraphTokenStreamFiniteStrings : public LuceneTestCase
{
  GET_CLASS_NAME(TestGraphTokenStreamFiniteStrings)

private:
  static std::shared_ptr<Token> token(const std::wstring &term, int posInc,
                                      int posLength);

  void
  assertTokenStream(std::shared_ptr<TokenStream> ts,
                    std::deque<std::wstring> &terms,
                    std::deque<int> &increments) ;

public:
  virtual void testIllegalState() ;

  virtual void testEmpty() ;

  virtual void testSingleGraph() ;

  virtual void testSingleGraphWithGap() ;

  virtual void testGraphAndGapSameToken() ;

  virtual void testGraphAndGapSameTokenTerm() ;

  virtual void testStackedGraph() ;

  virtual void testStackedGraphWithGap() ;

  virtual void testStackedGraphWithRepeat() ;

  virtual void testGraphWithRegularSynonym() ;

  virtual void testMultiGraph() ;

  virtual void testMultipleSidePaths() ;

protected:
  std::shared_ptr<TestGraphTokenStreamFiniteStrings> shared_from_this()
  {
    return std::static_pointer_cast<TestGraphTokenStreamFiniteStrings>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/graph/
