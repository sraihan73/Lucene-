#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::util::automaton
{
class CharacterRunAutomaton;
}
namespace org::apache::lucene::search::uhighlight
{
class PhraseHelper;
}
namespace org::apache::lucene::search::uhighlight
{
class UnifiedHighlighter;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search::uhighlight
{
class OffsetsEnum;
}
namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::analysis
{
class MockAnalyzer;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search::uhighlight
{
class PassageScorer;
}
namespace org::apache::lucene::search::uhighlight
{
class PassageFormatter;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search::spans
{
class SpanQuery;
}
namespace org::apache::lucene::search::uhighlight
{
class LimitedStoredFieldVisitor;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::search::uhighlight
{
class FieldHighlighter;
}
namespace org::apache::lucene::search::uhighlight
{
class FieldOffsetStrategy;
}
namespace org::apache::lucene::search::uhighlight
{
class Passage;
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

namespace org::apache::lucene::search::uhighlight::visibility
{

using IndexReader = org::apache::lucene::index::IndexReader;
using FieldHighlighter =
    org::apache::lucene::search::uhighlight::FieldHighlighter;
using FieldOffsetStrategy =
    org::apache::lucene::search::uhighlight::FieldOffsetStrategy;
using OffsetsEnum = org::apache::lucene::search::uhighlight::OffsetsEnum;
using Passage = org::apache::lucene::search::uhighlight::Passage;
using PassageFormatter =
    org::apache::lucene::search::uhighlight::PassageFormatter;
using PassageScorer = org::apache::lucene::search::uhighlight::PassageScorer;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Helps us be aware of visibility/extensibility concerns.
 */
class TestUnifiedHighlighterExtensibility : public LuceneTestCase
{
  GET_CLASS_NAME(TestUnifiedHighlighterExtensibility)

  /**
   * This test is for maintaining the extensibility of the FieldOffsetStrategy
   * for customizations out of package.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFieldOffsetStrategyExtensibility()
  virtual void testFieldOffsetStrategyExtensibility();

private:
  class FieldOffsetStrategyAnonymousInnerClass : public FieldOffsetStrategy
  {
    GET_CLASS_NAME(FieldOffsetStrategyAnonymousInnerClass)
  private:
    std::shared_ptr<TestUnifiedHighlighterExtensibility> outerInstance;

    UnifiedHighlighter::OffsetSource offsetSource =
        static_cast<UnifiedHighlighter::OffsetSource>(0);

  public:
    FieldOffsetStrategyAnonymousInnerClass(
        std::shared_ptr<TestUnifiedHighlighterExtensibility> outerInstance,
        std::deque<std::shared_ptr<BytesRef>> &org,
        std::shared_ptr<PhraseHelper> NONE,
        std::deque<std::shared_ptr<CharacterRunAutomaton>> &org,
        UnifiedHighlighter::OffsetSource offsetSource);

    UnifiedHighlighter::OffsetSource getOffsetSource() override;

    std::shared_ptr<OffsetsEnum>
    getOffsetsEnum(std::shared_ptr<IndexReader> reader, int docId,
                   const std::wstring &content)  override;

  protected:
    std::shared_ptr<OffsetsEnum>
    createOffsetsEnumFromReader(std::shared_ptr<LeafReader> leafReader,
                                int doc)  override;

  protected:
    std::shared_ptr<FieldOffsetStrategyAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FieldOffsetStrategyAnonymousInnerClass>(
          org.apache.lucene.search.uhighlight
              .FieldOffsetStrategy::shared_from_this());
    }
  };

  /**
   * This test is for maintaining the extensibility of the UnifiedHighlighter
   * for customizations out of package.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testUnifiedHighlighterExtensibility()
  virtual void testUnifiedHighlighterExtensibility();

private:
  class UnifiedHighlighterAnonymousInnerClass : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass)
  private:
    std::shared_ptr<TestUnifiedHighlighterExtensibility> outerInstance;

    int maxLength = 0;

  public:
    UnifiedHighlighterAnonymousInnerClass(
        std::shared_ptr<TestUnifiedHighlighterExtensibility> outerInstance,
        std::shared_ptr<MockAnalyzer> org, int maxLength);

  protected:
    std::unordered_map<std::wstring, std::deque<std::any>>
    highlightFieldsAsObjects(
        std::deque<std::wstring> &fieldsIn, std::shared_ptr<Query> query,
        std::deque<int> &docIdsIn,
        std::deque<int> &maxPassagesIn)  override;

    OffsetSource getOffsetSource(const std::wstring &field) override;

    std::shared_ptr<BreakIterator>
    getBreakIterator(const std::wstring &field) override;

    std::shared_ptr<PassageScorer>
    getScorer(const std::wstring &field) override;

    std::shared_ptr<PassageFormatter>
    getFormatter(const std::wstring &field) override;

  public:
    std::shared_ptr<Analyzer> getIndexAnalyzer() override;

    std::shared_ptr<IndexSearcher> getIndexSearcher() override;

  protected:
    int getMaxNoHighlightPassages(const std::wstring &field) override;

    std::optional<bool>
    requiresRewrite(std::shared_ptr<SpanQuery> spanQuery) override;

    std::shared_ptr<UnifiedHighlighter::LimitedStoredFieldVisitor>
    newLimitedStoredFieldsVisitor(std::deque<std::wstring> &fields) override;

    std::deque<std::deque<std::shared_ptr<std::wstring>>>
    loadFieldValues(std::deque<std::wstring> &fields,
                    std::shared_ptr<DocIdSetIterator> docIter,
                    int cacheCharsThreshold)  override;

    std::shared_ptr<FieldHighlighter>
    getFieldHighlighter(const std::wstring &field, std::shared_ptr<Query> query,
                        std::shared_ptr<Set<std::shared_ptr<Term>>> allTerms,
                        int maxPassages) override;

    std::shared_ptr<FieldOffsetStrategy> getOffsetStrategy(
        OffsetSource offsetSource, const std::wstring &field,
        std::deque<std::shared_ptr<BytesRef>> &terms,
        std::shared_ptr<PhraseHelper> phraseHelper,
        std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata,
        std::shared_ptr<Set<HighlightFlag>> highlightFlags) override;

  public:
    int getMaxLength() override;

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass>(
          org.apache.lucene.search.uhighlight
              .UnifiedHighlighter::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPassageFormatterExtensibility()
  virtual void testPassageFormatterExtensibility();

private:
  class PassageFormatterAnonymousInnerClass : public PassageFormatter
  {
    GET_CLASS_NAME(PassageFormatterAnonymousInnerClass)
  private:
    std::shared_ptr<TestUnifiedHighlighterExtensibility> outerInstance;

    std::any formattedResponse;

  public:
    PassageFormatterAnonymousInnerClass(
        std::shared_ptr<TestUnifiedHighlighterExtensibility> outerInstance,
        std::any formattedResponse);

    std::any format(std::deque<std::shared_ptr<Passage>> &passages,
                    const std::wstring &content) override;

  protected:
    std::shared_ptr<PassageFormatterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PassageFormatterAnonymousInnerClass>(
          org.apache.lucene.search.uhighlight
              .PassageFormatter::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFieldHiglighterExtensibility()
  virtual void testFieldHiglighterExtensibility();

private:
  class FieldHighlighterAnonymousInnerClass : public FieldHighlighter
  {
    GET_CLASS_NAME(FieldHighlighterAnonymousInnerClass)
  private:
    std::shared_ptr<TestUnifiedHighlighterExtensibility> outerInstance;

  public:
    FieldHighlighterAnonymousInnerClass(
        std::shared_ptr<TestUnifiedHighlighterExtensibility> outerInstance);

  protected:
    std::deque<std::shared_ptr<Passage>> highlightOffsetsEnums(
        std::shared_ptr<OffsetsEnum> offsetsEnums)  override;

  protected:
    std::shared_ptr<FieldHighlighterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FieldHighlighterAnonymousInnerClass>(
          org.apache.lucene.search.uhighlight
              .FieldHighlighter::shared_from_this());
    }
  };

  /** Tests maintaining extensibility/visibility of {@link
   * org.apache.lucene.search.uhighlight.FieldHighlighter} out of package. */
private:
  class CustomFieldHighlighter : public FieldHighlighter
  {
    GET_CLASS_NAME(CustomFieldHighlighter)
  public:
    CustomFieldHighlighter(
        const std::wstring &field,
        std::shared_ptr<FieldOffsetStrategy> fieldOffsetStrategy,
        std::shared_ptr<BreakIterator> breakIterator,
        std::shared_ptr<PassageScorer> passageScorer, int maxPassages,
        int maxNoHighlightPassages,
        std::shared_ptr<PassageFormatter> passageFormatter);

    std::any highlightFieldForDoc(
        std::shared_ptr<IndexReader> reader, int docId,
        const std::wstring &content)  override;

  protected:
    std::deque<std::shared_ptr<Passage>> highlightOffsetsEnums(
        std::shared_ptr<OffsetsEnum> offsetsEnums)  override;

  protected:
    std::shared_ptr<CustomFieldHighlighter> shared_from_this()
    {
      return std::static_pointer_cast<CustomFieldHighlighter>(
          org.apache.lucene.search.uhighlight
              .FieldHighlighter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestUnifiedHighlighterExtensibility> shared_from_this()
  {
    return std::static_pointer_cast<TestUnifiedHighlighterExtensibility>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::uhighlight::visibility
