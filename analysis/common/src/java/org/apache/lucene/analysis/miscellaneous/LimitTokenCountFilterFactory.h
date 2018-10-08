#pragma once
#include "../util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
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
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link LimitTokenCountFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_lngthcnt" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory"/&gt; &lt;filter
 * class="solr.LimitTokenCountFilterFactory" maxTokenCount="10"
 * consumeAllTokens="false" /&gt; &lt;/analyzer&gt; &lt;/fieldType&gt;</pre> <p>
 * The {@code consumeAllTokens} property is optional and defaults to {@code
 * false}. See {@link LimitTokenCountFilter} for an explanation of its use.
 * @since 3.1.0
 */
class LimitTokenCountFilterFactory : public TokenFilterFactory
{
  GET_CLASS_NAME(LimitTokenCountFilterFactory)

public:
  static const std::wstring MAX_TOKEN_COUNT_KEY;
  static const std::wstring CONSUME_ALL_TOKENS_KEY;
  const int maxTokenCount;
  const bool consumeAllTokens;

  /** Creates a new LimitTokenCountFilterFactory */
  LimitTokenCountFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<LimitTokenCountFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<LimitTokenCountFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous