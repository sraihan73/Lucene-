#pragma once
#include "../util/MultiTermAwareComponent.h"
#include "../util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::el
{
class GreekLowerCaseFilter;
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
namespace org::apache::lucene::analysis::el
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using GreekLowerCaseFilter =
    org::apache::lucene::analysis::el::GreekLowerCaseFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link GreekLowerCaseFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_glc" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.StandardTokenizerFactory"/&gt; &lt;filter
 * class="solr.GreekLowerCaseFilterFactory"/&gt; &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 */
class GreekLowerCaseFilterFactory : public TokenFilterFactory,
                                    public MultiTermAwareComponent
{
  GET_CLASS_NAME(GreekLowerCaseFilterFactory)

  /** Creates a new GreekLowerCaseFilterFactory */
public:
  GreekLowerCaseFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<GreekLowerCaseFilter>
  create(std::shared_ptr<TokenStream> in_) override;

  std::shared_ptr<AbstractAnalysisFactory> getMultiTermComponent() override;

protected:
  std::shared_ptr<GreekLowerCaseFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<GreekLowerCaseFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::el
