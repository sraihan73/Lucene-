#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class KeywordAttribute;
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
namespace org::apache::lucene::analysis::miscellaneous
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

/**
 * Testcase for {@link KeywordMarkerFilter}
 */
class TestKeywordMarkerFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestKeywordMarkerFilter)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSetFilterIncrementToken() throws
  // java.io.IOException
  virtual void testSetFilterIncrementToken() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPatternFilterIncrementToken() throws
  // java.io.IOException
  virtual void testPatternFilterIncrementToken() ;

  // LUCENE-2901
  virtual void testComposition() ;

public:
  class LowerCaseFilterMock final : public TokenFilter
  {
    GET_CLASS_NAME(LowerCaseFilterMock)

  private:
    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<KeywordAttribute> keywordAttr =
        addAttribute(KeywordAttribute::typeid);

  public:
    LowerCaseFilterMock(std::shared_ptr<TokenStream> in_);

    bool incrementToken()  override;

  protected:
    std::shared_ptr<LowerCaseFilterMock> shared_from_this()
    {
      return std::static_pointer_cast<LowerCaseFilterMock>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestKeywordMarkerFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestKeywordMarkerFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
