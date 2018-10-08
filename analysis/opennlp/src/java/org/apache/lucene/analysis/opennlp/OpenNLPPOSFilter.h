#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"

#include  "core/src/java/org/apache/lucene/analysis/opennlp/tools/NLPPOSTaggerOp.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/FlagsAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
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
using NLPPOSTaggerOp =
    org::apache::lucene::analysis::opennlp::tools::NLPPOSTaggerOp;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using FlagsAttribute =
    org::apache::lucene::analysis::tokenattributes::FlagsAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

/**
 * Run OpenNLP POS tagger.  Tags all terms in the TypeAttribute.
 */
class OpenNLPPOSFilter final : public TokenFilter
{
  GET_CLASS_NAME(OpenNLPPOSFilter)

private:
  std::deque<std::shared_ptr<AttributeSource>> sentenceTokenAttrs =
      std::deque<std::shared_ptr<AttributeSource>>();

public:
  std::deque<std::wstring> tags;

private:
  int tokenNum = 0;
  bool moreTokensAvailable = true;

  const std::shared_ptr<NLPPOSTaggerOp> posTaggerOp;
  const std::shared_ptr<TypeAttribute> typeAtt =
      addAttribute(TypeAttribute::typeid);
  const std::shared_ptr<FlagsAttribute> flagsAtt =
      addAttribute(FlagsAttribute::typeid);
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);

public:
  OpenNLPPOSFilter(std::shared_ptr<TokenStream> input,
                   std::shared_ptr<NLPPOSTaggerOp> posTaggerOp);

  bool incrementToken()  override final;

private:
  std::deque<std::wstring> nextSentence() ;

public:
  void reset()  override;

private:
  void clear();

protected:
  std::shared_ptr<OpenNLPPOSFilter> shared_from_this()
  {
    return std::static_pointer_cast<OpenNLPPOSFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/opennlp/
