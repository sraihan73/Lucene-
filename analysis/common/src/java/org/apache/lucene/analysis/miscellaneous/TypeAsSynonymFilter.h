#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"
#include  "core/src/java/org/apache/lucene/util/State.h"
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
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

/**
 * Adds the {@link TypeAttribute#type()} as a synonym,
 * i.e. another token at the same position, optionally with a specified prefix
 * prepended.
 */
class TypeAsSynonymFilter final : public TokenFilter
{
  GET_CLASS_NAME(TypeAsSynonymFilter)
private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAtt =
      addAttribute(TypeAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::wstring prefix;

public:
  std::shared_ptr<AttributeSource::State> savedToken = nullptr;

  TypeAsSynonymFilter(std::shared_ptr<TokenStream> input);

  /**
   * @param input input tokenstream
   * @param prefix Prepend this string to every token type emitted as token
   * text. If null, nothing will be prepended.
   */
  TypeAsSynonymFilter(std::shared_ptr<TokenStream> input,
                      const std::wstring &prefix);

  bool incrementToken()  override;

  void reset()  override;

protected:
  std::shared_ptr<TypeAsSynonymFilter> shared_from_this()
  {
    return std::static_pointer_cast<TypeAsSynonymFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
