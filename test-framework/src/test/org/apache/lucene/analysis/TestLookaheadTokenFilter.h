#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Position;
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
namespace org::apache::lucene::analysis
{

class TestLookaheadTokenFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestLookaheadTokenFilter)

public:
  virtual void testRandomStrings() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestLookaheadTokenFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestLookaheadTokenFilter> outerInstance);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          Analyzer::shared_from_this());
    }
  };

private:
  class NeverPeeksLookaheadTokenFilter
      : public LookaheadTokenFilter<
            std::shared_ptr<LookaheadTokenFilter::Position>>
  {
    GET_CLASS_NAME(NeverPeeksLookaheadTokenFilter)
  public:
    NeverPeeksLookaheadTokenFilter(std::shared_ptr<TokenStream> input);

    std::shared_ptr<Position> newPosition() override;

    bool incrementToken()  override;

  protected:
    std::shared_ptr<NeverPeeksLookaheadTokenFilter> shared_from_this()
    {
      return std::static_pointer_cast<NeverPeeksLookaheadTokenFilter>(
          LookaheadTokenFilter<
              LookaheadTokenFilter.Position>::shared_from_this());
    }
  };

public:
  virtual void testNeverCallingPeek() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestLookaheadTokenFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestLookaheadTokenFilter> outerInstance);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void testMissedFirstToken() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestLookaheadTokenFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestLookaheadTokenFilter> outerInstance);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          Analyzer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestLookaheadTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestLookaheadTokenFilter>(
        BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
