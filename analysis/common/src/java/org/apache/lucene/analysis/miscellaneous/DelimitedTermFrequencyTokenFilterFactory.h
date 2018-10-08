#pragma once
#include "../util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::miscellaneous
{
class DelimitedTermFrequencyTokenFilter;
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
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link DelimitedTermFrequencyTokenFilter}. The field must have
 * {@code omitPositions=true}. <pre class="prettyprint"> &lt;fieldType
 * name="text_tfdl" class="solr.TextField" omitPositions="true"&gt;
 *   &lt;analyzer&gt;
 *     &lt;tokenizer class="solr.WhitespaceTokenizerFactory"/&gt;
 *     &lt;filter class="solr.DelimitedTermFrequencyTokenFilterFactory"
 * delimiter="|"/&gt; &lt;/analyzer&gt; &lt;/fieldType&gt;</pre>
 * @since 7.0.0
 */
class DelimitedTermFrequencyTokenFilterFactory : public TokenFilterFactory
{
  GET_CLASS_NAME(DelimitedTermFrequencyTokenFilterFactory)
public:
  static const std::wstring DELIMITER_ATTR;

private:
  const wchar_t delimiter;

  /** Creates a new DelimitedPayloadTokenFilterFactory */
public:
  DelimitedTermFrequencyTokenFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<DelimitedTermFrequencyTokenFilter>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<DelimitedTermFrequencyTokenFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<DelimitedTermFrequencyTokenFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
