#pragma once
#include "../util/ResourceLoaderAware.h"
#include "../util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/payloads/PayloadEncoder.h"

#include  "core/src/java/org/apache/lucene/analysis/payloads/DelimitedPayloadTokenFilter.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
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
namespace org::apache::lucene::analysis::payloads
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link DelimitedPayloadTokenFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_dlmtd" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory"/&gt; &lt;filter
 * class="solr.DelimitedPayloadTokenFilterFactory" encoder="float"
 * delimiter="|"/&gt; &lt;/analyzer&gt; &lt;/fieldType&gt;</pre>
 */
class DelimitedPayloadTokenFilterFactory : public TokenFilterFactory,
                                           public ResourceLoaderAware
{
  GET_CLASS_NAME(DelimitedPayloadTokenFilterFactory)
public:
  static const std::wstring ENCODER_ATTR;
  static const std::wstring DELIMITER_ATTR;

private:
  const std::wstring encoderClass;
  const wchar_t delimiter;

  std::shared_ptr<PayloadEncoder> encoder;

  /** Creates a new DelimitedPayloadTokenFilterFactory */
public:
  DelimitedPayloadTokenFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<DelimitedPayloadTokenFilter>
  create(std::shared_ptr<TokenStream> input) override;

  void inform(std::shared_ptr<ResourceLoader> loader) override;

protected:
  std::shared_ptr<DelimitedPayloadTokenFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<DelimitedPayloadTokenFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/payloads/
