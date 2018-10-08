#pragma once
#include "../util/ResourceLoaderAware.h"
#include "../util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::miscellaneous
{
class StemmerOverrideFilter;
}

namespace org::apache::lucene::analysis::miscellaneous
{
class StemmerOverrideMap;
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
namespace org::apache::lucene::analysis::miscellaneous
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using StemmerOverrideMap = org::apache::lucene::analysis::miscellaneous::
    StemmerOverrideFilter::StemmerOverrideMap;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link StemmerOverrideFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_dicstem" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory"/&gt; &lt;filter
 * class="solr.StemmerOverrideFilterFactory" dictionary="dictionary.txt"
 * ignoreCase="false"/&gt; &lt;/analyzer&gt; &lt;/fieldType&gt;</pre>
 * @since 3.1.0
 */
class StemmerOverrideFilterFactory : public TokenFilterFactory,
                                     public ResourceLoaderAware
{
  GET_CLASS_NAME(StemmerOverrideFilterFactory)
private:
  std::shared_ptr<StemmerOverrideMap> dictionary;
  const std::wstring dictionaryFiles;
  const bool ignoreCase;

  /** Creates a new StemmerOverrideFilterFactory */
public:
  StemmerOverrideFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

  virtual bool isIgnoreCase();

  std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<StemmerOverrideFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<StemmerOverrideFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
