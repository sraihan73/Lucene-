#pragma once
#include "MultiTermAwareComponent.h"
#include "ResourceLoaderAware.h"
#include "TokenFilterFactory.h"
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
namespace org::apache::lucene::analysis::util
{
class ElisionFilter;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::analysis::util
{
class AbstractAnalysisFactory;
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
namespace org::apache::lucene::analysis::util
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * Factory for {@link ElisionFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_elsn" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.StandardTokenizerFactory"/&gt; &lt;filter
 * class="solr.LowerCaseFilterFactory"/&gt; &lt;filter
 * class="solr.ElisionFilterFactory" articles="stopwordarticles.txt"
 * ignoreCase="true"/&gt; &lt;/analyzer&gt; &lt;/fieldType&gt;</pre>
 */
class ElisionFilterFactory : public TokenFilterFactory,
                             public ResourceLoaderAware,
                             public MultiTermAwareComponent
{
  GET_CLASS_NAME(ElisionFilterFactory)
private:
  const std::wstring articlesFile;
  const bool ignoreCase;
  std::shared_ptr<CharArraySet> articles;

  /** Creates a new ElisionFilterFactory */
public:
  ElisionFilterFactory(std::unordered_map<std::wstring, std::wstring> &args);

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

  std::shared_ptr<ElisionFilter>
  create(std::shared_ptr<TokenStream> input) override;

  std::shared_ptr<AbstractAnalysisFactory> getMultiTermComponent() override;

protected:
  std::shared_ptr<ElisionFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<ElisionFilterFactory>(
        TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::util
