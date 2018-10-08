#pragma once
#include "../util/TokenizerFactory.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Tokenizer;
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
namespace org::apache::lucene::analysis::core
{

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.analysis.standard.StandardTokenizer.MAX_TOKEN_LENGTH_LIMIT;

/**
 * Factory for {@link WhitespaceTokenizer}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_ws" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory" rule="unicode" maxTokenLen="256"/&gt;
 *   &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 *
 * Options:
 * <ul>
 *   <li>rule: either "java" for {@link WhitespaceTokenizer}
 *      or "unicode" for {@link UnicodeWhitespaceTokenizer}</li>
 *   <li>maxTokenLen: max token length, should be greater than 0 and less than
 * MAX_TOKEN_LENGTH_LIMIT (1024*1024). It is rare to need to change this else
 * {@link CharTokenizer}::DEFAULT_MAX_TOKEN_LEN</li>
 * </ul>
 */
class WhitespaceTokenizerFactory : public TokenizerFactory
{
  GET_CLASS_NAME(WhitespaceTokenizerFactory)
public:
  static const std::wstring RULE_JAVA;
  static const std::wstring RULE_UNICODE;

private:
  static const std::shared_ptr<std::deque<std::wstring>> RULE_NAMES;

  const std::wstring rule;
  const int maxTokenLen;

  /** Creates a new WhitespaceTokenizerFactory */
public:
  WhitespaceTokenizerFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<Tokenizer>
  create(std::shared_ptr<AttributeFactory> factory) override;

protected:
  std::shared_ptr<WhitespaceTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<WhitespaceTokenizerFactory>(
        org.apache.lucene.analysis.util.TokenizerFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::core
