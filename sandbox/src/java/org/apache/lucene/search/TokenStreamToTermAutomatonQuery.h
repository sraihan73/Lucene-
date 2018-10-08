#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

#include  "core/src/java/org/apache/lucene/search/TermAutomatonQuery.h"

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
namespace org::apache::lucene::search
{

using TokenStream = org::apache::lucene::analysis::TokenStream;

/** Consumes a TokenStream and creates an {@link TermAutomatonQuery}
 *  where the transition labels are tokens from the {@link
 *  TermToBytesRefAttribute}.
 *
 *  <p>This code is very new and likely has exciting bugs!
 *
 *  @lucene.experimental */
class TokenStreamToTermAutomatonQuery
    : public std::enable_shared_from_this<TokenStreamToTermAutomatonQuery>
{
  GET_CLASS_NAME(TokenStreamToTermAutomatonQuery)

private:
  bool preservePositionIncrements = false;

  /** Sole constructor. */
public:
  TokenStreamToTermAutomatonQuery();

  /** Whether to generate holes in the automaton for missing positions,
   * <code>true</code> by default. */
  virtual void setPreservePositionIncrements(bool enablePositionIncrements);

  /** Pulls the graph (including {@link
   *  PositionLengthAttribute}) from the provided {@link
   *  TokenStream}, and creates the corresponding
   *  automaton where arcs are bytes (or Unicode code points
   *  if unicodeArcs = true) from each term. */
  virtual std::shared_ptr<TermAutomatonQuery>
  toQuery(const std::wstring &field,
          std::shared_ptr<TokenStream> in_) ;
};

} // #include  "core/src/java/org/apache/lucene/search/
