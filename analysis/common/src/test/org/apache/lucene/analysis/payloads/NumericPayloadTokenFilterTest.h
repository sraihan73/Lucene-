#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class TypeAttribute;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::analysis::payloads
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;

class NumericPayloadTokenFilterTest : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(NumericPayloadTokenFilterTest)

public:
  virtual void test() ;

private:
  class WordTokenFilter final : public TokenFilter
  {
    GET_CLASS_NAME(WordTokenFilter)
  private:
    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<TypeAttribute> typeAtt =
        addAttribute(TypeAttribute::typeid);

    WordTokenFilter(std::shared_ptr<TokenStream> input);

  public:
    bool incrementToken()  override;

  protected:
    std::shared_ptr<WordTokenFilter> shared_from_this()
    {
      return std::static_pointer_cast<WordTokenFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<NumericPayloadTokenFilterTest> shared_from_this()
  {
    return std::static_pointer_cast<NumericPayloadTokenFilterTest>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};
} // namespace org::apache::lucene::analysis::payloads
