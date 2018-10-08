#pragma once
#include "../util/MultiTermAwareComponent.h"
#include "../util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::ckb
{
class SoraniNormalizationFilter;
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
namespace org::apache::lucene::analysis::ckb
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link SoraniNormalizationFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_ckbnormal" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.StandardTokenizerFactory"/&gt; &lt;filter
 * class="solr.SoraniNormalizationFilterFactory"/&gt; &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 * @since 4.7.0
 */
class SoraniNormalizationFilterFactory : public TokenFilterFactory,
                                         public MultiTermAwareComponent
{
  GET_CLASS_NAME(SoraniNormalizationFilterFactory)

  /** Creates a new SoraniNormalizationFilterFactory */
public:
  SoraniNormalizationFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<SoraniNormalizationFilter>
  create(std::shared_ptr<TokenStream> input) override;

  std::shared_ptr<AbstractAnalysisFactory> getMultiTermComponent() override;

protected:
  std::shared_ptr<SoraniNormalizationFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<SoraniNormalizationFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ckb
