#pragma once
#include "../util/ResourceLoaderAware.h"
#include "../util/TokenFilterFactory.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/hunspell/Dictionary.h"

#include  "core/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
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
namespace org::apache::lucene::analysis::hunspell
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * TokenFilterFactory that creates instances of {@link HunspellStemFilter}.
 * Example config for British English:
 * <pre class="prettyprint">
 * &lt;filter class=&quot;solr.HunspellStemFilterFactory&quot;
 *         dictionary=&quot;en_GB.dic,my_custom.dic&quot;
 *         affix=&quot;en_GB.aff&quot;
 *         ignoreCase=&quot;false&quot;
 *         longestOnly=&quot;false&quot; /&gt;</pre>
 * Both parameters dictionary and affix are mandatory.
 * Dictionaries for many languages are available through the OpenOffice project.
 *
 * See <a
 * href="http://wiki.apache.org/solr/Hunspell">http://wiki.apache.org/solr/Hunspell</a>
 * @lucene.experimental
 * @since 3.5.0
 */
class HunspellStemFilterFactory : public TokenFilterFactory,
                                  public ResourceLoaderAware
{
  GET_CLASS_NAME(HunspellStemFilterFactory)
private:
  static const std::wstring PARAM_DICTIONARY;
  static const std::wstring PARAM_AFFIX;
  // NOTE: this one is currently unused?:
  static const std::wstring PARAM_RECURSION_CAP;
  static const std::wstring PARAM_IGNORE_CASE;
  static const std::wstring PARAM_LONGEST_ONLY;

  const std::wstring dictionaryFiles;
  const std::wstring affixFile;
  const bool ignoreCase;
  const bool longestOnly;
  std::shared_ptr<Dictionary> dictionary;

  /** Creates a new HunspellStemFilterFactory */
public:
  HunspellStemFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

  std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> tokenStream) override;

protected:
  std::shared_ptr<HunspellStemFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<HunspellStemFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/hunspell/
