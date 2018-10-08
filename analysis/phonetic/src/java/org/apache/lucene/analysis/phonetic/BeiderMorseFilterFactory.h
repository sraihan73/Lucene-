#pragma once
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::analysis::phonetic
{

using org::apache::commons::codec::language::bm::PhoneticEngine;
using org::apache::commons::codec::language::bm::Languages::LanguageSet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link BeiderMorseFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_bm" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.StandardTokenizerFactory"/&gt; &lt;filter
 * class="solr.BeiderMorseFilterFactory" nameType="GENERIC" ruleType="APPROX"
 *        concat="true" languageSet="auto"
 *     &lt;/filter&gt;
 *   &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 * @since 3.6.0
 */
class BeiderMorseFilterFactory : public TokenFilterFactory
{
  GET_CLASS_NAME(BeiderMorseFilterFactory)
private:
  const std::shared_ptr<PhoneticEngine> engine;
  const std::shared_ptr<LanguageSet> languageSet;

  /** Creates a new BeiderMorseFilterFactory */
public:
  BeiderMorseFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<BeiderMorseFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<BeiderMorseFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/phonetic/
