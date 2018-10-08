#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cctype>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/morfologik/MorphosyntacticTagsAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include  "core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
namespace org::apache::lucene::analysis::morfologik
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;

using morfologik::stemming::Dictionary;
using morfologik::stemming::IStemmer;
using morfologik::stemming::WordData;

/**
 * {@link TokenFilter} using Morfologik library to transform input tokens into
 * lemma and morphosyntactic (POS) tokens. Applies to Polish only.
 *
 * <p>MorfologikFilter contains a {@link MorphosyntacticTagsAttribute}, which
 * provides morphosyntactic annotations for produced lemmas. See the Morfologik
 * documentation for details.</p>
 *
 * @see <a href="http://morfologik.blogspot.com/">Morfologik project page</a>
 */
class MorfologikFilter : public TokenFilter
{
  GET_CLASS_NAME(MorfologikFilter)

private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<MorphosyntacticTagsAttribute> tagsAtt =
      addAttribute(MorphosyntacticTagsAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAttr =
      addAttribute(KeywordAttribute::typeid);

  const std::shared_ptr<CharsRefBuilder> scratch =
      std::make_shared<CharsRefBuilder>();

  std::shared_ptr<State> current;
  const std::shared_ptr<TokenStream> input;
  const std::shared_ptr<IStemmer> stemmer;

  std::deque<std::shared_ptr<WordData>> lemmaList;
  const std::deque<std::shared_ptr<StringBuilder>> tagsList =
      std::deque<std::shared_ptr<StringBuilder>>();

  int lemmaListIndex = 0;

  /**
   * Creates a filter with the default (Polish) dictionary.
   */
public:
  MorfologikFilter(std::shared_ptr<TokenStream> in_);

  /**
   * Creates a filter with a given dictionary.
   *
   * @param in input token stream.
   * @param dict Dictionary to use for stemming.
   */
  MorfologikFilter(std::shared_ptr<TokenStream> in_,
                   std::shared_ptr<Dictionary> dict);

  /**
   * A pattern used to split lemma forms.
   */
private:
  static const std::shared_ptr<Pattern> lemmaSplitter;

  void popNextLemma();

  /**
   * Lookup a given surface form of a token and update
   * {@link #lemmaList} and {@link #lemmaListIndex} accordingly.
   */
  bool lookupSurfaceForm(std::shared_ptr<std::wstring> token);

  /** Retrieves the next token (possibly from the deque of lemmas). */
public:
  bool incrementToken()  override final;

  /**
   * Convert to lowercase in-place.
   */
private:
  std::shared_ptr<std::wstring> toLowercase(std::shared_ptr<std::wstring> chs);

  /** Resets stems accumulator and hands over to superclass. */
public:
  void reset()  override;

protected:
  std::shared_ptr<MorfologikFilter> shared_from_this()
  {
    return std::static_pointer_cast<MorfologikFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/morfologik/
