#pragma once
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/CharFilterFactory.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/MultiTermAwareComponent.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharFilter.h"

#include  "core/src/java/org/apache/lucene/analysis/util/AbstractAnalysisFactory.h"

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
namespace org::apache::lucene::analysis::ja
{

using CharFilter = org::apache::lucene::analysis::CharFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;

/**
 * Factory for {@link
 * org.apache.lucene.analysis.ja.JapaneseIterationMarkCharFilter}. <pre
 * class="prettyprint"> &lt;fieldType name="text_ja" class="solr.TextField"
 * positionIncrementGap="100" autoGeneratePhraseQueries="false"&gt;
 *   &lt;analyzer&gt;
 *     &lt;charFilter class="solr.JapaneseIterationMarkCharFilterFactory
 * normalizeKanji="true" normalizeKana="true"/&gt; &lt;tokenizer
 * class="solr.JapaneseTokenizerFactory"/&gt; &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 */
class JapaneseIterationMarkCharFilterFactory : public CharFilterFactory,
                                               public MultiTermAwareComponent
{
  GET_CLASS_NAME(JapaneseIterationMarkCharFilterFactory)

private:
  static const std::wstring NORMALIZE_KANJI_PARAM;
  static const std::wstring NORMALIZE_KANA_PARAM;

  const bool normalizeKanji;
  const bool normalizeKana;

  /** Creates a new JapaneseIterationMarkCharFilterFactory */
public:
  JapaneseIterationMarkCharFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<CharFilter> create(std::shared_ptr<Reader> input) override;

  std::shared_ptr<AbstractAnalysisFactory> getMultiTermComponent() override;

protected:
  std::shared_ptr<JapaneseIterationMarkCharFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<JapaneseIterationMarkCharFilterFactory>(
        org.apache.lucene.analysis.util.CharFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/ja/
