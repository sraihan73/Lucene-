#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
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

namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PayloadAttribute;
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
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;

class DelimitedPayloadTokenFilterTest : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(DelimitedPayloadTokenFilterTest)

public:
  virtual void testPayloads() ;

  virtual void testNext() ;

  virtual void testFloatEncoding() ;

  virtual void testIntEncoding() ;

  virtual void
  assertTermEquals(const std::wstring &expected,
                   std::shared_ptr<TokenStream> stream,
                   std::deque<char> &expectPay) ;

  virtual void
  assertTermEquals(const std::wstring &expected,
                   std::shared_ptr<TokenStream> stream,
                   std::shared_ptr<CharTermAttribute> termAtt,
                   std::shared_ptr<PayloadAttribute> payAtt,
                   std::deque<char> &expectPay) ;

protected:
  std::shared_ptr<DelimitedPayloadTokenFilterTest> shared_from_this()
  {
    return std::static_pointer_cast<DelimitedPayloadTokenFilterTest>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::payloads
