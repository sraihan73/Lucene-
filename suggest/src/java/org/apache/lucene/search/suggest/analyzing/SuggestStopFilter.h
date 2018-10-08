#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class KeywordAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis
{
class CharArraySet;
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
namespace org::apache::lucene::search::suggest::analyzing
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

/** Like {@link StopFilter} except it will not remove the
 *  last token if that token was not followed by some token
 *  separator.  For example, a query 'find the' would
 *  preserve the 'the' since it was not followed by a space or
 *  punctuation or something, and mark it KEYWORD so future
 *  stemmers won't touch it either while a query like "find
 *  the popsicle' would remove 'the' as a stopword.
 *
 *  <p>Normally you'd use the ordinary {@link StopFilter}
 *  in your indexAnalyzer and then this class in your
 *  queryAnalyzer, when using one of the analyzing suggesters. */

class SuggestStopFilter final : public TokenFilter
{
  GET_CLASS_NAME(SuggestStopFilter)

private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAtt =
      addAttribute(KeywordAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<CharArraySet> stopWords;

  std::shared_ptr<State> endState;

  /** Sole constructor. */
public:
  SuggestStopFilter(std::shared_ptr<TokenStream> input,
                    std::shared_ptr<CharArraySet> stopWords);

  void reset()  override;

  void end()  override;

  bool incrementToken()  override;

protected:
  std::shared_ptr<SuggestStopFilter> shared_from_this()
  {
    return std::static_pointer_cast<SuggestStopFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::analyzing
