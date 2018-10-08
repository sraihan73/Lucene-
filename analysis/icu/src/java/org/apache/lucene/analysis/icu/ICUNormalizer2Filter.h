#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
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
namespace org::apache::lucene::analysis::icu
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

using com::ibm::icu::text::Normalizer2;

/**
 * Normalize token text with ICU's {@link com.ibm.icu.text.Normalizer2}
 * <p>
 * With this filter, you can normalize text in the following ways:
 * <ul>
 *  <li> NFKC Normalization, Case Folding, and removing Ignorables (the default)
 *  <li> Using a standard Normalization mode (NFC, NFD, NFKC, NFKD)
 *  <li> Based on rules from a custom normalization mapping.
 * </ul>
 * <p>
 * If you use the defaults, this filter is a simple way to standardize Unicode
 * text in a language-independent way for search: <ul> <li> The case folding
 * that it does can be seen as a replacement for LowerCaseFilter: For example,
 * it handles cases such as the Greek sigma, so that "Μάϊος" and "ΜΆΪΟΣ" will
 * match correctly. <li> The normalization will standardizes different forms of
 * the same character in Unicode. For example, CJK full-width numbers will be
 * standardized to their ASCII forms. <li> Ignorables such as Zero-Width Joiner
 * and Variation Selectors are removed. These are typically modifier characters
 * that affect display.
 * </ul>
 *
 * @see com.ibm.icu.text.Normalizer2
 * @see com.ibm.icu.text.FilteredNormalizer2
 */
class ICUNormalizer2Filter : public TokenFilter
{
  GET_CLASS_NAME(ICUNormalizer2Filter)
private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<Normalizer2> normalizer;
  const std::shared_ptr<StringBuilder> buffer =
      std::make_shared<StringBuilder>();

  /**
   * Create a new Normalizer2Filter that combines NFKC normalization, Case
   * Folding, and removes Default Ignorables (NFKC_Casefold)
   */
public:
  ICUNormalizer2Filter(std::shared_ptr<TokenStream> input);

  /**
   * Create a new Normalizer2Filter with the specified Normalizer2
   * @param input stream
   * @param normalizer normalizer to use
   */
  ICUNormalizer2Filter(std::shared_ptr<TokenStream> input,
                       std::shared_ptr<Normalizer2> normalizer);

  bool incrementToken()  override final;

protected:
  std::shared_ptr<ICUNormalizer2Filter> shared_from_this()
  {
    return std::static_pointer_cast<ICUNormalizer2Filter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::icu
