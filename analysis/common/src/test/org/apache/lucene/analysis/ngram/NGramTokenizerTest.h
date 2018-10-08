#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "../../../../../../java/org/apache/lucene/analysis/ngram/NGramTokenizer.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
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
namespace org::apache::lucene::analysis::ngram
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/**
 * Tests {@link NGramTokenizer} for correctness.
 */
class NGramTokenizerTest : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(NGramTokenizerTest)
private:
  std::shared_ptr<StringReader> input;

public:
  void setUp()  override;

  virtual void testInvalidInput() ;

  virtual void testInvalidInput2() ;

  virtual void testUnigrams() ;

  virtual void testBigrams() ;

  virtual void testNgrams() ;

  virtual void testOversizedNgrams() ;

  virtual void testReset() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<NGramTokenizerTest> outerInstance;

    int min = 0;
    int max = 0;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<NGramTokenizerTest> outerInstance, int min, int max);

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

private:
  static void testNGrams(int minGram, int maxGram, int length,
                         const std::wstring &nonTokenChars) ;

  static void testNGrams(int minGram, int maxGram, const std::wstring &s,
                         const std::wstring &nonTokenChars) ;

public:
  static std::deque<int> toCodePoints(std::shared_ptr<std::wstring> s);

  static bool isTokenChar(const std::wstring &nonTokenChars, int codePoint);

  static void testNGrams(int minGram, int maxGram, const std::wstring &s,
                         const std::wstring &nonTokenChars,
                         bool edgesOnly) ;

private:
  class NGramTokenizerAnonymousInnerClass : public NGramTokenizer
  {
    GET_CLASS_NAME(NGramTokenizerAnonymousInnerClass)
  private:
    std::wstring nonTokenChars;

  public:
    NGramTokenizerAnonymousInnerClass(int minGram, int maxGram, bool edgesOnly,
                                      const std::wstring &nonTokenChars);

  protected:
    bool isTokenChar(int chr) override;

  protected:
    std::shared_ptr<NGramTokenizerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<NGramTokenizerAnonymousInnerClass>(
          NGramTokenizer::shared_from_this());
    }
  };

public:
  virtual void testLargeInput() ;

  virtual void testLargeMaxGram() ;

  virtual void testPreTokenization() ;

  virtual void testHeavyPreTokenization() ;

  virtual void testFewTokenChars() ;

  virtual void testFullUTF8Range() ;

protected:
  std::shared_ptr<NGramTokenizerTest> shared_from_this()
  {
    return std::static_pointer_cast<NGramTokenizerTest>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ngram
