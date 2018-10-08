#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class TokenStream;
}

namespace org::apache::lucene::search::highlight
{
class TextFragment;
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
namespace org::apache::lucene::search::highlight
{

using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * A Scorer is responsible for scoring a stream of tokens. These token scores
 * can then be used to compute {@link TextFragment} scores.
 */
class Scorer
{
  GET_CLASS_NAME(Scorer)

  /**
   * Called to init the Scorer with a {@link TokenStream}. You can grab
   * references to the attributes you are interested in here and access them
   * from {@link #getTokenScore()}.
   *
   * @param tokenStream the {@link TokenStream} that will be scored.
   * @return either a {@link TokenStream} that the Highlighter should continue
   * using (eg if you read the tokenSream in this method) or null to continue
   *         using the same {@link TokenStream} that was passed in.
   * @throws IOException If there is a low-level I/O error
   */
public:
  virtual std::shared_ptr<TokenStream>
  init(std::shared_ptr<TokenStream> tokenStream) = 0;

  /**
   * Called when a new fragment is started for consideration.
   *
   * @param newFragment the fragment that will be scored next
   */
  virtual void startFragment(std::shared_ptr<TextFragment> newFragment) = 0;

  /**
   * Called for each token in the current fragment. The {@link Highlighter} will
   * increment the {@link TokenStream} passed to init on every call.
   *
   * @return a score which is passed to the {@link Highlighter} class to
   * influence the mark-up of the text (this return value is NOT used to score
   * the fragment)
   */
  virtual float getTokenScore() = 0;

  /**
   * Called when the {@link Highlighter} has no more tokens for the current
   * fragment - the Scorer returns the weighting it has derived for the most
   * recent fragment, typically based on the results of {@link
   * #getTokenScore()}.
   *
   */
  virtual float getFragmentScore() = 0;
};

} // namespace org::apache::lucene::search::highlight
