#pragma once
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::phonetic
{
class DaitchMokotoffSoundexFilter;
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
namespace org::apache::lucene::analysis::phonetic
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link DaitchMokotoffSoundexFilter}.
 *
 * Create tokens based on Daitch–Mokotoff Soundex phonetic filter.
 * <p>
 * This takes one optional argument:
 * <dl>
 *  <dt>inject</dt><dd> (default=true) add tokens to the stream with the
 * offset=0</dd>
 * </dl>
 *
 * <pre class="prettyprint">
 * &lt;fieldType name="text_phonetic" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory"/&gt; &lt;filter
 * class="solr.DaitchMokotoffSoundexFilterFactory" inject="true"/&gt;
 *   &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 *
 * @see DaitchMokotoffSoundexFilter
 *
 * @lucene.experimental
 * @since 5.0.0
 */
class DaitchMokotoffSoundexFilterFactory : public TokenFilterFactory
{
  GET_CLASS_NAME(DaitchMokotoffSoundexFilterFactory)
  /** parameter name: true if encoded tokens should be added as synonyms */
public:
  static const std::wstring INJECT; // bool

  const bool inject; // accessed by the test

  /** Creates a new PhoneticFilterFactory */
  DaitchMokotoffSoundexFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<DaitchMokotoffSoundexFilter>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<DaitchMokotoffSoundexFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<DaitchMokotoffSoundexFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::phonetic