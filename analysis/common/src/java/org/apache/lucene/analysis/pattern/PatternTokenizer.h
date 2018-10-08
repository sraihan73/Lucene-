#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
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

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * This tokenizer uses regex pattern matching to construct distinct tokens
 * for the input stream.  It takes two arguments:  "pattern" and "group".
 * <ul>
 * <li>"pattern" is the regular expression.</li>
 * <li>"group" says which group to extract into tokens.</li>
 *  </ul>
 * <p>
 * group=-1 (the default) is equivalent to "split".  In this case, the tokens
 *will be equivalent to the output from (without empty tokens):
 * {@link std::wstring#split(java.lang.std::wstring)}
 * </p>
 * <p>
 * Using group &gt;= 0 selects the matching group as the token.  For example, if
 *you have:<br> <pre> pattern = \'([^\']+)\' group = 0 input = aaa 'bbb' 'ccc'
 *</pre>
 * the output will be two tokens: 'bbb' and 'ccc' (including the ' marks).  With
 *the same input but using group=1, the output would be: bbb and ccc (no '
 *marks) <p>NOTE: This Tokenizer does not output tokens that are of zero
 *length.</p>
 *
 * @see Pattern
 */
class PatternTokenizer final : public Tokenizer
{
  GET_CLASS_NAME(PatternTokenizer)

private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);

  const std::shared_ptr<StringBuilder> str = std::make_shared<StringBuilder>();
  int index = 0;

  const int group;
  const std::shared_ptr<Matcher> matcher;

  /** creates a new PatternTokenizer returning tokens from group (-1 for split
   * functionality) */
public:
  PatternTokenizer(std::shared_ptr<Pattern> pattern, int group);

  /** creates a new PatternTokenizer returning tokens from group (-1 for split
   * functionality) */
  PatternTokenizer(std::shared_ptr<AttributeFactory> factory,
                   std::shared_ptr<Pattern> pattern, int group);

  bool incrementToken() override;

  void end()  override;

  virtual ~PatternTokenizer();

  void reset()  override;

  // TODO: we should see if we can make this tokenizer work without reading
  // the entire document into RAM, perhaps with Matcher.hitEnd/requireEnd ?
  std::deque<wchar_t> const buffer = std::deque<wchar_t>(8192);

private:
  void fillBuffer(std::shared_ptr<Reader> input) ;

protected:
  std::shared_ptr<PatternTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<PatternTokenizer>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/pattern/
