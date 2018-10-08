#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TermFrequencyAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
using TermFrequencyAttribute =
    org::apache::lucene::analysis::tokenattributes::TermFrequencyAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * Characters before the delimiter are the "token", the textual integer after is
 * the term frequency. To use this {@code TokenFilter} the field must be indexed
 * with
 * {@link IndexOptions#DOCS_AND_FREQS} but no positions or offsets.
 * <p>
 * For example, if the delimiter is '|', then for the string "foo|5", "foo" is
 * the token and "5" is a term frequency. If there is no delimiter, the
 * TokenFilter does not modify the term frequency. <p> Note make sure your
 * Tokenizer doesn't split on the delimiter, or this won't work
 */
class DelimitedTermFrequencyTokenFilter final : public TokenFilter
{
  GET_CLASS_NAME(DelimitedTermFrequencyTokenFilter)
public:
  static constexpr wchar_t DEFAULT_DELIMITER = L'|';

private:
  const wchar_t delimiter;
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<TermFrequencyAttribute> tfAtt =
      addAttribute(TermFrequencyAttribute::typeid);

public:
  DelimitedTermFrequencyTokenFilter(std::shared_ptr<TokenStream> input);

  DelimitedTermFrequencyTokenFilter(std::shared_ptr<TokenStream> input,
                                    wchar_t delimiter);

  bool incrementToken()  override;

protected:
  std::shared_ptr<DelimitedTermFrequencyTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<DelimitedTermFrequencyTokenFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
