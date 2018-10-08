#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/FilteringTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::standard
{
class StandardTokenizer;
}

namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::analysis::sinks
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;

/**
 * tests for the TestTeeSinkTokenFilter
 */
class TestTeeSinkTokenFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestTeeSinkTokenFilter)
protected:
  std::shared_ptr<StringBuilder> buffer1;
  std::shared_ptr<StringBuilder> buffer2;
  std::deque<std::wstring> tokens1;
  std::deque<std::wstring> tokens2;

public:
  void setUp()  override;

  // LUCENE-1448
  // TODO: instead of testing it this way, we can test
  // with BaseTokenStreamTestCase now...
  virtual void
  testEndOffsetPositionWithTeeSinkTokenFilter() ;

  virtual void testGeneral() ;

  virtual void testMultipleSources() ;

private:
  std::shared_ptr<StandardTokenizer>
  standardTokenizer(std::shared_ptr<StringBuilder> builder);

  /**
   * Not an explicit test, just useful to print out some info on performance
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("resource") public void performance()
  // throws Exception
  virtual void performance() ;

public:
  class ModuloTokenFilter : public TokenFilter
  {
    GET_CLASS_NAME(ModuloTokenFilter)

  public:
    int modCount = 0;

    ModuloTokenFilter(std::shared_ptr<TokenStream> input, int mc);

    int count = 0;

    // return every 100 tokens
    bool incrementToken()  override;

  protected:
    std::shared_ptr<ModuloTokenFilter> shared_from_this()
    {
      return std::static_pointer_cast<ModuloTokenFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

public:
  class ModuloSinkFilter : public FilteringTokenFilter
  {
    GET_CLASS_NAME(ModuloSinkFilter)
  public:
    int count = 0;
    int modCount = 0;

    ModuloSinkFilter(std::shared_ptr<TokenStream> input, int mc);

  protected:
    bool accept()  override;

  protected:
    std::shared_ptr<ModuloSinkFilter> shared_from_this()
    {
      return std::static_pointer_cast<ModuloSinkFilter>(
          org.apache.lucene.analysis.FilteringTokenFilter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestTeeSinkTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestTeeSinkTokenFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::sinks
