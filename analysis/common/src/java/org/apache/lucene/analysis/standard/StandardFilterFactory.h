#pragma once
#include "../util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

#include  "core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"

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
namespace org::apache::lucene::analysis::standard
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link StandardFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_stndrd" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.StandardTokenizerFactory"/&gt; &lt;filter
 * class="solr.StandardFilterFactory"/&gt; &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 */
class StandardFilterFactory : public TokenFilterFactory
{
  GET_CLASS_NAME(StandardFilterFactory)

  /** Creates a new StandardFilterFactory */
public:
  StandardFilterFactory(std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<StandardFilter>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<StandardFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<StandardFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/standard/
