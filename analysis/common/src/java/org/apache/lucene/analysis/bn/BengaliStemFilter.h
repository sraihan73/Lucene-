#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class KeywordAttribute;
}
namespace org::apache::lucene::analysis::bn
{
class BengaliStemmer;
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
namespace org::apache::lucene::analysis::bn
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

/**
 * A {@link TokenFilter} that applies {@link BengaliStemmer} to stem Bengali
 * words.
 */
class BengaliStemFilter final : public TokenFilter
{
  GET_CLASS_NAME(BengaliStemFilter)
private:
  const std::shared_ptr<CharTermAttribute> termAttribute =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAttribute =
      addAttribute(KeywordAttribute::typeid);
  const std::shared_ptr<BengaliStemmer> bengaliStemmer =
      std::make_shared<BengaliStemmer>();

public:
  BengaliStemFilter(std::shared_ptr<TokenStream> input);

  bool incrementToken()  override;

protected:
  std::shared_ptr<BengaliStemFilter> shared_from_this()
  {
    return std::static_pointer_cast<BengaliStemFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::bn
