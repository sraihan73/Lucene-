#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

#include  "core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"

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
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

using namespace org::apache::lucene::analysis::miscellaneous;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.analysis.miscellaneous.CapitalizationFilter.*;

/** Tests {@link CapitalizationFilter} */
class TestCapitalizationFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestCapitalizationFilter)
public:
  virtual void testCapitalization() ;

  static void assertCapitalizesTo(
      std::shared_ptr<Tokenizer> tokenizer, std::deque<std::wstring> &expected,
      bool onlyFirstWord, std::shared_ptr<CharArraySet> keep,
      bool forceFirstLetter,
      std::shared_ptr<std::deque<std::deque<wchar_t>>> okPrefix,
      int minWordLength, int maxWordCount,
      int maxTokenLength) ;

  static void assertCapitalizesTo(
      const std::wstring &input, std::deque<std::wstring> &expected,
      bool onlyFirstWord, std::shared_ptr<CharArraySet> keep,
      bool forceFirstLetter,
      std::shared_ptr<std::deque<std::deque<wchar_t>>> okPrefix,
      int minWordLength, int maxWordCount,
      int maxTokenLength) ;

  static void assertCapitalizesToKeyword(
      const std::wstring &input, const std::wstring &expected,
      bool onlyFirstWord, std::shared_ptr<CharArraySet> keep,
      bool forceFirstLetter,
      std::shared_ptr<std::deque<std::deque<wchar_t>>> okPrefix,
      int minWordLength, int maxWordCount,
      int maxTokenLength) ;

  /** blast some random strings through the analyzer */
  virtual void testRandomString() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestCapitalizationFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestCapitalizationFilter> outerInstance);

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
  virtual void testEmptyTerm() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestCapitalizationFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestCapitalizationFilter> outerInstance);

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

  /**
   * checking the validity of constructor arguments
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test(expected = IllegalArgumentException.class) public void
  // testIllegalArguments() throws Exception
  virtual void testIllegalArguments() ;
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test(expected = IllegalArgumentException.class) public void
  // testIllegalArguments1() throws Exception
  virtual void testIllegalArguments1() ;
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test(expected = IllegalArgumentException.class) public void
  // testIllegalArguments2() throws Exception
  virtual void testIllegalArguments2() ;

protected:
  std::shared_ptr<TestCapitalizationFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestCapitalizationFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
