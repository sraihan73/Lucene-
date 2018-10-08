#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/uhighlight/PhraseHelper.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/automaton/CharacterRunAutomaton.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/OffsetsEnum.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"

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

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using Terms = org::apache::lucene::index::Terms;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

/**
 * Ultimately returns an {@link OffsetsEnum} yielding potentially highlightable
 * words in the text.  Needs information about the query up front.
 *
 * @lucene.internal
 */
class FieldOffsetStrategy
    : public std::enable_shared_from_this<FieldOffsetStrategy>
{
  GET_CLASS_NAME(FieldOffsetStrategy)

protected:
  const std::wstring field;
  const std::shared_ptr<PhraseHelper>
      phraseHelper; // Query: position-sensitive information
  std::deque<std::shared_ptr<BytesRef>> const
      terms; // Query: all terms we extracted (some may be position sensitive)
  std::deque<std::shared_ptr<CharacterRunAutomaton>> const
      automata; // Query: wildcards (i.e. multi-term query), not position
                // sensitive

public:
  FieldOffsetStrategy(
      const std::wstring &field,
      std::deque<std::shared_ptr<BytesRef>> &queryTerms,
      std::shared_ptr<PhraseHelper> phraseHelper,
      std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata);

  virtual std::wstring getField();

  virtual UnifiedHighlighter::OffsetSource getOffsetSource() = 0;

  /**
   * The primary method -- return offsets for highlightable words in the
   * specified document.
   *
   * Callers are expected to close the returned OffsetsEnum when it has been
   * finished with
   */
  virtual std::shared_ptr<OffsetsEnum>
  getOffsetsEnum(std::shared_ptr<IndexReader> reader, int docId,
                 const std::wstring &content) = 0;

protected:
  virtual std::shared_ptr<OffsetsEnum>
  createOffsetsEnumFromReader(std::shared_ptr<LeafReader> leafReader,
                              int doc) ;

  virtual void createOffsetsEnumsForTerms(
      std::deque<std::shared_ptr<BytesRef>> &sourceTerms,
      std::shared_ptr<Terms> termsIndex, int doc,
      std::deque<std::shared_ptr<OffsetsEnum>> &results) ;

  virtual void createOffsetsEnumsForAutomata(
      std::shared_ptr<Terms> termsIndex, int doc,
      std::deque<std::shared_ptr<OffsetsEnum>> &results) ;
};

} // #include  "core/src/java/org/apache/lucene/search/uhighlight/
