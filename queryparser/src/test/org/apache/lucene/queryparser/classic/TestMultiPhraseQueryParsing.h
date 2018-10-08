#pragma once
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
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
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
namespace org::apache::lucene::queryparser::classic
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestMultiPhraseQueryParsing : public LuceneTestCase
{
  GET_CLASS_NAME(TestMultiPhraseQueryParsing)

private:
  class TokenAndPos : public std::enable_shared_from_this<TokenAndPos>
  {
    GET_CLASS_NAME(TokenAndPos)
  public:
    const std::wstring token;
    const int pos;
    TokenAndPos(const std::wstring &token, int pos);
  };

private:
  class CannedAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(CannedAnalyzer)
  private:
    std::deque<std::shared_ptr<TokenAndPos>> const tokens;

  public:
    CannedAnalyzer(std::deque<std::shared_ptr<TokenAndPos>> &tokens);

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<CannedAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<CannedAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class CannedTokenizer : public Tokenizer
  {
    GET_CLASS_NAME(CannedTokenizer)
  private:
    std::deque<std::shared_ptr<TokenAndPos>> const tokens;
    int upto = 0;
    int lastPos = 0;
    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
        addAttribute(PositionIncrementAttribute::typeid);

  public:
    CannedTokenizer(std::deque<std::shared_ptr<TokenAndPos>> &tokens);

    bool incrementToken() override final;

    void reset()  override;

  protected:
    std::shared_ptr<CannedTokenizer> shared_from_this()
    {
      return std::static_pointer_cast<CannedTokenizer>(
          org.apache.lucene.analysis.Tokenizer::shared_from_this());
    }
  };

public:
  virtual void testMultiPhraseQueryParsing() ;

protected:
  std::shared_ptr<TestMultiPhraseQueryParsing> shared_from_this()
  {
    return std::static_pointer_cast<TestMultiPhraseQueryParsing>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::classic
