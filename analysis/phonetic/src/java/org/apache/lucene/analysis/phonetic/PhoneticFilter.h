#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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

using org::apache::commons::codec::Encoder;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

/**
 * Create tokens for phonetic matches.
 * @see <a
 * href="http://commons.apache.org/codec/api-release/org/apache/commons/codec/language/package-summary.html">
 * Apache Commons Codec</a>
 */
class PhoneticFilter final : public TokenFilter
{
  GET_CLASS_NAME(PhoneticFilter)
  /** true if encoded tokens should be added as synonyms */
protected:
  bool inject = true;
  /** phonetic encoder */
  std::shared_ptr<Encoder> encoder = nullptr;
  /** captured state, non-null when <code>inject=true</code> and a token is
   * buffered */
  std::shared_ptr<State> save = nullptr;

private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posAtt =
      addAttribute(PositionIncrementAttribute::typeid);

  /** Creates a PhoneticFilter with the specified encoder, and either
   *  adding encoded forms as synonyms (<code>inject=true</code>) or
   *  replacing them.
   */
public:
  PhoneticFilter(std::shared_ptr<TokenStream> in_,
                 std::shared_ptr<Encoder> encoder, bool inject);

  bool incrementToken()  override;

  void reset()  override;

protected:
  std::shared_ptr<PhoneticFilter> shared_from_this()
  {
    return std::static_pointer_cast<PhoneticFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/phonetic/
