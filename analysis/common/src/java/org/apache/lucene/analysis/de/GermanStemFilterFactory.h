#pragma once
#include "../util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::de
{
class GermanStemFilter;
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
namespace org::apache::lucene::analysis::de
{

using GermanStemFilter = org::apache::lucene::analysis::de::GermanStemFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link GermanStemFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_destem" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.StandardTokenizerFactory"/&gt; &lt;filter
 * class="solr.LowerCaseFilterFactory"/&gt; &lt;filter
 * class="solr.GermanStemFilterFactory"/&gt; &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 */
class GermanStemFilterFactory : public TokenFilterFactory
{
  GET_CLASS_NAME(GermanStemFilterFactory)

  /** Creates a new GermanStemFilterFactory */
public:
  GermanStemFilterFactory(std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<GermanStemFilter>
  create(std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<GermanStemFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<GermanStemFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::de