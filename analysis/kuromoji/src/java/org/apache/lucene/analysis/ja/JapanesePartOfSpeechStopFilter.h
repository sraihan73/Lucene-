#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/FilteringTokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::ja::tokenattributes
{
class PartOfSpeechAttribute;
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
namespace org::apache::lucene::analysis::ja
{

using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PartOfSpeechAttribute =
    org::apache::lucene::analysis::ja::tokenattributes::PartOfSpeechAttribute;

/**
 * Removes tokens that match a set of part-of-speech tags.
 */
class JapanesePartOfSpeechStopFilter final : public FilteringTokenFilter
{
  GET_CLASS_NAME(JapanesePartOfSpeechStopFilter)
private:
  const std::shared_ptr<Set<std::wstring>> stopTags;
  const std::shared_ptr<PartOfSpeechAttribute> posAtt =
      addAttribute(PartOfSpeechAttribute::typeid);

  /**
   * Create a new {@link JapanesePartOfSpeechStopFilter}.
   * @param input    the {@link TokenStream} to consume
   * @param stopTags the part-of-speech tags that should be removed
   */
public:
  JapanesePartOfSpeechStopFilter(std::shared_ptr<TokenStream> input,
                                 std::shared_ptr<Set<std::wstring>> stopTags);

protected:
  bool accept() override;

protected:
  std::shared_ptr<JapanesePartOfSpeechStopFilter> shared_from_this()
  {
    return std::static_pointer_cast<JapanesePartOfSpeechStopFilter>(
        org.apache.lucene.analysis.FilteringTokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ja
