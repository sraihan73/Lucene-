#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
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

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

/**
 * A TokenFilter which filters out Tokens at the same position and Term text as
 * the previous token in the stream.
 */
class RemoveDuplicatesTokenFilter final : public TokenFilter
{
  GET_CLASS_NAME(RemoveDuplicatesTokenFilter)

private:
  const std::shared_ptr<CharTermAttribute> termAttribute =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncAttribute =
      addAttribute(PositionIncrementAttribute::typeid);

  const std::shared_ptr<CharArraySet> previous =
      std::make_shared<CharArraySet>(8, false);

  /**
   * Creates a new RemoveDuplicatesTokenFilter
   *
   * @param in TokenStream that will be filtered
   */
public:
  RemoveDuplicatesTokenFilter(std::shared_ptr<TokenStream> in_);

  bool incrementToken()  override;

  void reset()  override;

protected:
  std::shared_ptr<RemoveDuplicatesTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<RemoveDuplicatesTokenFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
