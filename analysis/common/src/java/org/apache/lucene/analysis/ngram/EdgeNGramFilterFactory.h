#pragma once
#include "../util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/TokenFilter.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
namespace org::apache::lucene::analysis::ngram
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Creates new instances of {@link EdgeNGramTokenFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_edgngrm" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory"/&gt; &lt;filter
 * class="solr.EdgeNGramFilterFactory" minGramSize="1" maxGramSize="2"
 * preserveOriginal="true"/&gt; &lt;/analyzer&gt; &lt;/fieldType&gt;</pre>
 */
class EdgeNGramFilterFactory : public TokenFilterFactory
{
  GET_CLASS_NAME(EdgeNGramFilterFactory)
private:
  const int maxGramSize;
  const int minGramSize;
  const bool preserveOriginal;

  /** Creates a new EdgeNGramFilterFactory */
public:
  EdgeNGramFilterFactory(std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<TokenFilter>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<EdgeNGramFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<EdgeNGramFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/ngram/
