#pragma once
#include "TokenStream.h"
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
namespace org::apache::lucene::analysis
{

/** A TokenFilter is a TokenStream whose input is another TokenStream.
  <p>
  This is an abstract class; subclasses must override {@link #incrementToken()}.
  @see TokenStream
  */
class TokenFilter : public TokenStream
{
  GET_CLASS_NAME(TokenFilter)
  /** The source of tokens for this filter. */
protected:
  const std::shared_ptr<TokenStream> input;

  /** Construct a token stream filtering the given input. */
  TokenFilter(std::shared_ptr<TokenStream> input);

  /**
   * {@inheritDoc}
   * <p>
   * <b>NOTE:</b>
   * The default implementation chains the call to the input TokenStream, so
   * be sure to call <code>super.end()</code> first when overriding this method.
   */
public:
  void end()  override;

  /**
   * {@inheritDoc}
   * <p>
   * <b>NOTE:</b>
   * The default implementation chains the call to the input TokenStream, so
   * be sure to call <code>super.close()</code> when overriding this method.
   */
  virtual ~TokenFilter();

  /**
   * {@inheritDoc}
   * <p>
   * <b>NOTE:</b>
   * The default implementation chains the call to the input TokenStream, so
   * be sure to call <code>super.reset()</code> when overriding this method.
   */
  void reset()  override;

protected:
  std::shared_ptr<TokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<TokenFilter>(
        TokenStream::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
