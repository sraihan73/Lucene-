#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

#include  "core/src/java/org/apache/lucene/analysis/charfilter/NormalizeCharMap.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
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
namespace org::apache::lucene::analysis::core
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs("Direct") public class TestBugInSomething
// extends org.apache.lucene.analysis.BaseTokenStreamTestCase
class TestBugInSomething : public BaseTokenStreamTestCase
{
public:
  virtual void test() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestBugInSomething> outerInstance;

    std::shared_ptr<CharArraySet> cas;
    std::shared_ptr<NormalizeCharMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestBugInSomething> outerInstance,
        std::shared_ptr<CharArraySet> cas,
        std::shared_ptr<NormalizeCharMap> map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

    std::shared_ptr<Reader> initReader(const std::wstring &fieldName,
                                       std::shared_ptr<Reader> reader) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  std::shared_ptr<CharFilter> wrappedStream =
      std::make_shared<CharFilterAnonymousInnerClass>(
          std::make_shared<StringReader>(L"bogus"));

private:
  class CharFilterAnonymousInnerClass : public CharFilter
  {
    GET_CLASS_NAME(CharFilterAnonymousInnerClass)
  public:
    CharFilterAnonymousInnerClass(std::shared_ptr<StringReader> java);

    void mark(int readAheadLimit) override;

    bool markSupported() override;

    int read() override;

    int read(std::deque<wchar_t> &cbuf) override;

    int read(std::shared_ptr<CharBuffer> target) override;

    bool ready() override;

    void reset() override;

    int64_t skip(int64_t n) override;

    int correct(int currentOff) override;

    virtual ~CharFilterAnonymousInnerClass();

    int read(std::deque<wchar_t> &arg0, int arg1, int arg2) override;

  protected:
    std::shared_ptr<CharFilterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CharFilterAnonymousInnerClass>(
          org.apache.lucene.analysis.CharFilter::shared_from_this());
    }
  };

public:
  virtual void testWrapping() ;

  // todo: test framework?

public:
  class SopTokenFilter final : public TokenFilter
  {
    GET_CLASS_NAME(SopTokenFilter)

  public:
    SopTokenFilter(std::shared_ptr<TokenStream> input);

    bool incrementToken()  override;

    void end()  override;

    virtual ~SopTokenFilter();

    void reset()  override;

  protected:
    std::shared_ptr<SopTokenFilter> shared_from_this()
    {
      return std::static_pointer_cast<SopTokenFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

  // LUCENE-5269
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testUnicodeShinglesAndNgrams() throws
  // Exception
  virtual void testUnicodeShinglesAndNgrams() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestBugInSomething> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestBugInSomething> outerInstance);

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

public:
  virtual void testCuriousWikipediaString() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestBugInSomething> outerInstance;

    std::shared_ptr<CharArraySet> protWords;
    std::deque<char> table;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestBugInSomething> outerInstance,
        std::shared_ptr<CharArraySet> protWords, std::deque<char> &table);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestBugInSomething> shared_from_this()
  {
    return std::static_pointer_cast<TestBugInSomething>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/core/
