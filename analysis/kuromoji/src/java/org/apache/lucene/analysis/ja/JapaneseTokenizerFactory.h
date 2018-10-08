#pragma once
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoaderAware.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include "JapaneseTokenizer.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::ja::dict
{
class UserDictionary;
}

namespace org::apache::lucene::analysis::util
{
class ResourceLoader;
}
namespace org::apache::lucene::analysis::ja
{
class JapaneseTokenizer;
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
namespace org::apache::lucene::analysis::ja
{

using Mode = org::apache::lucene::analysis::ja::JapaneseTokenizer::Mode;
using UserDictionary = org::apache::lucene::analysis::ja::dict::UserDictionary;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;

/**
 * Factory for {@link org.apache.lucene.analysis.ja.JapaneseTokenizer}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_ja" class="solr.TextField"&gt;
 *   &lt;analyzer&gt;
 *     &lt;tokenizer class="solr.JapaneseTokenizerFactory"
 *       mode="NORMAL"
 *       userDictionary="user.txt"
 *       userDictionaryEncoding="UTF-8"
 *       discardPunctuation="true"
 *     /&gt;
 *     &lt;filter class="solr.JapaneseBaseFormFilterFactory"/&gt;
 *   &lt;/analyzer&gt;
 * &lt;/fieldType&gt;
 * </pre>
 * <p>
 * Additional expert user parameters nBestCost and nBestExamples can be
 * used to include additional searchable tokens that those most likely
 * according to the statistical model. A typical use-case for this is to
 * improve recall and make segmentation more resilient to mistakes.
 * The feature can also be used to get a decompounding effect.
 * <p>
 * The nBestCost parameter specifies an additional Viterbi cost, and
 * when used, JapaneseTokenizer will include all tokens in Viterbi paths
 * that are within the nBestCost value of the best path.
 * <p>
 * Finding a good value for nBestCost can be difficult to do by hand. The
 * nBestExamples parameter can be used to find an nBestCost value based on
 * examples with desired segmentation outcomes.
 * <p>
 * For example, a value of /箱根山-箱根/成田空港-成田/ indicates that in
 * the texts, 箱根山 (Mt. Hakone) and 成田空港 (Narita Airport) we'd like
 * a cost that gives is us 箱根 (Hakone) and 成田 (Narita). Notice that
 * costs are estimated for each example individually, and the maximum
 * nBestCost found across all examples is used.
 * <p>
 * If both nBestCost and nBestExamples is used in a configuration,
 * the largest value of the two is used.
 * <p>
 * Parameters nBestCost and nBestExamples work with all tokenizer
 * modes, but it makes the most sense to use them with NORMAL mode.
 */
class JapaneseTokenizerFactory : public TokenizerFactory,
                                 public ResourceLoaderAware
{
  GET_CLASS_NAME(JapaneseTokenizerFactory)
private:
  static const std::wstring MODE;

  static const std::wstring USER_DICT_PATH;

  static const std::wstring USER_DICT_ENCODING;

  static const std::wstring DISCARD_PUNCTUATION; // Expert option

  static const std::wstring NBEST_COST;

  static const std::wstring NBEST_EXAMPLES;

  std::shared_ptr<UserDictionary> userDictionary;

  const Mode mode;
  const bool discardPunctuation;
  const std::wstring userDictionaryPath;
  const std::wstring userDictionaryEncoding;

  /* Example string for NBEST output.
   * its form as:
   *   nbestExamples := [ / ] example [ / example ]... [ / ]
   *   example := TEXT - TOKEN
   *   TEXT := input text
   *   TOKEN := token should be in nbest result
   * Ex. /箱根山-箱根/成田空港-成田/
   * When the result tokens are "箱根山", "成田空港" in NORMAL mode,
   * /箱根山-箱根/成田空港-成田/ requests "箱根" and "成田" to be in the result
   * in NBEST output.
   */
  const std::wstring nbestExamples;
  int nbestCost = -1;

  /** Creates a new JapaneseTokenizerFactory */
public:
  JapaneseTokenizerFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

  std::shared_ptr<JapaneseTokenizer>
  create(std::shared_ptr<AttributeFactory> factory) override;

protected:
  std::shared_ptr<JapaneseTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<JapaneseTokenizerFactory>(
        org.apache.lucene.analysis.util.TokenizerFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ja
