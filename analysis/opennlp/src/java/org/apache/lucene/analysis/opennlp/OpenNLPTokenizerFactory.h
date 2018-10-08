#pragma once
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoaderAware.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/opennlp/OpenNLPTokenizer.h"

#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include  "core/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"

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

using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * Factory for {@link OpenNLPTokenizer}.
 *
 * <pre class="prettyprint">
 * &lt;fieldType name="text_opennlp" class="solr.TextField"
 * positionIncrementGap="100" &lt;analyzer&gt; &lt;tokenizer
 * class="solr.OpenNLPTokenizerFactory" sentenceModel="filename"
 * tokenizerModel="filename"/&gt; &lt;/analyzer&gt; &lt;/fieldType&gt;</pre>
 * @since 7.3.0
 */
class OpenNLPTokenizerFactory : public TokenizerFactory,
                                public ResourceLoaderAware
{
  GET_CLASS_NAME(OpenNLPTokenizerFactory)
public:
  static const std::wstring SENTENCE_MODEL;
  static const std::wstring TOKENIZER_MODEL;

private:
  const std::wstring sentenceModelFile;
  const std::wstring tokenizerModelFile;

public:
  OpenNLPTokenizerFactory(std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<OpenNLPTokenizer>
  create(std::shared_ptr<AttributeFactory> factory) override;

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

protected:
  std::shared_ptr<OpenNLPTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<OpenNLPTokenizerFactory>(
        org.apache.lucene.analysis.util.TokenizerFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/opennlp/
