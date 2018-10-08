#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class KeywordAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
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

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

/**
 * This TokenFilter emits each incoming token twice once as keyword and once
 * non-keyword, in other words once with
 * {@link KeywordAttribute#setKeyword(bool)} set to <code>true</code> and
 * once set to <code>false</code>. This is useful if used with a stem filter
 * that respects the {@link KeywordAttribute} to index the stemmed and the
 * un-stemmed version of a term into the same field.
 */
class KeywordRepeatFilter final : public TokenFilter
{
  GET_CLASS_NAME(KeywordRepeatFilter)

private:
  const std::shared_ptr<KeywordAttribute> keywordAttribute =
      addAttribute(KeywordAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncAttr =
      addAttribute(PositionIncrementAttribute::typeid);
  std::shared_ptr<State> state;

  /**
   * Construct a token stream filtering the given input.
   */
public:
  KeywordRepeatFilter(std::shared_ptr<TokenStream> input);

  bool incrementToken()  override;

  void reset()  override;

protected:
  std::shared_ptr<KeywordRepeatFilter> shared_from_this()
  {
    return std::static_pointer_cast<KeywordRepeatFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
