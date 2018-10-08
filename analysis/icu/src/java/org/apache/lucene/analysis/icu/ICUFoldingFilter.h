#pragma once
#include "ICUNormalizer2Filter.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
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

using TokenStream = org::apache::lucene::analysis::TokenStream;

using com::ibm::icu::text::Normalizer2;

/**
 * A TokenFilter that applies search term folding to Unicode text,
 * applying foldings from UTR#30 Character Foldings.
 * <p>
 * This filter applies the following foldings from the report to unicode text:
 * <ul>
 * <li>Accent removal
 * <li>Case folding
 * <li>Canonical duplicates folding
 * <li>Dashes folding
 * <li>Diacritic removal (including stroke, hook, descender)
 * <li>Greek letterforms folding
 * <li>Han Radical folding
 * <li>Hebrew Alternates folding
 * <li>Jamo folding
 * <li>Letterforms folding
 * <li>Math symbol folding
 * <li>Multigraph Expansions: All
 * <li>Native digit folding
 * <li>No-break folding
 * <li>Overline folding
 * <li>Positional forms folding
 * <li>Small forms folding
 * <li>Space folding
 * <li>Spacing Accents folding
 * <li>Subscript folding
 * <li>Superscript folding
 * <li>Suzhou Numeral folding
 * <li>Symbol folding
 * <li>Underline folding
 * <li>Vertical forms folding
 * <li>Width folding
 * </ul>
 * <p>
 * Additionally, Default Ignorables are removed, and text is normalized to NFKC.
 * All foldings, case folding, and normalization mappings are applied
 * recursively to ensure a fully folded and normalized result.
 * </p>
 * <p>
 * A normalizer with additional settings such as a filter that lists characters
 * not to be normalized can be passed in the constructor.
 * </p>
 */
class ICUFoldingFilter final : public ICUNormalizer2Filter
{
  GET_CLASS_NAME(ICUFoldingFilter)
  /**
   * A normalizer for search term folding to Unicode text,
   * applying foldings from UTR#30 Character Foldings.
   */
public:
  static const std::shared_ptr<Normalizer2> NORMALIZER;

  /**
   * Create a new ICUFoldingFilter on the specified input
   */
  ICUFoldingFilter(std::shared_ptr<TokenStream> input);

  /**
   * Create a new ICUFoldingFilter on the specified input with the specified
   * normalizer
   */
  ICUFoldingFilter(std::shared_ptr<TokenStream> input,
                   std::shared_ptr<Normalizer2> normalizer);

protected:
  std::shared_ptr<ICUFoldingFilter> shared_from_this()
  {
    return std::static_pointer_cast<ICUFoldingFilter>(
        ICUNormalizer2Filter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::icu
