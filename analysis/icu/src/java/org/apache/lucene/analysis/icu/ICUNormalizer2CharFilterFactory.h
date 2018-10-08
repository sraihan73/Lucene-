#pragma once
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/CharFilterFactory.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/MultiTermAwareComponent.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::analysis::icu
{

using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;

using com::ibm::icu::text::Normalizer2;

/**
 * Factory for {@link ICUNormalizer2CharFilter}
 * <p>
 * Supports the following attributes:
 * <ul>
 *   <li>name: A <a href="http://unicode.org/reports/tr15/">Unicode
 * Normalization Form</a>, one of 'nfc','nfkc', 'nfkc_cf'. Default is nfkc_cf.
 *   <li>mode: Either 'compose' or 'decompose'. Default is compose. Use
 * "decompose" with nfc or nfkc, to get nfd or nfkd, respectively. <li>filter: A
 * {@link UnicodeSet} pattern. Codepoints outside the set are always left
 * unchanged. Default is [] (the null set, no filtering).
 * </ul>
 * @see ICUNormalizer2CharFilter
 * @see Normalizer2
 * @see FilteredNormalizer2
 */
class ICUNormalizer2CharFilterFactory : public CharFilterFactory,
                                        public MultiTermAwareComponent
{
  GET_CLASS_NAME(ICUNormalizer2CharFilterFactory)
private:
  const std::shared_ptr<Normalizer2> normalizer;

  /** Creates a new ICUNormalizer2CharFilterFactory */
public:
  ICUNormalizer2CharFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<Reader> create(std::shared_ptr<Reader> input) override;

  std::shared_ptr<AbstractAnalysisFactory> getMultiTermComponent() override;

protected:
  std::shared_ptr<ICUNormalizer2CharFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<ICUNormalizer2CharFilterFactory>(
        org.apache.lucene.analysis.util.CharFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/icu/
