#pragma once
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/MultiTermAwareComponent.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class TokenStream;
}

namespace org::apache::lucene::analysis::util
{
class AbstractAnalysisFactory;
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
namespace org::apache::lucene::analysis::icu
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory; // javadocs
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

using com::ibm::icu::text::Normalizer2;

/**
 * Factory for {@link ICUNormalizer2Filter}
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
 * @see ICUNormalizer2Filter
 * @see Normalizer2
 * @see FilteredNormalizer2
 * @since 3.1.0
 */
class ICUNormalizer2FilterFactory : public TokenFilterFactory,
                                    public MultiTermAwareComponent
{
  GET_CLASS_NAME(ICUNormalizer2FilterFactory)
private:
  const std::shared_ptr<Normalizer2> normalizer;

  /** Creates a new ICUNormalizer2FilterFactory */
public:
  ICUNormalizer2FilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  // TODO: support custom normalization

  std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> input) override;

  std::shared_ptr<AbstractAnalysisFactory> getMultiTermComponent() override;

protected:
  std::shared_ptr<ICUNormalizer2FilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<ICUNormalizer2FilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::icu
