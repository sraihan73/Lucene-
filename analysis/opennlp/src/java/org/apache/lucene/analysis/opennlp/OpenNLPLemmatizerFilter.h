#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/opennlp/tools/NLPLemmatizerOp.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/FlagsAttribute.h"
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"
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

namespace org::apache::lucene::analysis::opennlp
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using NLPLemmatizerOp =
    org::apache::lucene::analysis::opennlp::tools::NLPLemmatizerOp;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using FlagsAttribute =
    org::apache::lucene::analysis::tokenattributes::FlagsAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

/**
 * <p>Runs OpenNLP dictionary-based and/or MaxEnt lemmatizers.</p>
 * <p>
 *   Both a dictionary-based lemmatizer and a MaxEnt lemmatizer are supported,
 *   via the "dictionary" and "lemmatizerModel" params, respectively.
 *   If both are configured, the dictionary-based lemmatizer is tried first,
 *   and then the MaxEnt lemmatizer is consulted for out-of-vocabulary tokens.
 * </p>
 * <p>
 *   The dictionary file must be encoded as UTF-8, with one entry per line,
 *   in the form <tt>word[tab]lemma[tab]part-of-speech</tt>
 * </p>
 */
class OpenNLPLemmatizerFilter : public TokenFilter
{
  GET_CLASS_NAME(OpenNLPLemmatizerFilter)
private:
  const std::shared_ptr<NLPLemmatizerOp> lemmatizerOp;
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAtt =
      addAttribute(TypeAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAtt =
      addAttribute(KeywordAttribute::typeid);
  const std::shared_ptr<FlagsAttribute> flagsAtt =
      addAttribute(FlagsAttribute::typeid);
  std::deque<std::shared_ptr<AttributeSource>> sentenceTokenAttrs =
      std::deque<std::shared_ptr<AttributeSource>>();
  std::shared_ptr<Iterator<std::shared_ptr<AttributeSource>>>
      sentenceTokenAttrsIter = nullptr;
  bool moreTokensAvailable = true;
  std::deque<std::wstring> sentenceTokens;     // non-keyword tokens
  std::deque<std::wstring> sentenceTokenTypes; // types for non-keyword tokens
  std::deque<std::wstring> lemmas;             // lemmas for non-keyword tokens
  int lemmaNum = 0;                             // lemma counter

public:
  OpenNLPLemmatizerFilter(std::shared_ptr<TokenStream> input,
                          std::shared_ptr<NLPLemmatizerOp> lemmatizerOp);

  bool incrementToken()  override final;

private:
  void nextSentence() ;

public:
  void reset()  override;

private:
  void clear();

protected:
  std::shared_ptr<OpenNLPLemmatizerFilter> shared_from_this()
  {
    return std::static_pointer_cast<OpenNLPLemmatizerFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/opennlp/
