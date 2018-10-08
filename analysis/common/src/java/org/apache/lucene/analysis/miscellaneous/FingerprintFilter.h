#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionLengthAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class TypeAttribute;
}
namespace org::apache::lucene::analysis
{
class CharArraySet;
}
namespace org::apache::lucene::util
{
class AttributeSource;
}
namespace org::apache::lucene::util
{
class State;
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

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

/**
 * Filter outputs a single token which is a concatenation of the sorted and
 * de-duplicated set of input tokens. This can be useful for clustering/linking
 * use cases.
 */
class FingerprintFilter : public TokenFilter
{
  GET_CLASS_NAME(FingerprintFilter)

public:
  static constexpr int DEFAULT_MAX_OUTPUT_TOKEN_SIZE = 1024;
  static constexpr wchar_t DEFAULT_SEPARATOR = L' ';

private:
  const std::shared_ptr<CharTermAttribute> termAttribute =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLenAtt =
      addAttribute(PositionLengthAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAtt =
      addAttribute(TypeAttribute::typeid);

  std::shared_ptr<CharArraySet> uniqueTerms = nullptr;
  const int maxOutputTokenSize;
  std::shared_ptr<AttributeSource::State> finalState;

  const wchar_t separator;
  bool inputEnded = false;

  /**
   * Create a new FingerprintFilter with default settings
   */
public:
  FingerprintFilter(std::shared_ptr<TokenStream> input);

  /**
   * Create a new FingerprintFilter with control over all settings
   *
   * @param input
   *          the source of tokens to be summarized into a single token
   * @param maxOutputTokenSize
   *          the maximum length of the summarized output token. If exceeded, no
   *          output token is emitted
   * @param separator
   *          the character used to separate tokens combined into the single
   *          output token
   */
  FingerprintFilter(std::shared_ptr<TokenStream> input, int maxOutputTokenSize,
                    wchar_t separator);

  bool incrementToken()  override final;

  /**
   * Gathers all tokens from input, de-duplicates, sorts then concatenates.
   *
   * @return false for end of stream; true otherwise
   */
private:
  bool buildSingleOutputToken() ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::any>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<FingerprintFilter> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<FingerprintFilter> outerInstance);

    int compare(std::any o1, std::any o2);
  };

public:
  void end()  override final;

  void reset()  override;

protected:
  std::shared_ptr<FingerprintFilter> shared_from_this()
  {
    return std::static_pointer_cast<FingerprintFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
