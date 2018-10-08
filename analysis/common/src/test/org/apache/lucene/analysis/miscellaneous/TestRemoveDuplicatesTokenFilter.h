#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Token;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::synonym
{
class SynonymMap;
}
namespace org::apache::lucene::analysis::synonym
{
class Builder;
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
namespace org::apache::lucene::analysis::miscellaneous
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Token = org::apache::lucene::analysis::Token;
using SynonymMap = org::apache::lucene::analysis::synonym::SynonymMap;

class TestRemoveDuplicatesTokenFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestRemoveDuplicatesTokenFilter)

public:
  static std::shared_ptr<Token> tok(int pos, const std::wstring &t, int start,
                                    int end);
  static std::shared_ptr<Token> tok(int pos, const std::wstring &t);

  virtual void testDups(const std::wstring &expected,
                        std::deque<Token> &tokens) ;

private:
  class TokenStreamAnonymousInnerClass : public TokenStream
  {
    GET_CLASS_NAME(TokenStreamAnonymousInnerClass)
  private:
    std::shared_ptr<TestRemoveDuplicatesTokenFilter> outerInstance;

    std::shared_ptr<Iterator<std::shared_ptr<Token>>> toks;

  public:
    TokenStreamAnonymousInnerClass(
        std::shared_ptr<TestRemoveDuplicatesTokenFilter> outerInstance,
        std::shared_ptr<Iterator<std::shared_ptr<Token>>> toks);

    std::shared_ptr<CharTermAttribute> termAtt;
    std::shared_ptr<OffsetAttribute> offsetAtt;
    std::shared_ptr<PositionIncrementAttribute> posIncAtt;
    bool incrementToken() override;

  protected:
    std::shared_ptr<TokenStreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TokenStreamAnonymousInnerClass>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

public:
  virtual void testNoDups() ;

  virtual void testSimpleDups() ;

  virtual void testComplexDups() ;

  // some helper methods for the below test with synonyms
private:
  std::wstring randomNonEmptyString();

  void add(std::shared_ptr<SynonymMap::Builder> b, const std::wstring &input,
           const std::wstring &output, bool keepOrig);

  /** blast some random strings through the analyzer */
public:
  virtual void testRandomStrings() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestRemoveDuplicatesTokenFilter> outerInstance;

    std::shared_ptr<SynonymMap> map_obj;
    bool ignoreCase = false;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestRemoveDuplicatesTokenFilter> outerInstance,
        std::shared_ptr<SynonymMap> map_obj, bool ignoreCase);

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
    std::shared_ptr<TestRemoveDuplicatesTokenFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestRemoveDuplicatesTokenFilter> outerInstance);

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
  std::shared_ptr<TestRemoveDuplicatesTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestRemoveDuplicatesTokenFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
