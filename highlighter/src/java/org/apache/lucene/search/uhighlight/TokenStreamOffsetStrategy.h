#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/util/automaton/CharacterRunAutomaton.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/PhraseHelper.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/OffsetsEnum.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"

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
namespace org::apache::lucene::search::uhighlight
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using IndexReader = org::apache::lucene::index::IndexReader;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

/**
 * Analyzes the text, producing a single {@link OffsetsEnum} wrapping the {@link
 * TokenStream} filtered to terms in the query, including wildcards.  It can't
 * handle position-sensitive queries (phrases). Passage accuracy suffers because
 * the freq() is unknown -- it's always {@link Integer#MAX_VALUE} instead.
 */
class TokenStreamOffsetStrategy : public AnalysisOffsetStrategy
{
  GET_CLASS_NAME(TokenStreamOffsetStrategy)

private:
  static std::deque<std::shared_ptr<BytesRef>> const ZERO_LEN_BYTES_REF_ARRAY;

public:
  TokenStreamOffsetStrategy(
      const std::wstring &field, std::deque<std::shared_ptr<BytesRef>> &terms,
      std::shared_ptr<PhraseHelper> phraseHelper,
      std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata,
      std::shared_ptr<Analyzer> indexAnalyzer);

private:
  static std::deque<std::shared_ptr<CharacterRunAutomaton>>
  convertTermsToAutomata(
      std::deque<std::shared_ptr<BytesRef>> &terms,
      std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata);

private:
  class CharacterRunAutomatonAnonymousInnerClass : public CharacterRunAutomaton
  {
    GET_CLASS_NAME(CharacterRunAutomatonAnonymousInnerClass)
  private:
    std::wstring termString;

  public:
    CharacterRunAutomatonAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::util::automaton::Automaton>
            makeString,
        const std::wstring &termString);

    virtual std::wstring toString();

  protected:
    std::shared_ptr<CharacterRunAutomatonAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CharacterRunAutomatonAnonymousInnerClass>(
          org.apache.lucene.util.automaton
              .CharacterRunAutomaton::shared_from_this());
    }
  };

public:
  std::shared_ptr<OffsetsEnum>
  getOffsetsEnum(std::shared_ptr<IndexReader> reader, int docId,
                 const std::wstring &content)  override;

private:
  class TokenStreamOffsetsEnum : public OffsetsEnum
  {
    GET_CLASS_NAME(TokenStreamOffsetsEnum)
  public:
    std::shared_ptr<TokenStream> stream; // becomes null when closed
    std::deque<std::shared_ptr<CharacterRunAutomaton>> const matchers;
    const std::shared_ptr<CharTermAttribute> charTermAtt;
    const std::shared_ptr<OffsetAttribute> offsetAtt;

    int currentMatch = -1;

    std::deque<std::shared_ptr<BytesRef>> const matchDescriptions;

    TokenStreamOffsetsEnum(std::shared_ptr<TokenStream> ts,
                           std::deque<std::shared_ptr<CharacterRunAutomaton>>
                               &matchers) ;

    bool nextPosition()  override;

    int freq()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getTerm()  override;

    virtual ~TokenStreamOffsetsEnum();

  protected:
    std::shared_ptr<TokenStreamOffsetsEnum> shared_from_this()
    {
      return std::static_pointer_cast<TokenStreamOffsetsEnum>(
          OffsetsEnum::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TokenStreamOffsetStrategy> shared_from_this()
  {
    return std::static_pointer_cast<TokenStreamOffsetStrategy>(
        AnalysisOffsetStrategy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/uhighlight/
