#pragma once
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/memory/MemoryIndex.h"

#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/util/automaton/CharacterRunAutomaton.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/PhraseHelper.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/OffsetsEnum.h"
#include  "core/src/java/org/apache/lucene/analysis/FilteringTokenFilter.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

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
using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using MemoryIndex = org::apache::lucene::index::memory::MemoryIndex;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

/**
 * Uses an {@link Analyzer} on content to get offsets and then populates a
 * {@link MemoryIndex}.
 *
 * @lucene.internal
 */
class MemoryIndexOffsetStrategy : public AnalysisOffsetStrategy
{
  GET_CLASS_NAME(MemoryIndexOffsetStrategy)

private:
  const std::shared_ptr<MemoryIndex> memoryIndex;
  const std::shared_ptr<LeafReader> leafReader;
  const std::shared_ptr<CharacterRunAutomaton> preMemIndexFilterAutomaton;

public:
  MemoryIndexOffsetStrategy(
      const std::wstring &field,
      std::function<bool(const std::wstring &)> &fieldMatcher,
      std::deque<std::shared_ptr<BytesRef>> &extractedTerms,
      std::shared_ptr<PhraseHelper> phraseHelper,
      std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata,
      std::shared_ptr<Analyzer> analyzer,
      std::function<std::deque<std::shared_ptr<Query>>> &multiTermQueryRewrite);

  /**
   * Build one {@link CharacterRunAutomaton} matching any term the query might
   * match.
   */
private:
  static std::shared_ptr<CharacterRunAutomaton> buildCombinedAutomaton(
      std::function<bool(const std::wstring &)> &fieldMatcher,
      std::deque<std::shared_ptr<BytesRef>> &terms,
      std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata,
      std::shared_ptr<PhraseHelper> strictPhrases,
      std::function<std::deque<std::shared_ptr<Query>>> &multiTermQueryRewrite);

private:
  class CharacterRunAutomatonAnonymousInnerClass : public CharacterRunAutomaton
  {
    GET_CLASS_NAME(CharacterRunAutomatonAnonymousInnerClass)
  private:
    std::deque<std::shared_ptr<CharacterRunAutomaton>> allAutomata;

  public:
    CharacterRunAutomatonAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::util::automaton::Automaton>
            makeEmpty,
        std::deque<std::shared_ptr<CharacterRunAutomaton>> &allAutomata);

    bool run(std::deque<wchar_t> &chars, int offset, int length) override;

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
  static std::shared_ptr<FilteringTokenFilter>
  newKeepWordFilter(std::shared_ptr<TokenStream> tokenStream,
                    std::shared_ptr<CharacterRunAutomaton> charRunAutomaton);

private:
  class FilteringTokenFilterAnonymousInnerClass : public FilteringTokenFilter
  {
    GET_CLASS_NAME(FilteringTokenFilterAnonymousInnerClass)
  private:
    std::shared_ptr<CharacterRunAutomaton> charRunAutomaton;

  public:
    FilteringTokenFilterAnonymousInnerClass(
        std::shared_ptr<TokenStream> tokenStream,
        std::shared_ptr<CharacterRunAutomaton> charRunAutomaton);

    const std::shared_ptr<CharTermAttribute> charAtt =
        addAttribute(CharTermAttribute::typeid);

  protected:
    bool accept()  override;

  protected:
    std::shared_ptr<FilteringTokenFilterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilteringTokenFilterAnonymousInnerClass>(
          org.apache.lucene.analysis.FilteringTokenFilter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<MemoryIndexOffsetStrategy> shared_from_this()
  {
    return std::static_pointer_cast<MemoryIndexOffsetStrategy>(
        AnalysisOffsetStrategy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/uhighlight/
