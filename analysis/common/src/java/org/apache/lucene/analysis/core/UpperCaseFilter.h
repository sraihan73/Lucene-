#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
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
namespace org::apache::lucene::analysis::core
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * Normalizes token text to UPPER CASE.
 *
 * <p><b>NOTE:</b> In Unicode, this transformation may lose information when the
 * upper case character represents more than one lower case character. Use this
 * filter when you require uppercase tokens.  Use the {@link LowerCaseFilter}
 * for general search matching
 */
class UpperCaseFilter final : public TokenFilter
{
  GET_CLASS_NAME(UpperCaseFilter)
private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);

  /**
   * Create a new UpperCaseFilter, that normalizes token text to upper case.
   *
   * @param in TokenStream to filter
   */
public:
  UpperCaseFilter(std::shared_ptr<TokenStream> in_);

  bool incrementToken()  override final;

protected:
  std::shared_ptr<UpperCaseFilter> shared_from_this()
  {
    return std::static_pointer_cast<UpperCaseFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::core
