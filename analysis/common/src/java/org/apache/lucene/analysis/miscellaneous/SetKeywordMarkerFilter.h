#pragma once
#include "KeywordMarkerFilter.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"
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
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

/**
 * Marks terms as keywords via the {@link KeywordAttribute}. Each token
 * contained in the provided set is marked as a keyword by setting
 * {@link KeywordAttribute#setKeyword(bool)} to <code>true</code>.
 */
class SetKeywordMarkerFilter final : public KeywordMarkerFilter
{
  GET_CLASS_NAME(SetKeywordMarkerFilter)
private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<CharArraySet> keywordSet;

  /**
   * Create a new KeywordSetMarkerFilter, that marks the current token as a
   * keyword if the tokens term buffer is contained in the given set via the
   * {@link KeywordAttribute}.
   *
   * @param in
   *          TokenStream to filter
   * @param keywordSet
   *          the keywords set to lookup the current termbuffer
   */
public:
  SetKeywordMarkerFilter(std::shared_ptr<TokenStream> in_,
                         std::shared_ptr<CharArraySet> keywordSet);

protected:
  bool isKeyword() override;

protected:
  std::shared_ptr<SetKeywordMarkerFilter> shared_from_this()
  {
    return std::static_pointer_cast<SetKeywordMarkerFilter>(
        KeywordMarkerFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
