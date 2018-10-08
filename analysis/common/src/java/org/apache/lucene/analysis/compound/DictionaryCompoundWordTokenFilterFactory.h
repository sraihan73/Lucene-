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
namespace org::apache::lucene::analysis::compound
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link DictionaryCompoundWordTokenFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_dictcomp" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory"/&gt; &lt;filter
 * class="solr.DictionaryCompoundWordTokenFilterFactory"
 * dictionary="dictionary.txt" minWordSize="5" minSubwordSize="2"
 * maxSubwordSize="15" onlyLongestMatch="true"/&gt; &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 */
class DictionaryCompoundWordTokenFilterFactory : public TokenFilterFactory,
                                                 public ResourceLoaderAware
{
  GET_CLASS_NAME(DictionaryCompoundWordTokenFilterFactory)
private:
  std::shared_ptr<CharArraySet> dictionary;
  const std::wstring dictFile;
  const int minWordSize;
  const int minSubwordSize;
  const int maxSubwordSize;
  const bool onlyLongestMatch;

  /** Creates a new DictionaryCompoundWordTokenFilterFactory */
public:
  DictionaryCompoundWordTokenFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

  std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<DictionaryCompoundWordTokenFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<DictionaryCompoundWordTokenFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::compound
