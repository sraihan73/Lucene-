#pragma once
#include "../util/ResourceLoaderAware.h"
#include "../util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class CharArraySet;
}

namespace org::apache::lucene::analysis::util
{
class ResourceLoader;
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
namespace org::apache::lucene::analysis::core
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link StopFilter}.
 *
 * <pre class="prettyprint">
 * &lt;fieldType name="text_stop" class="solr.TextField"
 * positionIncrementGap="100" autoGeneratePhraseQueries="true"&gt;
 *   &lt;analyzer&gt;
 *     &lt;tokenizer class="solr.WhitespaceTokenizerFactory"/&gt;
 *     &lt;filter class="solr.StopFilterFactory" ignoreCase="true"
 *             words="stopwords.txt" format="wordset"
 *   &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 *
 * <p>
 * All attributes are optional:
 * </p>
 * <ul>
 *  <li><code>ignoreCase</code> defaults to <code>false</code></li>
 *  <li><code>words</code> should be the name of a stopwords file to parse, if
 * not specified the factory will use {@link
 * StopAnalyzer#ENGLISH_STOP_WORDS_SET}
 *  </li>
 *  <li><code>format</code> defines how the <code>words</code> file will be
 * parsed, and defaults to <code>wordset</code>.  If <code>words</code> is not
 * specified, then <code>format</code> must not be specified.
 *  </li>
 * </ul>
 * <p>
 * The valid values for the <code>format</code> option are:
 * </p>
 * <ul>
 *  <li><code>wordset</code> - This is the default format, which supports one
 * word per line (including any intra-word whitespace) and allows whole line
 * comments beginning with the "#" character.  Blank lines are ignored.  See
 *      {@link WordlistLoader#getLines WordlistLoader.getLines} for details.
 *  </li>
 *  <li><code>snowball</code> - This format allows for multiple words specified
 * on each line, and trailing comments may be specified using the vertical line
 * ("&#124;"). Blank lines are ignored.  See
 *      {@link WordlistLoader#getSnowballWordSet
 * WordlistLoader.getSnowballWordSet} for details.
 *  </li>
 * </ul>
 */
class StopFilterFactory : public TokenFilterFactory, public ResourceLoaderAware
{
  GET_CLASS_NAME(StopFilterFactory)
public:
  static const std::wstring FORMAT_WORDSET;
  static const std::wstring FORMAT_SNOWBALL;

private:
  std::shared_ptr<CharArraySet> stopWords;
  const std::wstring stopWordFiles;
  const std::wstring format;
  const bool ignoreCase;

  /** Creates a new StopFilterFactory */
public:
  StopFilterFactory(std::unordered_map<std::wstring, std::wstring> &args);

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

  virtual bool isIgnoreCase();

  virtual std::shared_ptr<CharArraySet> getStopWords();

  std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<StopFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<StopFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::core
