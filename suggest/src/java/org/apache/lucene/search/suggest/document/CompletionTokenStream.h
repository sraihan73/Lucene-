#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class PayloadAttribute;
}

namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util::automaton
{
class Automaton;
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

namespace org::apache::lucene::search::suggest::document
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;

/**
 * A {@link ConcatenateGraphFilter} but we can set the payload and provide
 * access to config options.
 * @lucene.experimental
 */
class CompletionTokenStream final : public TokenFilter
{
  GET_CLASS_NAME(CompletionTokenStream)

private:
  const std::shared_ptr<PayloadAttribute> payloadAttr =
      addAttribute(PayloadAttribute::typeid);

  // package accessible on purpose
public:
  const std::shared_ptr<TokenStream> inputTokenStream;
  const bool preserveSep;
  const bool preservePositionIncrements;
  const int maxGraphExpansions;

private:
  std::shared_ptr<BytesRef> payload; // note doesn't participate in TokenStream
                                     // lifecycle; it's effectively constant

public:
  CompletionTokenStream(std::shared_ptr<TokenStream> inputTokenStream);

  CompletionTokenStream(std::shared_ptr<TokenStream> inputTokenStream,
                        bool preserveSep, bool preservePositionIncrements,
                        int maxGraphExpansions);

  /**
   * Sets a payload available throughout successive token stream enumeration
   */
  void setPayload(std::shared_ptr<BytesRef> payload);

  bool incrementToken()  override;

  /** Delegates to...At
   * @see ConcatenateGraphFilter#toAutomaton()  */
  std::shared_ptr<Automaton> toAutomaton() ;

  /** Delegates to...
   *  @see ConcatenateGraphFilter#toAutomaton(bool) */
  std::shared_ptr<Automaton> toAutomaton(bool unicodeAware) ;

protected:
  std::shared_ptr<CompletionTokenStream> shared_from_this()
  {
    return std::static_pointer_cast<CompletionTokenStream>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::document
