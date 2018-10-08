#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/automaton/CharacterRunAutomaton.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/PhraseHelper.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/UnifiedHighlighter.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
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
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

/**
 * Provides a base class for analysis based offset strategies to extend from.
 * Requires an Analyzer and provides an override-able method for altering how
 * the TokenStream is created.
 *
 * @lucene.internal
 */
class AnalysisOffsetStrategy : public FieldOffsetStrategy
{
  GET_CLASS_NAME(AnalysisOffsetStrategy)

protected:
  const std::shared_ptr<Analyzer> analyzer;

public:
  AnalysisOffsetStrategy(
      const std::wstring &field,
      std::deque<std::shared_ptr<BytesRef>> &queryTerms,
      std::shared_ptr<PhraseHelper> phraseHelper,
      std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata,
      std::shared_ptr<Analyzer> analyzer);

  UnifiedHighlighter::OffsetSource getOffsetSource() override final;

protected:
  virtual std::shared_ptr<TokenStream>
  tokenStream(const std::wstring &content) ;

  /**
   * Wraps an {@link Analyzer} and string text that represents multiple values
   * delimited by a specified character. This exposes a TokenStream that matches
   * what would get indexed considering the
   * {@link Analyzer#getPositionIncrementGap(std::wstring)}. Currently this assumes
   * {@link Analyzer#getOffsetGap(std::wstring)} is 1; an exception will be thrown if
   * it isn't. <br /> It would be more orthogonal for this to be an Analyzer
   * since we're wrapping an Analyzer but doing so seems like more work.  The
   * underlying components see a Reader not a std::wstring -- and the std::wstring is easy
   * to split up without redundant buffering.
   *
   * @lucene.internal
   */
  // TODO we could make this go away.  MemoryIndexOffsetStrategy could simply
  // split and analyze each value into the
  //   MemoryIndex. TokenStreamOffsetStrategy's hack TokenStreamPostingsEnum
  //   could incorporate this logic, albeit with less code, less hack.
private:
  class MultiValueTokenStream final : public TokenFilter
  {
    GET_CLASS_NAME(MultiValueTokenStream)

  private:
    const std::wstring fieldName;
    const std::shared_ptr<Analyzer> indexAnalyzer;
    const std::wstring content;
    const wchar_t splitChar;

    const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
        addAttribute(PositionIncrementAttribute::typeid);
    const std::shared_ptr<OffsetAttribute> offsetAtt =
        addAttribute(OffsetAttribute::typeid);

    int startValIdx = 0;
    int endValIdx = 0;
    int remainingPosInc = 0;

    MultiValueTokenStream(
        std::shared_ptr<TokenStream> subTokenStream,
        const std::wstring &fieldName, std::shared_ptr<Analyzer> indexAnalyzer,
        const std::wstring &content, wchar_t splitChar,
        int splitCharIdx); // subTokenStream is already initialized to operate
                           // on the first value

  public:
    void reset()  override;

    bool incrementToken()  override;

    void end()  override;

  protected:
    std::shared_ptr<MultiValueTokenStream> shared_from_this()
    {
      return std::static_pointer_cast<MultiValueTokenStream>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<AnalysisOffsetStrategy> shared_from_this()
  {
    return std::static_pointer_cast<AnalysisOffsetStrategy>(
        FieldOffsetStrategy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/uhighlight/
