#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
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
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

/**
 * A token filter for truncating the terms into a specific length.
 * Fixed prefix truncation, as a stemming method, produces good results on
 * Turkish language. It is reported that F5, using first 5 characters, produced
 * best results in <a
 * href="http://www.users.muohio.edu/canf/papers/JASIST2008offPrint.pdf">
 * Information Retrieval on Turkish Texts</a>
 */
class TruncateTokenFilter final : public TokenFilter
{
  GET_CLASS_NAME(TruncateTokenFilter)

private:
  const std::shared_ptr<CharTermAttribute> termAttribute =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAttr =
      addAttribute(KeywordAttribute::typeid);

  const int length;

public:
  TruncateTokenFilter(std::shared_ptr<TokenStream> input, int length);

  bool incrementToken()  override final;

protected:
  std::shared_ptr<TruncateTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<TruncateTokenFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
