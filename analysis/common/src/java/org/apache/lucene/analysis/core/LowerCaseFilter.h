#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class LowerCaseFilter;
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

using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * Normalizes token text to lower case.
 * <p>
 * This class moved to Lucene Core, but a reference in the {@code
 * analysis/common} module is preserved for documentation purposes and
 * consistency with filter factory.
 * @see org.apache.lucene.analysis.LowerCaseFilter
 * @see LowerCaseFilterFactory
 */
class LowerCaseFilter final
    : public org::apache::lucene::analysis::LowerCaseFilter
{
  GET_CLASS_NAME(LowerCaseFilter)

  /**
   * Create a new LowerCaseFilter, that normalizes token text to lower case.
   *
   * @param in TokenStream to filter
   */
public:
  LowerCaseFilter(std::shared_ptr<TokenStream> in_);

protected:
  std::shared_ptr<LowerCaseFilter> shared_from_this()
  {
    return std::static_pointer_cast<LowerCaseFilter>(
        org.apache.lucene.analysis.LowerCaseFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::core