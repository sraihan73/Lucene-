#pragma once
#include "../util/TokenizerFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::automaton
{
class Automaton;
}

namespace org::apache::lucene::analysis::pattern
{
class SimplePatternTokenizer;
}
namespace org::apache::lucene::util
{
class AttributeFactory;
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

namespace org::apache::lucene::analysis::pattern
{

using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using Automaton = org::apache::lucene::util::automaton::Automaton;

/**
 * Factory for {@link SimplePatternTokenizer}, for matching tokens based on the
 * provided regexp.
 *
 * <p>This tokenizer uses Lucene {@link RegExp} pattern matching to construct
 * distinct tokens for the input stream.  The syntax is more limited than {@link
 * PatternTokenizer}, but the tokenization is quite a bit faster.  It takes two
 * arguments: <br> <ul> <li>"pattern" (required) is the regular expression,
 * according to the syntax described at {@link RegExp}</li>
 * <li>"maxDeterminizedStates" (optional, default 10000) the limit on total
 * state count for the determined automaton computed from the regexp</li>
 * </ul>
 * <p>
 * The pattern matches the characters to include in a token (not the split
 * characters), and the matching is greedy such that the longest token matching
 * at a given point is created.  Empty tokens are never created.
 *
 * <p>For example, to match tokens delimited by simple whitespace characters:
 *
 * <pre class="prettyprint">
 * &lt;fieldType name="text_ptn" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.SimplePatternTokenizerFactory" pattern="[^ \t\r\n]+"/&gt;
 *   &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 *
 * @lucene.experimental
 *
 * @see SimplePatternTokenizer
 */
class SimplePatternTokenizerFactory : public TokenizerFactory
{
  GET_CLASS_NAME(SimplePatternTokenizerFactory)
public:
  static const std::wstring PATTERN;

private:
  const std::shared_ptr<Automaton> dfa;
  const int maxDeterminizedStates;

  /** Creates a new SimplePatternTokenizerFactory */
public:
  SimplePatternTokenizerFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<SimplePatternTokenizer>
  create(std::shared_ptr<AttributeFactory> factory) override;

protected:
  std::shared_ptr<SimplePatternTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<SimplePatternTokenizerFactory>(
        org.apache.lucene.analysis.util.TokenizerFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::pattern
