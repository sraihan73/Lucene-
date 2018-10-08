#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class TokenStream;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class TermFrequencyAttribute;
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
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TermFrequencyAttribute =
    org::apache::lucene::analysis::tokenattributes::TermFrequencyAttribute;

class DelimitedTermFrequencyTokenFilterTest : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(DelimitedTermFrequencyTokenFilterTest)

public:
  virtual void testTermFrequency() ;

  virtual void testInvalidNegativeTf() ;

  virtual void testInvalidFloatTf() ;

  virtual void assertTermEquals(const std::wstring &expected,
                                std::shared_ptr<TokenStream> stream,
                                std::shared_ptr<CharTermAttribute> termAtt,
                                std::shared_ptr<TermFrequencyAttribute> tfAtt,
                                int expectedTf) ;

protected:
  std::shared_ptr<DelimitedTermFrequencyTokenFilterTest> shared_from_this()
  {
    return std::static_pointer_cast<DelimitedTermFrequencyTokenFilterTest>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
