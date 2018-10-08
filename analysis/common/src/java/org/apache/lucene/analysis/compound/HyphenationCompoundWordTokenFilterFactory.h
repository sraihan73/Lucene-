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

namespace org::apache::lucene::analysis::compound::hyphenation
{
class HyphenationTree;
}
namespace org::apache::lucene::analysis::util
{
class ResourceLoader;
}
namespace org::apache::lucene::analysis
{
class TokenFilter;
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
namespace org::apache::lucene::analysis::compound
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using HyphenationTree =
    org::apache::lucene::analysis::compound::hyphenation::HyphenationTree;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link HyphenationCompoundWordTokenFilter}.
 * <p>
 * This factory accepts the following parameters:
 * <ul>
 *  <li><code>hyphenator</code> (mandatory): path to the FOP xml hyphenation
 * pattern. See <a
 * href="http://offo.sourceforge.net/hyphenation/">http://offo.sourceforge.net/hyphenation/</a>.
 *  <li><code>encoding</code> (optional): encoding of the xml hyphenation file.
 * defaults to UTF-8. <li><code>dictionary</code> (optional): dictionary of
 * words. defaults to no dictionary. <li><code>minWordSize</code> (optional):
 * minimal word length that gets decomposed. defaults to 5.
 *  <li><code>minSubwordSize</code> (optional): minimum length of subwords.
 * defaults to 2. <li><code>maxSubwordSize</code> (optional): maximum length of
 * subwords. defaults to 15. <li><code>onlyLongestMatch</code> (optional): if
 * true, adds only the longest matching subword to the stream. defaults to
 * false.
 * </ul>
 * <br>
 * <pre class="prettyprint">
 * &lt;fieldType name="text_hyphncomp" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory"/&gt; &lt;filter
 * class="solr.HyphenationCompoundWordTokenFilterFactory"
 * hyphenator="hyphenator.xml" encoding="UTF-8" dictionary="dictionary.txt"
 * minWordSize="5" minSubwordSize="2" maxSubwordSize="15"
 * onlyLongestMatch="false"/&gt; &lt;/analyzer&gt; &lt;/fieldType&gt;</pre>
 *
 * @see HyphenationCompoundWordTokenFilter
 * @since 3.1.0
 */
class HyphenationCompoundWordTokenFilterFactory : public TokenFilterFactory,
                                                  public ResourceLoaderAware
{
  GET_CLASS_NAME(HyphenationCompoundWordTokenFilterFactory)
private:
  std::shared_ptr<CharArraySet> dictionary;
  std::shared_ptr<HyphenationTree> hyphenator;
  const std::wstring dictFile;
  const std::wstring hypFile;
  const std::wstring encoding;
  const int minWordSize;
  const int minSubwordSize;
  const int maxSubwordSize;
  const bool onlyLongestMatch;

  /** Creates a new HyphenationCompoundWordTokenFilterFactory */
public:
  HyphenationCompoundWordTokenFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

  std::shared_ptr<TokenFilter>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<HyphenationCompoundWordTokenFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<HyphenationCompoundWordTokenFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::compound
