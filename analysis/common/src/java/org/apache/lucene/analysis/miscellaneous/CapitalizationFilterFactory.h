#pragma once
#include "../util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

#include  "core/src/java/org/apache/lucene/analysis/miscellaneous/CapitalizationFilter.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link CapitalizationFilter}.
 * <p>
 * The factory takes parameters:
 * <ul>
 * <li> "onlyFirstWord" - should each word be capitalized or all of the words?
 * <li> "keep" - a keep word deque.  Each word that should be kept separated by
 * whitespace. <li> "keepIgnoreCase - true or false.  If true, the keep deque
 * will be considered case-insensitive. <li> "forceFirstLetter" - Force the
 * first letter to be capitalized even if it is in the keep deque <li> "okPrefix"
 * - do not change word capitalization if a word begins with something in this
 * deque. for example if "McK" is on the okPrefix deque, the word "McKinley"
 * should not be changed to "Mckinley" <li> "minWordLength" - how long the word
 * needs to be to get capitalization applied.  If the minWordLength is 3, "and"
 * &gt; "And" but "or" stays "or" <li>"maxWordCount" - if the token contains
 * more then maxWordCount words, the capitalization is assumed to be correct.
 * </ul>
 *
 * <pre class="prettyprint">
 * &lt;fieldType name="text_cptlztn" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory"/&gt; &lt;filter
 * class="solr.CapitalizationFilterFactory" onlyFirstWord="true" keep="java solr
 * lucene" keepIgnoreCase="false" okPrefix="McK McD McA"/&gt; &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 *
 * @since solr 1.3
 */
class CapitalizationFilterFactory : public TokenFilterFactory
{
  GET_CLASS_NAME(CapitalizationFilterFactory)
public:
  static const std::wstring KEEP;
  static const std::wstring KEEP_IGNORE_CASE;
  static const std::wstring OK_PREFIX;
  static const std::wstring MIN_WORD_LENGTH;
  static const std::wstring MAX_WORD_COUNT;
  static const std::wstring MAX_TOKEN_LENGTH;
  static const std::wstring ONLY_FIRST_WORD;
  static const std::wstring FORCE_FIRST_LETTER;

  std::shared_ptr<CharArraySet> keep;

  std::shared_ptr<std::deque<std::deque<wchar_t>>> okPrefix =
      Collections::emptyList(); // for Example: McK

  const int
      minWordLength; // don't modify capitalization for words shorter then this
  const int maxWordCount;
  const int maxTokenLength;
  const bool onlyFirstWord;
  const bool forceFirstLetter; // make sure the first letter is capital even if
                               // it is in the keep deque

  /** Creates a new CapitalizationFilterFactory */
  CapitalizationFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<CapitalizationFilter>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<CapitalizationFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<CapitalizationFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
