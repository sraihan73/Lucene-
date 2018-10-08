#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
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

using namespace org::apache::lucene::analysis;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * When the plain text is extracted from documents, we will often have many
 * words hyphenated and broken into two lines. This is often the case with
 * documents where narrow text columns are used, such as newsletters. In order
 * to increase search efficiency, this filter puts hyphenated words broken into
 * two lines back together. This filter should be used on indexing time only.
 * Example field definition in schema.xml:
 * <pre class="prettyprint">
 * &lt;fieldtype name="text" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer type="index"&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory"/&gt; &lt;filter
 * class="solr.SynonymFilterFactory" synonyms="index_synonyms.txt"
 * ignoreCase="true" expand="false"/&gt; &lt;filter
 * class="solr.StopFilterFactory" ignoreCase="true"/&gt; &lt;filter
 * class="solr.HyphenatedWordsFilterFactory"/&gt; &lt;filter
 * class="solr.WordDelimiterFilterFactory" generateWordParts="1"
 * generateNumberParts="1" catenateWords="1" catenateNumbers="1"
 * catenateAll="0"/&gt; &lt;filter class="solr.LowerCaseFilterFactory"/&gt;
 *      &lt;filter class="solr.RemoveDuplicatesTokenFilterFactory"/&gt;
 *  &lt;/analyzer&gt;
 *  &lt;analyzer type="query"&gt;
 *      &lt;tokenizer class="solr.WhitespaceTokenizerFactory"/&gt;
 *      &lt;filter class="solr.SynonymFilterFactory" synonyms="synonyms.txt"
 * ignoreCase="true" expand="true"/&gt; &lt;filter
 * class="solr.StopFilterFactory" ignoreCase="true"/&gt; &lt;filter
 * class="solr.WordDelimiterFilterFactory" generateWordParts="1"
 * generateNumberParts="1" catenateWords="0" catenateNumbers="0"
 * catenateAll="0"/&gt; &lt;filter class="solr.LowerCaseFilterFactory"/&gt;
 *      &lt;filter class="solr.RemoveDuplicatesTokenFilterFactory"/&gt;
 *  &lt;/analyzer&gt;
 * &lt;/fieldtype&gt;
 * </pre>
 *
 */
class HyphenatedWordsFilter final : public TokenFilter
{
  GET_CLASS_NAME(HyphenatedWordsFilter)

private:
  const std::shared_ptr<CharTermAttribute> termAttribute =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAttribute =
      addAttribute(OffsetAttribute::typeid);

  const std::shared_ptr<StringBuilder> hyphenated =
      std::make_shared<StringBuilder>();
  std::shared_ptr<State> savedState;
  bool exhausted = false;
  int lastEndOffset = 0;

  /**
   * Creates a new HyphenatedWordsFilter
   *
   * @param in TokenStream that will be filtered
   */
public:
  HyphenatedWordsFilter(std::shared_ptr<TokenStream> in_);

  bool incrementToken()  override;

  void reset()  override;

  // ================================================= Helper Methods
  // ================================================

  /**
   * Writes the joined unhyphenated term
   */
private:
  void unhyphenate();

protected:
  std::shared_ptr<HyphenatedWordsFilter> shared_from_this()
  {
    return std::static_pointer_cast<HyphenatedWordsFilter>(
        TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
