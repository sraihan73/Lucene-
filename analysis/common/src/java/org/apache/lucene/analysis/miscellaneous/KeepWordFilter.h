#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/FilteringTokenFilter.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class CharArraySet;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
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
using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * A TokenFilter that only keeps tokens with text contained in the
 * required words.  This filter behaves like the inverse of StopFilter.
 *
 * @since solr 1.3
 */
class KeepWordFilter final : public FilteringTokenFilter
{
  GET_CLASS_NAME(KeepWordFilter)
private:
  const std::shared_ptr<CharArraySet> words;
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);

  /**
   * Create a new {@link KeepWordFilter}.
   * <p><b>NOTE</b>: The words set passed to this constructor will be directly
   * used by this filter and should not be modified.
   * @param in      the {@link TokenStream} to consume
   * @param words   the words to keep
   */
public:
  KeepWordFilter(std::shared_ptr<TokenStream> in_,
                 std::shared_ptr<CharArraySet> words);

  bool accept() override;

protected:
  std::shared_ptr<KeepWordFilter> shared_from_this()
  {
    return std::static_pointer_cast<KeepWordFilter>(
        org.apache.lucene.analysis.FilteringTokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
