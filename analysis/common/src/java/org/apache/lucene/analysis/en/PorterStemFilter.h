#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::en
{
class PorterStemmer;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class KeywordAttribute;
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
namespace org::apache::lucene::analysis::en
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/** Transforms the token stream as per the Porter stemming algorithm.
    Note: the input to the stemming filter must already be in lower case,
    so you will need to use LowerCaseFilter or LowerCaseTokenizer farther
    down the Tokenizer chain in order for this to work properly!
    <P>
    To use this filter with other analyzers, you'll want to write an
    Analyzer class that sets up the TokenStream chain as you want it.
    To use this with LowerCaseTokenizer, for example, you'd write an
    analyzer like this:
    <br>
    <PRE class="prettyprint">
    class MyAnalyzer extends Analyzer {
GET_CLASS_NAME(="prettyprint">)
      {@literal @Override}
      protected TokenStreamComponents createComponents(std::wstring fieldName) {
        Tokenizer source = new LowerCaseTokenizer(version, reader);
        return new TokenStreamComponents(source, new PorterStemFilter(source));
      }
    }
    </PRE>
    <p>
    Note: This filter is aware of the {@link KeywordAttribute}. To prevent
    certain terms from being passed to the stemmer
    {@link KeywordAttribute#isKeyword()} should be set to <code>true</code>
    in a previous {@link TokenStream}.

    Note: For including the original term as well as the stemmed version, see
   {@link org.apache.lucene.analysis.miscellaneous.KeywordRepeatFilterFactory}
    </p>
*/
class PorterStemFilter final : public TokenFilter
{
  GET_CLASS_NAME(PorterStemFilter)
private:
  const std::shared_ptr<PorterStemmer> stemmer =
      std::make_shared<PorterStemmer>();
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAttr =
      addAttribute(KeywordAttribute::typeid);

public:
  PorterStemFilter(std::shared_ptr<TokenStream> in_);

  bool incrementToken()  override final;

protected:
  std::shared_ptr<PorterStemFilter> shared_from_this()
  {
    return std::static_pointer_cast<PorterStemFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::en
