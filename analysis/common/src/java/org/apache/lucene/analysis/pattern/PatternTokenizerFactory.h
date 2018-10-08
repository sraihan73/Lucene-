#pragma once
#include "../util/TokenizerFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/pattern/PatternTokenizer.h"

#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"

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

/**
 * Factory for {@link PatternTokenizer}.
 * This tokenizer uses regex pattern matching to construct distinct tokens
 * for the input stream.  It takes two arguments:  "pattern" and "group".
 * <br>
 * <ul>
 * <li>"pattern" is the regular expression.</li>
 * <li>"group" says which group to extract into tokens.</li>
 *  </ul>
 * <p>
 * group=-1 (the default) is equivalent to "split".  In this case, the tokens
 * will be equivalent to the output from (without empty tokens):
 * {@link std::wstring#split(java.lang.std::wstring)}
 * </p>
 * <p>
 * Using group &gt;= 0 selects the matching group as the token.  For example, if
 * you have:<br> <pre> pattern = \'([^\']+)\' group = 0 input = aaa 'bbb' 'ccc'
 * </pre>
 * the output will be two tokens: 'bbb' and 'ccc' (including the ' marks).  With
 * the same input but using group=1, the output would be: bbb and ccc (no '
 * marks) <p>NOTE: This Tokenizer does not output tokens that are of zero
 * length.</p>
 *
 * <pre class="prettyprint">
 * &lt;fieldType name="text_ptn" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.PatternTokenizerFactory" pattern="\'([^\']+)\'" group="1"/&gt;
 *   &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 *
 * @see PatternTokenizer
 * @since solr1.2
 */
class PatternTokenizerFactory : public TokenizerFactory
{
  GET_CLASS_NAME(PatternTokenizerFactory)
public:
  static const std::wstring PATTERN;
  static const std::wstring GROUP;

protected:
  const std::shared_ptr<Pattern> pattern;
  const int group;

  /** Creates a new PatternTokenizerFactory */
public:
  PatternTokenizerFactory(std::unordered_map<std::wstring, std::wstring> &args);

  /**
   * Split the input using configured pattern
   */
  std::shared_ptr<PatternTokenizer>
  create(std::shared_ptr<AttributeFactory> factory) override;

protected:
  std::shared_ptr<PatternTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<PatternTokenizerFactory>(
        org.apache.lucene.analysis.util.TokenizerFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/pattern/
