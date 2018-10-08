#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
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
namespace org::apache::lucene::analysis::pattern
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;

/**
 * CaptureGroup uses Java regexes to emit multiple tokens - one for each capture
 * group in one or more patterns.
 *
 * <p>
 * For example, a pattern like:
 * </p>
 *
 * <p>
 * <code>"(https?://([a-zA-Z\-_0-9.]+))"</code>
 * </p>
 *
 * <p>
 * when matched against the string "http://www.foo.com/index" would return the
 * tokens "https://www.foo.com" and "www.foo.com".
 * </p>
 *
 * <p>
 * If none of the patterns match, or if preserveOriginal is true, the original
 * token will be preserved.
 * </p>
 * <p>
 * Each pattern is matched as often as it can be, so the pattern
 * <code> "(...)"</code>, when matched against <code>"abcdefghi"</code> would
 * produce <code>["abc","def","ghi"]</code>
 * </p>
 * <p>
 * A camelCaseFilter could be written as:
 * </p>
 * <p>
 * <code>
 *   "([A-Z]{2,})",
 *   "(?&lt;![A-Z])([A-Z][a-z]+)",
 *   "(?:^|\\b|(?&lt;=[0-9_])|(?&lt;=[A-Z]{2}))([a-z]+)",
 *   "([0-9]+)"
 * </code>
 * </p>
 * <p>
 * plus if {@link #preserveOriginal} is true, it would also return
 * <code>"camelCaseFilter"</code>
 * </p>
 */
class PatternCaptureGroupTokenFilter final : public TokenFilter
{
  GET_CLASS_NAME(PatternCaptureGroupTokenFilter)

private:
  const std::shared_ptr<CharTermAttribute> charTermAttr =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posAttr =
      addAttribute(PositionIncrementAttribute::typeid);
  std::shared_ptr<State> state;
  std::deque<std::shared_ptr<Matcher>> const matchers;
  const std::shared_ptr<CharsRefBuilder> spare =
      std::make_shared<CharsRefBuilder>();
  std::deque<int> const groupCounts;
  const bool preserveOriginal;
  std::deque<int> currentGroup;
  int currentMatcher = 0;

  /**
   * @param input
   *          the input {@link TokenStream}
   * @param preserveOriginal
   *          set to true to return the original token even if one of the
   *          patterns matches
   * @param patterns
   *          an array of {@link Pattern} objects to match against each token
   */

public:
  PatternCaptureGroupTokenFilter(std::shared_ptr<TokenStream> input,
                                 bool preserveOriginal,
                                 std::deque<Pattern> &patterns);

private:
  bool nextCapture();

public:
  bool incrementToken()  override;

  void reset()  override;

protected:
  std::shared_ptr<PatternCaptureGroupTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<PatternCaptureGroupTokenFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/pattern/
