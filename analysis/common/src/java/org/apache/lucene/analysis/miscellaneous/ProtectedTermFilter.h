#pragma once
#include "ConditionalTokenFilter.h"
#include "stringhelper.h"
#include <functional>
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class CharArraySet;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
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

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * A ConditionalTokenFilter that only applies its wrapped filters to tokens that
 * are not contained in a protected set.
 */
class ProtectedTermFilter : public ConditionalTokenFilter
{
  GET_CLASS_NAME(ProtectedTermFilter)

private:
  const std::shared_ptr<CharArraySet> protectedTerms;

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);

  /**
   * Creates a new ProtectedTermFilter
   * @param protectedTerms  the set of terms to skip the wrapped filters for
   * @param input         the input TokenStream
   * @param inputFactory  a factory function to create the wrapped filter(s)
   */
public:
  ProtectedTermFilter(
      std::shared_ptr<CharArraySet> protectedTerms,
      std::shared_ptr<TokenStream> input,
      std::function<TokenStream *(TokenStream *)> &inputFactory);

protected:
  bool shouldFilter() override;

protected:
  std::shared_ptr<ProtectedTermFilter> shared_from_this()
  {
    return std::static_pointer_cast<ProtectedTermFilter>(
        ConditionalTokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
