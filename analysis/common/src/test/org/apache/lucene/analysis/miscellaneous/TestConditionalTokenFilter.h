#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/FilteringTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "../../../../../../java/org/apache/lucene/analysis/de/GermanStemFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/in/IndicNormalizationFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ConditionalTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/KeywordRepeatFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ProtectedTermFilter.h"
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis
{
class CannedTokenStream;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::analysis
{
class CharArraySet;
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

namespace org::apache::lucene::analysis::miscellaneous
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

class TestConditionalTokenFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestConditionalTokenFilter)

public:
  bool closed = false;
  bool ended = false;
  bool reset = false;

private:
  class AssertingLowerCaseFilter final : public TokenFilter
  {
    GET_CLASS_NAME(AssertingLowerCaseFilter)
  private:
    std::shared_ptr<TestConditionalTokenFilter> outerInstance;

    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);

  public:
    AssertingLowerCaseFilter(
        std::shared_ptr<TestConditionalTokenFilter> outerInstance,
        std::shared_ptr<TokenStream> in_);

    bool incrementToken()  override final;

    void end()  override;

    virtual ~AssertingLowerCaseFilter();

    void reset()  override;

  protected:
    std::shared_ptr<AssertingLowerCaseFilter> shared_from_this()
    {
      return std::static_pointer_cast<AssertingLowerCaseFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

private:
  class SkipMatchingFilter : public ConditionalTokenFilter
  {
    GET_CLASS_NAME(SkipMatchingFilter)
  private:
    std::shared_ptr<TestConditionalTokenFilter> outerInstance;

    const std::shared_ptr<Pattern> pattern;
    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);

  public:
    SkipMatchingFilter(
        std::shared_ptr<TestConditionalTokenFilter> outerInstance,
        std::shared_ptr<TokenStream> input,
        std::function<TokenStream *(TokenStream *)> &inputFactory,
        const std::wstring &termRegex);

  protected:
    bool shouldFilter()  override;

  protected:
    std::shared_ptr<SkipMatchingFilter> shared_from_this()
    {
      return std::static_pointer_cast<SkipMatchingFilter>(
          ConditionalTokenFilter::shared_from_this());
    }
  };

public:
  virtual void testSimple() ;

private:
  class TokenSplitter final : public TokenFilter
  {
    GET_CLASS_NAME(TokenSplitter)
  private:
    std::shared_ptr<TestConditionalTokenFilter> outerInstance;

  public:
    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    std::shared_ptr<State> state = nullptr;
    std::wstring half;

  protected:
    TokenSplitter(std::shared_ptr<TestConditionalTokenFilter> outerInstance,
                  std::shared_ptr<TokenStream> input);

  public:
    bool incrementToken()  override;

  protected:
    std::shared_ptr<TokenSplitter> shared_from_this()
    {
      return std::static_pointer_cast<TokenSplitter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

public:
  virtual void testMultitokenWrapping() ;

private:
  class EndTrimmingFilter final : public FilteringTokenFilter
  {
    GET_CLASS_NAME(EndTrimmingFilter)
  private:
    std::shared_ptr<TestConditionalTokenFilter> outerInstance;

  public:
    const std::shared_ptr<OffsetAttribute> offsetAtt =
        addAttribute(OffsetAttribute::typeid);

    EndTrimmingFilter(std::shared_ptr<TestConditionalTokenFilter> outerInstance,
                      std::shared_ptr<TokenStream> in_);

  protected:
    bool accept()  override;

  public:
    void end()  override;

  protected:
    std::shared_ptr<EndTrimmingFilter> shared_from_this()
    {
      return std::static_pointer_cast<EndTrimmingFilter>(
          org.apache.lucene.analysis.FilteringTokenFilter::shared_from_this());
    }
  };

public:
  virtual void testEndPropagation() ;

private:
  class ConditionalTokenFilterAnonymousInnerClass
      : public ConditionalTokenFilter
  {
    GET_CLASS_NAME(ConditionalTokenFilterAnonymousInnerClass)
  private:
    std::shared_ptr<TestConditionalTokenFilter> outerInstance;

  public:
    ConditionalTokenFilterAnonymousInnerClass(
        std::shared_ptr<TestConditionalTokenFilter> outerInstance,
        std::shared_ptr<CannedTokenStream> cts1,
        std::shared_ptr<UnknownType> EndTrimmingFilter);

  protected:
    bool shouldFilter()  override;

  protected:
    std::shared_ptr<ConditionalTokenFilterAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          ConditionalTokenFilterAnonymousInnerClass>(
          ConditionalTokenFilter::shared_from_this());
    }
  };

private:
  class ConditionalTokenFilterAnonymousInnerClass2
      : public ConditionalTokenFilter
  {
    GET_CLASS_NAME(ConditionalTokenFilterAnonymousInnerClass2)
  private:
    std::shared_ptr<TestConditionalTokenFilter> outerInstance;

  public:
    ConditionalTokenFilterAnonymousInnerClass2(
        std::shared_ptr<TestConditionalTokenFilter> outerInstance,
        std::shared_ptr<CannedTokenStream> cts2,
        std::shared_ptr<UnknownType> EndTrimmingFilter);

  protected:
    bool shouldFilter()  override;

  protected:
    std::shared_ptr<ConditionalTokenFilterAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          ConditionalTokenFilterAnonymousInnerClass2>(
          ConditionalTokenFilter::shared_from_this());
    }
  };

public:
  virtual void testWrapGraphs() ;

  virtual void testReadaheadWithNoFiltering() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestConditionalTokenFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestConditionalTokenFilter> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };
  return std::make_shared<Analyzer::TokenStreamComponents>(source, sink);

protected:
  std::shared_ptr<TestConditionalTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestConditionalTokenFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};
} // namespace org::apache::lucene::analysis::miscellaneous

// C++ TODO: The following line could not be converted:
public
void testReadaheadWithFiltering() throws java.io.IOException
{

  std::shared_ptr<CharArraySet> protectedTerms =
      std::make_shared<CharArraySet>(2, true);
  protectedTerms->add(L"three");

  std::shared_ptr<Analyzer> analyzer =
      std::make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(),
                                                    protectedTerms);

  std::wstring input = L"one two three four";

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream("", input))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(L"", input);
    BaseTokenStreamTestCase::assertTokenStreamContents(
        ts,
        std::deque<std::wstring>{L"one", L"one two", L"two", L"three",
                                  L"four"},
        std::deque<int>{0, 0, 4, 8, 14}, std::deque<int>{3, 7, 7, 13, 18},
        std::deque<int>{1, 0, 1, 1, 1}, std::deque<int>{1, 2, 1, 1, 1}, 18);
  }
}

public:
void testFilteringWithReadahead() throws IOException
{

  std::shared_ptr<CharArraySet> protectedTerms =
      std::make_shared<CharArraySet>(2, true);
  protectedTerms->add(L"two");
  protectedTerms->add(L"two three");

  std::shared_ptr<Analyzer> analyzer =
      std::make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this(),
                                                     protectedTerms);

  std::wstring input = L"one two three four";

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream("", input))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(L"", input);
    BaseTokenStreamTestCase::assertTokenStreamContents(
        ts, std::deque<std::wstring>{L"two", L"two three"},
        std::deque<int>{4, 4}, std::deque<int>{7, 13}, std::deque<int>{2, 0},
        std::deque<int>{1, 2}, 18);
  }
}

void testMultipleConditionalFilters() throws IOException
{
  std::shared_ptr<TokenStream> stream =
      BaseTokenStreamTestCase::whitespaceMockTokenizer(
          L"Alice Bob Clara David");
  std::shared_ptr<TokenStream> t = std::make_shared<SkipMatchingFilter>(
      stream,
      [&](std::any in_) {
        std::shared_ptr<TruncateTokenFilter> truncateFilter =
            std::make_shared<TruncateTokenFilter>(in_, 2);
        return std::make_shared<AssertingLowerCaseFilter>(truncateFilter);
      },
      L".*o.*");

  BaseTokenStreamTestCase::assertTokenStreamContents(
      t, std::deque<std::wstring>{L"al", L"Bob", L"cl", L"da"});
  assertTrue(closed);
  assertTrue(reset);
  assertTrue(ended);
}

void testFilteredTokenFilters() throws IOException
{

  std::shared_ptr<CharArraySet> protectedTerms =
      std::make_shared<CharArraySet>(2, true);
  protectedTerms->add(L"foobar");

  TokenStream ts =
      BaseTokenStreamTestCase::whitespaceMockTokenizer(L"wuthering foobar abc");
  ts = ProtectedTermFilter(protectedTerms, ts, [&](std::any in_) {
    std::make_shared<LengthFilter>(in_, 1, 4);
  });
  BaseTokenStreamTestCase::assertTokenStreamContents(
      ts, std::deque<std::wstring>{L"foobar", L"abc"});

  ts = BaseTokenStreamTestCase::whitespaceMockTokenizer(L"foobar abc");
  ts = ProtectedTermFilter(protectedTerms, ts, [&](std::any in_) {
    std::make_shared<LengthFilter>(in_, 1, 4);
  });
  BaseTokenStreamTestCase::assertTokenStreamContents(
      ts, std::deque<std::wstring>{L"foobar", L"abc"});
}

void testConsistentOffsets() throws IOException
{

  int64_t seed = random().nextLong();
  std::shared_ptr<Analyzer> analyzer =
      std::make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this(), seed);

  BaseTokenStreamTestCase::checkRandomData(random(), analyzer, 1);
}

void testEndWithShingles() throws IOException
{
  TokenStream ts = BaseTokenStreamTestCase::whitespaceMockTokenizer(
      L"cyk jvboq \u092e\u0962\u093f");
  ts = GermanStemFilter(ts);
  ts = NonRandomSkippingFilter(
      ts,
      [&](std::any in_) {
        std::make_shared<
            org::apache::lucene::analysis::shingle::FixedShingleFilter>(in_, 2);
      },
      true, false, true);
  ts = NonRandomSkippingFilter(ts, IndicNormalizationFilter::new, true);

  BaseTokenStreamTestCase::assertTokenStreamContents(
      ts, std::deque<std::wstring>{L"jvboq"});
}

void testInternalPositionAdjustment() throws IOException
{
  // check that the partial TokenStream sent to the condition filter begins with
  // a posInc of 1, even if the input stream has a posInc of 0 at that position,
  // and that the filtered stream has the correct posInc afterwards
  TokenStream ts =
      BaseTokenStreamTestCase::whitespaceMockTokenizer(L"one two three");
  ts = KeywordRepeatFilter(ts);
  ts = NonRandomSkippingFilter(ts, PositionAssertingTokenFilter::new, false,
                               true, true, true, true, false);

  BaseTokenStreamTestCase::assertTokenStreamContents(
      ts,
      std::deque<std::wstring>{L"one", L"one", L"two", L"two", L"three",
                                L"three"},
      std::deque<int>{1, 0, 1, 0, 1, 0});
}

private:
static final class PositionAssertingTokenFilter extends TokenFilter
{
  GET_CLASS_NAME(PositionAssertingTokenFilter)

  bool reset = false;
  std::shared_ptr<PositionIncrementAttribute> *const posIncAtt =
      addAttribute(PositionIncrementAttribute::typeid);

protected:
  PositionAssertingTokenFilter(TokenStream input) { __super(input); }

public:
  void reset() throws IOException
  {
    __super::reset();
    this->reset = true;
  }

  bool incrementToken() throws IOException
  {
    bool more = input::incrementToken();
    if (more && reset) {
      assertEquals(1, posIncAtt->getPositionIncrement());
    }
    reset = false;
    return more;
  }
}

private : static class RandomSkippingFilter extends ConditionalTokenFilter
{
  GET_CLASS_NAME(RandomSkippingFilter)

  std::shared_ptr<Random> random;
  constexpr int64_t seed;

protected:
  RandomSkippingFilter(TokenStream input, int64_t seed,
                       std::function<TokenStream *(TokenStream *)> inputFactory)
  {
    __super(input, inputFactory);
    this->seed = seed;
    this->random = std::make_shared<Random>(seed);
  }

  bool shouldFilter() throws IOException { return random->nextBoolean(); }

public:
  void reset() throws IOException
  {
    __super::reset();
    random = std::make_shared<Random>(seed);
  }
}

private : static class NonRandomSkippingFilter extends ConditionalTokenFilter
{
  GET_CLASS_NAME(NonRandomSkippingFilter)

  const std::deque<bool> shouldFilters;
  int pos;

  /**
   * Create a new BypassingTokenFilter
   *
   * @param input        the input TokenStream
   * @param inputFactory a factory function to create a new instance of the
   * TokenFilter to wrap
   */
protected:
  NonRandomSkippingFilter(
      TokenStream input,
      std::function<TokenStream *(TokenStream *)> inputFactory,
      bool... shouldFilters)
  {
    __super(input, inputFactory);
    this->shouldFilters = shouldFilters;
  }

  bool shouldFilter() throws IOException
  {
    return shouldFilters[pos++ % shouldFilters.size()];
  }

public:
  void reset() throws IOException
  {
    __super::reset();
    pos = 0;
  }
}
}
}
