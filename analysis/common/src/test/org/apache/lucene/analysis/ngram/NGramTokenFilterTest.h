#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class TokenStream;
}

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
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * Tests {@link NGramTokenFilter} for correctness.
 */
class NGramTokenFilterTest : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(NGramTokenFilterTest)
private:
  std::shared_ptr<TokenStream> input;

public:
  void setUp()  override;

  virtual void testInvalidInput() ;

  virtual void testInvalidInput2() ;

  virtual void testUnigrams() ;

  virtual void testBigrams() ;

  virtual void testNgrams() ;

  virtual void testNgramsNoIncrement() ;

  virtual void testOversizedNgrams() ;

  virtual void testOversizedNgramsPreserveOriginal() ;

  virtual void testSmallTokenInStream() ;

  virtual void
  testSmallTokenInStreamPreserveOriginal() ;

  virtual void testReset() ;

  virtual void testKeepShortTermKeepLongTerm() ;

  // LUCENE-3642
  // EdgeNgram blindly adds term length to offset, but this can take things out
  // of bounds wrt original text if a previous filter increases the length of
  // the word (in this case æ -> ae) so in this case we behave like WDF, and
  // preserve any modified offsets
  virtual void testInvalidOffsets() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<NGramTokenFilterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<NGramTokenFilterTest> outerInstance);

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
  virtual void testEndPositionIncrement() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<NGramTokenFilterTest> outerInstance;

    int min = 0;
    int max = 0;
    bool preserveOriginal = false;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<NGramTokenFilterTest> outerInstance, int min, int max,
        bool preserveOriginal);

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

public:
  virtual void testEmptyTerm() ;

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<NGramTokenFilterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<NGramTokenFilterTest> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass3>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testSupplementaryCharacters() ;

protected:
  std::shared_ptr<NGramTokenFilterTest> shared_from_this()
  {
    return std::static_pointer_cast<NGramTokenFilterTest>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ngram
