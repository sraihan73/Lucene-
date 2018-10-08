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
namespace org::apache::lucene::analysis::miscellaneous
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link KeywordMarkerFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_keyword" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory"/&gt; &lt;filter
 * class="solr.KeywordMarkerFilterFactory" protected="protectedkeyword.txt"
 * pattern="^.+er$" ignoreCase="false"/&gt; &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 * @since 3.1.0
 */
class KeywordMarkerFilterFactory : public TokenFilterFactory,
                                   public ResourceLoaderAware
{
  GET_CLASS_NAME(KeywordMarkerFilterFactory)
public:
  static const std::wstring PROTECTED_TOKENS;
  static const std::wstring PATTERN;

private:
  const std::wstring wordFiles;
  const std::wstring stringPattern;
  const bool ignoreCase;
  std::shared_ptr<Pattern> pattern;
  std::shared_ptr<CharArraySet> protectedWords;

  /** Creates a new KeywordMarkerFilterFactory */
public:
  KeywordMarkerFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

  virtual bool isIgnoreCase();

  std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<KeywordMarkerFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<KeywordMarkerFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
