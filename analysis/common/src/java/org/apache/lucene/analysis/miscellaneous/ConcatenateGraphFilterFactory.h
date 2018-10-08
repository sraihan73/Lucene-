#pragma once
#include "../util/TokenFilterFactory.h"
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
namespace org::apache::lucene::analysis::miscellaneous
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link ConcatenateGraphFilter}.
 *
 * <ul>
 *   <li><tt>preserveSep</tt>:
 *                            Whether {@link ConcatenateGraphFilter#SEP_LABEL}
 *                            should separate the input tokens in the
 * concatenated token
 *                            </li>
 *   <li><tt>preservePositionIncrements</tt>:
 *                                       Whether to add an empty token for
 * missing positions. The effect is a consecutive {@link
 * ConcatenateGraphFilter#SEP_LABEL}. When false, it's as if there were no
 * missing positions (we pretend the surrounding tokens were adjacent).
 *                                       </li>
 *   <li><tt>maxGraphExpansions</tt>:
 *                            If the tokenStream graph has more than this many
 * possible paths through, then we'll throw
 *                            {@link TooComplexToDeterminizeException} to
 * preserve the stability and memory of the machine.
 *                            </li>
 * </ul>
 * @see ConcatenateGraphFilter
 * @since 7.4.0
 */
class ConcatenateGraphFilterFactory : public TokenFilterFactory
{
  GET_CLASS_NAME(ConcatenateGraphFilterFactory)

private:
  bool preserveSep = false;
  bool preservePositionIncrements = false;
  int maxGraphExpansions = 0;

public:
  ConcatenateGraphFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<ConcatenateGraphFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<ConcatenateGraphFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
