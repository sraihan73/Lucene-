#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
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
namespace org::apache::lucene::analysis::pattern
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/**
 * Tests {@link PatternReplaceCharFilter}
 */
class TestPatternReplaceCharFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestPatternReplaceCharFilter)
public:
  virtual void testFailingDot() ;

  virtual void testLongerReplacement() ;

  virtual void testShorterReplacement() ;

private:
  void checkOutput(
      const std::wstring &input, const std::wstring &pattern,
      const std::wstring &replacement, const std::wstring &expectedOutput,
      const std::wstring &expectedIndexMatchedOutput) ;

  //           1111
  // 01234567890123
  // this is test.
public:
  virtual void testNothingChange() ;

  // 012345678
  // aa bb cc
  virtual void testReplaceByEmpty() ;

  // 012345678
  // aa bb cc
  // aa#bb#cc
  virtual void test1block1matchSameLength() ;

  //           11111
  // 012345678901234
  // aa bb cc dd
  // aa##bb###cc dd
  virtual void test1block1matchLonger() ;

  // 01234567
  //  a  a
  //  aa  aa
  virtual void test1block2matchLonger() ;

  //           11111
  // 012345678901234
  // aa  bb   cc dd
  // aa#bb dd
  virtual void test1block1matchShorter() ;

  //           111111111122222222223333
  // 0123456789012345678901234567890123
  //   aa bb cc --- aa bb aa   bb   cc
  //   aa  bb  cc --- aa bb aa  bb  cc
  virtual void test1blockMultiMatches() ;

  //           11111111112222222222333333333
  // 012345678901234567890123456789012345678
  //   aa bb cc --- aa bb aa. bb aa   bb cc
  //   aa##bb cc --- aa##bb aa. bb aa##bb cc

  //   aa bb cc --- aa bbbaa. bb aa   b cc

  virtual void test2blocksMultiMatches() ;

  //           11111111112222222222333333333
  // 012345678901234567890123456789012345678
  //  a bb - ccc . --- bb a . ccc ccc bb
  //  aa b - c . --- b aa . c c b
  virtual void testChain() ;

private:
  std::shared_ptr<Pattern> pattern(const std::wstring &p);

  /**
   * A demonstration of how backtracking regular expressions can lead to
   * relatively easy DoS attacks.
   *
   * @see <a
   * href="http://swtch.com/~rsc/regexp/regexp1.html">"http://swtch.com/~rsc/regexp/regexp1.html"</a>
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Ignore public void testNastyPattern() throws Exception
  virtual void testNastyPattern() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestPatternReplaceCharFilter> outerInstance;

    std::shared_ptr<Pattern> p;
    std::wstring replacement;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestPatternReplaceCharFilter> outerInstance,
        std::shared_ptr<Pattern> p, const std::wstring &replacement);

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

protected:
  std::shared_ptr<TestPatternReplaceCharFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestPatternReplaceCharFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::pattern
