#pragma once
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoaderAware.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"

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
namespace org::apache::lucene::analysis::morfologik
{

using morfologik::stemming::Dictionary;

using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Filter factory for {@link MorfologikFilter}.
 *
 * <p>An explicit resource name of the dictionary ({@code ".dict"}) can be
 * provided via the <code>dictionary</code> attribute, as the example below
 * demonstrates: <pre class="prettyprint"> &lt;fieldType name="text_mylang"
 * class="solr.TextField" positionIncrementGap="100"&gt; &lt;analyzer&gt;
 *     &lt;tokenizer class="solr.WhitespaceTokenizerFactory"/&gt;
 *     &lt;filter class="solr.MorfologikFilterFactory" dictionary="mylang.dict"
 * /&gt; &lt;/analyzer&gt; &lt;/fieldType&gt;</pre>
 *
 * <p>If the dictionary attribute is not provided, the Polish dictionary is
 * loaded and used by default.
 *
 * @see <a href="http://morfologik.blogspot.com/">Morfologik web site</a>
 * @since 4.0.0
 */
class MorfologikFilterFactory : public TokenFilterFactory,
                                public ResourceLoaderAware
{
  GET_CLASS_NAME(MorfologikFilterFactory)
  /** Dictionary resource attribute (should have {@code ".dict"} suffix), loaded
   * from {@link ResourceLoader}. */
public:
  static const std::wstring DICTIONARY_ATTRIBUTE;

  /** {@link #DICTIONARY_ATTRIBUTE} value passed to {@link #inform}. */
private:
  std::wstring resourceName;

  /** Loaded {@link Dictionary}, initialized on {@link #inform(ResourceLoader)}.
   */
  std::shared_ptr<Dictionary> dictionary;

  /** Creates a new MorfologikFilterFactory */
public:
  MorfologikFilterFactory(std::unordered_map<std::wstring, std::wstring> &args);

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

  std::shared_ptr<TokenStream> create(std::shared_ptr<TokenStream> ts) override;

protected:
  std::shared_ptr<MorfologikFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<MorfologikFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/morfologik/
