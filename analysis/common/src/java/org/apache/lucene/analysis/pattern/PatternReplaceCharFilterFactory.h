#pragma once
#include "../util/CharFilterFactory.h"
#include "../util/MultiTermAwareComponent.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class CharFilter;
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
namespace org::apache::lucene::analysis::pattern
{

using CharFilter = org::apache::lucene::analysis::CharFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;

/**
 * Factory for {@link PatternReplaceCharFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_ptnreplace" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;charFilter
 * class="solr.PatternReplaceCharFilterFactory" pattern="([^a-z])"
 * replacement=""/&gt; &lt;tokenizer class="solr.KeywordTokenizerFactory"/&gt;
 *   &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 *
 * @since Solr 3.1
 */
class PatternReplaceCharFilterFactory : public CharFilterFactory,
                                        public MultiTermAwareComponent
{
  GET_CLASS_NAME(PatternReplaceCharFilterFactory)
private:
  const std::shared_ptr<Pattern> pattern;
  const std::wstring replacement;

  /** Creates a new PatternReplaceCharFilterFactory */
public:
  PatternReplaceCharFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<CharFilter> create(std::shared_ptr<Reader> input) override;

  std::shared_ptr<AbstractAnalysisFactory> getMultiTermComponent() override;

protected:
  std::shared_ptr<PatternReplaceCharFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<PatternReplaceCharFilterFactory>(
        org.apache.lucene.analysis.util.CharFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::pattern