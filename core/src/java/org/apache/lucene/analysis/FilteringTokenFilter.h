#pragma once
#include "TokenFilter.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
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
namespace org::apache::lucene::analysis
{

using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

/**
 * Abstract base class for TokenFilters that may remove tokens.
 * You have to implement {@link #accept} and return a bool if the current
GET_CLASS_NAME(for)
 * token should be preserved. {@link #incrementToken} uses this method
 * to decide if a token should be passed to the caller.
 */
class FilteringTokenFilter : public TokenFilter
{
  GET_CLASS_NAME(FilteringTokenFilter)

private:
  const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  int skippedPositions = 0;

  /**
   * Create a new {@link FilteringTokenFilter}.
   * @param in      the {@link TokenStream} to consume
   */
public:
  FilteringTokenFilter(std::shared_ptr<TokenStream> in_);

  /** Override this method and return if the current input token should be
   * returned by {@link #incrementToken}. */
protected:
  virtual bool accept() = 0;

public:
  bool incrementToken()  override final;

  void reset()  override;

  void end()  override;

protected:
  std::shared_ptr<FilteringTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<FilteringTokenFilter>(
        TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
