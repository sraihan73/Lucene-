#pragma once
#include "../util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
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

namespace org::apache::lucene::analysis::shingle
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link FixedShingleFilter}
 *
 * Parameters are:
 * <ul>
 *   <li>shingleSize - how many tokens should be combined into each shingle
 * (default: 2) <li>tokenSeparator - how tokens should be joined together in the
 * shingle (default: space) <li>fillerToken - what should be added in place of
 * stop words (default: _ )
 * </ul>
 */
class FixedShingleFilterFactory : public TokenFilterFactory
{
  GET_CLASS_NAME(FixedShingleFilterFactory)

private:
  const int shingleSize;
  const std::wstring tokenSeparator;
  const std::wstring fillerToken;

public:
  FixedShingleFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<FixedShingleFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<FixedShingleFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::shingle
