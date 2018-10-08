#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <deque>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
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
namespace org::apache::lucene::analysis::phonetic
{

using org::apache::commons::codec::language::DoubleMetaphone;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

/**
 * Filter for DoubleMetaphone (supporting secondary codes)
 */
class DoubleMetaphoneFilter final : public TokenFilter
{
  GET_CLASS_NAME(DoubleMetaphoneFilter)

private:
  static const std::wstring TOKEN_TYPE;

  const std::deque<std::shared_ptr<State>> remainingTokens =
      std::deque<std::shared_ptr<State>>();
  const std::shared_ptr<DoubleMetaphone> encoder =
      std::make_shared<DoubleMetaphone>();
  const bool inject;
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posAtt =
      addAttribute(PositionIncrementAttribute::typeid);

  /** Creates a DoubleMetaphoneFilter with the specified maximum code length,
   *  and either adding encoded forms as synonyms (<code>inject=true</code>) or
   *  replacing them.
   */
public:
  DoubleMetaphoneFilter(std::shared_ptr<TokenStream> input, int maxCodeLength,
                        bool inject);

  bool incrementToken()  override;

  void reset()  override;

protected:
  std::shared_ptr<DoubleMetaphoneFilter> shared_from_this()
  {
    return std::static_pointer_cast<DoubleMetaphoneFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/phonetic/
