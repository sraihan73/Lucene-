#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::analysis::pattern
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

class TestPatternTokenizer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestPatternTokenizer)
public:
  virtual void testSplitting() ;

  virtual void testOffsetCorrection() ;

  /**
   * TODO: rewrite tests not to use string comparison.
   */
private:
  static std::wstring
  tsToString(std::shared_ptr<TokenStream> in_) ;

  /** blast some random strings through the analyzer */
public:
  virtual void testRandomStrings() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestPatternTokenizer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestPatternTokenizer> outerInstance);

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
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestPatternTokenizer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestPatternTokenizer> outerInstance);

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

  // LUCENE-6814
public:
  virtual void testHeapFreedAfterClose() ;

protected:
  std::shared_ptr<TestPatternTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<TestPatternTokenizer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::pattern
