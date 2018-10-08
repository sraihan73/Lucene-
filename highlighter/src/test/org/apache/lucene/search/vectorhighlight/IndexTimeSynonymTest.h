#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class AttributeFactory;
}

namespace org::apache::lucene::util
{
class AttributeImpl;
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
namespace org::apache::lucene::search::vectorhighlight
{

using namespace org::apache::lucene::analysis;

class IndexTimeSynonymTest : public AbstractTestCase
{
  GET_CLASS_NAME(IndexTimeSynonymTest)

public:
  virtual void testFieldTermStackIndex1wSearch1term() ;

  virtual void
  testFieldTermStackIndex1wSearch2terms() ;

  virtual void
  testFieldTermStackIndex1w2wSearch1term() ;

  virtual void
  testFieldTermStackIndex1w2wSearch1phrase() ;

  virtual void
  testFieldTermStackIndex1w2wSearch1partial() ;

  virtual void
  testFieldTermStackIndex1w2wSearch1term1phrase() ;

  virtual void
  testFieldTermStackIndex2w1wSearch1term() ;

  virtual void
  testFieldTermStackIndex2w1wSearch1phrase() ;

  virtual void
  testFieldTermStackIndex2w1wSearch1partial() ;

  virtual void
  testFieldTermStackIndex2w1wSearch1term1phrase() ;

  virtual void
  testFieldPhraseListIndex1w2wSearch1phrase() ;

  virtual void
  testFieldPhraseListIndex1w2wSearch1partial() ;

  virtual void
  testFieldPhraseListIndex1w2wSearch1term1phrase() ;

  virtual void
  testFieldPhraseListIndex2w1wSearch1term() ;

  virtual void
  testFieldPhraseListIndex2w1wSearch1phrase() ;

  virtual void
  testFieldPhraseListIndex2w1wSearch1partial() ;

  virtual void
  testFieldPhraseListIndex2w1wSearch1term1phrase() ;

private:
  void makeIndex1w() ;

  void makeIndex1w2w() ;

  void makeIndex2w1w() ;

public:
  virtual void
  makeSynonymIndex(const std::wstring &value,
                   std::deque<Token> &tokens) ;

  static std::shared_ptr<Token> t(const std::wstring &text, int startOffset,
                                  int endOffset);

  static std::shared_ptr<Token> t(const std::wstring &text, int startOffset,
                                  int endOffset, int positionIncrement);

public:
  class TokenArrayAnalyzer final : public Analyzer
  {
    GET_CLASS_NAME(TokenArrayAnalyzer)
  public:
    std::deque<std::shared_ptr<Token>> const tokens;
    TokenArrayAnalyzer(std::deque<Token> &tokens);

    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  private:
    class TokenizerAnonymousInnerClass : public Tokenizer
    {
      GET_CLASS_NAME(TokenizerAnonymousInnerClass)
    private:
      std::shared_ptr<TokenArrayAnalyzer> outerInstance;

    public:
      TokenizerAnonymousInnerClass(
          std::shared_ptr<TokenArrayAnalyzer> outerInstance,
          std::shared_ptr<org::apache::lucene::util::AttributeFactory>
              TOKEN_ATTRIBUTE_FACTORY);

      const std::shared_ptr<AttributeImpl> reusableToken;
      int p = 0;

      bool incrementToken() override;

      void reset()  override;

    protected:
      std::shared_ptr<TokenizerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<TokenizerAnonymousInnerClass>(
            Tokenizer::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<TokenArrayAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<TokenArrayAnalyzer>(
          Analyzer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<IndexTimeSynonymTest> shared_from_this()
  {
    return std::static_pointer_cast<IndexTimeSynonymTest>(
        AbstractTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::vectorhighlight
