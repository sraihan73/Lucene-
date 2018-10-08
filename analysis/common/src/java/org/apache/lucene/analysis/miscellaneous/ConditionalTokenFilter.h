#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "stringhelper.h"
#include <functional>
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::util
{
class AttributeSource;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::util
{
class State;
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

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

/**
 * Allows skipping TokenFilters based on the current set of attributes.
 *
 * To use, implement the {@link #shouldFilter()} method.  If it returns {@code
 * true}, then calling {@link #incrementToken()} will use the wrapped
 * TokenFilter(s) to make changes to the tokenstream.  If it returns {@code
 * false}, then the wrapped filter(s) will be skipped.
 */
class ConditionalTokenFilter : public TokenFilter
{
  GET_CLASS_NAME(ConditionalTokenFilter)

private:
  enum class TokenState {
    GET_CLASS_NAME(TokenState) READING,
    PREBUFFERING,
    DELEGATING
  };

private:
  class OneTimeWrapper final : public TokenStream
  {
    GET_CLASS_NAME(OneTimeWrapper)
  private:
    std::shared_ptr<ConditionalTokenFilter> outerInstance;

    const std::shared_ptr<OffsetAttribute> offsetAtt;
    const std::shared_ptr<PositionIncrementAttribute> posIncAtt;

  public:
    OneTimeWrapper(std::shared_ptr<ConditionalTokenFilter> outerInstance,
                   std::shared_ptr<AttributeSource> attributeSource);

    bool incrementToken()  override;

    void reset()  override;

    void end()  override;

  protected:
    std::shared_ptr<OneTimeWrapper> shared_from_this()
    {
      return std::static_pointer_cast<OneTimeWrapper>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

private:
  const std::shared_ptr<TokenStream> delegate_;
  TokenState state = TokenState::READING;
  bool lastTokenFiltered = false;
  std::shared_ptr<AttributeSource::State> bufferedState = nullptr;
  bool exhausted = false;
  bool adjustPosition = false;
  std::shared_ptr<AttributeSource::State> endState = nullptr;
  int endOffset = 0;

  const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
      addAttribute(PositionIncrementAttribute::typeid);

  /**
   * Create a new ConditionalTokenFilter
   * @param input         the input TokenStream
   * @param inputFactory  a factory function to create the wrapped filter(s)
   */
protected:
  ConditionalTokenFilter(
      std::shared_ptr<TokenStream> input,
      std::function<TokenStream *(TokenStream *)> &inputFactory);

  /**
   * Whether or not to execute the wrapped TokenFilter(s) for the current token
   */
  virtual bool shouldFilter() = 0;

public:
  void reset()  override;

  void end()  override;

  virtual ~ConditionalTokenFilter();

  bool incrementToken()  override final;

private:
  bool endDelegating() ;

protected:
  std::shared_ptr<ConditionalTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<ConditionalTokenFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
