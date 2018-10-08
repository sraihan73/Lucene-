#pragma once
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoaderAware.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::opennlp
{
class OpenNLPChunkerFilter;
}

namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::analysis::util
{
class ResourceLoader;
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

namespace org::apache::lucene::analysis::opennlp
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link OpenNLPChunkerFilter}.
 *
 * <pre class="prettyprint">
 * &lt;fieldType name="text_opennlp_chunked" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.OpenNLPTokenizerFactory" sentenceModel="filename"
 * tokenizerModel="filename"/&gt; &lt;filter
 * class="solr.OpenNLPPOSFilterFactory" posTaggerModel="filename"/&gt;
 *     &lt;filter class="solr.OpenNLPChunkerFilterFactory"
 * chunkerModel="filename"/&gt; &lt;/analyzer&gt; &lt;/fieldType&gt;</pre>
 * @since 7.3.0
 */
class OpenNLPChunkerFilterFactory : public TokenFilterFactory,
                                    public ResourceLoaderAware
{
  GET_CLASS_NAME(OpenNLPChunkerFilterFactory)
public:
  static const std::wstring CHUNKER_MODEL;

private:
  const std::wstring chunkerModelFile;

public:
  OpenNLPChunkerFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<OpenNLPChunkerFilter>
  create(std::shared_ptr<TokenStream> in_) override;

  void inform(std::shared_ptr<ResourceLoader> loader) override;

protected:
  std::shared_ptr<OpenNLPChunkerFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<OpenNLPChunkerFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::opennlp