using namespace std;

#include "TestUnifiedHighlighterExtensibility.h"

namespace org::apache::lucene::search::uhighlight::visibility
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using FieldHighlighter =
    org::apache::lucene::search::uhighlight::FieldHighlighter;
using FieldOffsetStrategy =
    org::apache::lucene::search::uhighlight::FieldOffsetStrategy;
using OffsetsEnum = org::apache::lucene::search::uhighlight::OffsetsEnum;
using Passage = org::apache::lucene::search::uhighlight::Passage;
using PassageFormatter =
    org::apache::lucene::search::uhighlight::PassageFormatter;
using PassageScorer = org::apache::lucene::search::uhighlight::PassageScorer;
using PhraseHelper = org::apache::lucene::search::uhighlight::PhraseHelper;
using SplittingBreakIterator =
    org::apache::lucene::search::uhighlight::SplittingBreakIterator;
using UnifiedHighlighter =
    org::apache::lucene::search::uhighlight::UnifiedHighlighter;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFieldOffsetStrategyExtensibility()
void TestUnifiedHighlighterExtensibility::testFieldOffsetStrategyExtensibility()
{
  constexpr UnifiedHighlighter::OffsetSource offsetSource =
      UnifiedHighlighter::OffsetSource::NONE_NEEDED;
  shared_ptr<FieldOffsetStrategy> strategy =
      make_shared<FieldOffsetStrategyAnonymousInnerClass>(
          shared_from_this(), std::deque<std::shared_ptr<BytesRef>>(0),
          PhraseHelper::NONE,
          std::deque<std::shared_ptr<CharacterRunAutomaton>>(0), offsetSource);
  assertEquals(offsetSource, strategy->getOffsetSource());
}

TestUnifiedHighlighterExtensibility::FieldOffsetStrategyAnonymousInnerClass::
    FieldOffsetStrategyAnonymousInnerClass(
        shared_ptr<TestUnifiedHighlighterExtensibility> outerInstance,
        std::deque<std::shared_ptr<BytesRef>> &org,
        shared_ptr<PhraseHelper> NONE,
        std::deque<std::shared_ptr<CharacterRunAutomaton>> &org,
        UnifiedHighlighter::OffsetSource offsetSource)
    : org.apache::lucene::search::uhighlight::FieldOffsetStrategy(
          L"field", BytesRef, NONE, CharacterRunAutomaton)
{
  this->outerInstance = outerInstance;
  this->offsetSource = offsetSource;
}

UnifiedHighlighter::OffsetSource TestUnifiedHighlighterExtensibility::
    FieldOffsetStrategyAnonymousInnerClass::getOffsetSource()
{
  return offsetSource;
}

shared_ptr<OffsetsEnum>
TestUnifiedHighlighterExtensibility::FieldOffsetStrategyAnonymousInnerClass::
    getOffsetsEnum(shared_ptr<IndexReader> reader, int docId,
                   const wstring &content) 
{
  return OffsetsEnum::EMPTY;
}

shared_ptr<OffsetsEnum>
TestUnifiedHighlighterExtensibility::FieldOffsetStrategyAnonymousInnerClass::
    createOffsetsEnumFromReader(shared_ptr<LeafReader> leafReader,
                                int doc) 
{
  return outerInstance->super->createOffsetsEnumFromReader(leafReader, doc);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testUnifiedHighlighterExtensibility()
void TestUnifiedHighlighterExtensibility::testUnifiedHighlighterExtensibility()
{
  constexpr int maxLength = 1000;
  shared_ptr<UnifiedHighlighter> uh =
      make_shared<UnifiedHighlighterAnonymousInnerClass>(
          shared_from_this(), make_shared<MockAnalyzer>(random()), maxLength);
  assertEquals(uh->getMaxLength(), maxLength);
}

TestUnifiedHighlighterExtensibility::UnifiedHighlighterAnonymousInnerClass::
    UnifiedHighlighterAnonymousInnerClass(
        shared_ptr<TestUnifiedHighlighterExtensibility> outerInstance,
        shared_ptr<MockAnalyzer> org, int maxLength)
    : org->apache.lucene.search.uhighlight.UnifiedHighlighter(nullptr,
                                                              MockAnalyzer)
{
  this->outerInstance = outerInstance;
  this->maxLength = maxLength;
}

unordered_map<wstring, std::deque<any>>
TestUnifiedHighlighterExtensibility::UnifiedHighlighterAnonymousInnerClass::
    highlightFieldsAsObjects(std::deque<wstring> &fieldsIn,
                             shared_ptr<Query> query,
                             std::deque<int> &docIdsIn,
                             std::deque<int> &maxPassagesIn) 
{
  return outerInstance->super->highlightFieldsAsObjects(
      fieldsIn, query, docIdsIn, maxPassagesIn);
}

OffsetSource TestUnifiedHighlighterExtensibility::
    UnifiedHighlighterAnonymousInnerClass::getOffsetSource(const wstring &field)
{
  return outerInstance->super->getOffsetSource(field);
}

shared_ptr<BreakIterator>
TestUnifiedHighlighterExtensibility::UnifiedHighlighterAnonymousInnerClass::
    getBreakIterator(const wstring &field)
{
  return outerInstance->super->getBreakIterator(field);
}

shared_ptr<PassageScorer> TestUnifiedHighlighterExtensibility::
    UnifiedHighlighterAnonymousInnerClass::getScorer(const wstring &field)
{
  return outerInstance->super->getScorer(field);
}

shared_ptr<PassageFormatter> TestUnifiedHighlighterExtensibility::
    UnifiedHighlighterAnonymousInnerClass::getFormatter(const wstring &field)
{
  return outerInstance->super->getFormatter(field);
}

shared_ptr<Analyzer> TestUnifiedHighlighterExtensibility::
    UnifiedHighlighterAnonymousInnerClass::getIndexAnalyzer()
{
  return outerInstance->super->getIndexAnalyzer();
}

shared_ptr<IndexSearcher> TestUnifiedHighlighterExtensibility::
    UnifiedHighlighterAnonymousInnerClass::getIndexSearcher()
{
  return outerInstance->super->getIndexSearcher();
}

int TestUnifiedHighlighterExtensibility::UnifiedHighlighterAnonymousInnerClass::
    getMaxNoHighlightPassages(const wstring &field)
{
  return outerInstance->super->getMaxNoHighlightPassages(field);
}

optional<bool>
TestUnifiedHighlighterExtensibility::UnifiedHighlighterAnonymousInnerClass::
    requiresRewrite(shared_ptr<SpanQuery> spanQuery)
{
  return outerInstance->super->requiresRewrite(spanQuery);
}

shared_ptr<UnifiedHighlighter::LimitedStoredFieldVisitor>
TestUnifiedHighlighterExtensibility::UnifiedHighlighterAnonymousInnerClass::
    newLimitedStoredFieldsVisitor(std::deque<wstring> &fields)
{
  return outerInstance->super->newLimitedStoredFieldsVisitor(fields);
}

deque<std::deque<std::shared_ptr<std::wstring>>>
TestUnifiedHighlighterExtensibility::UnifiedHighlighterAnonymousInnerClass::
    loadFieldValues(std::deque<wstring> &fields,
                    shared_ptr<DocIdSetIterator> docIter,
                    int cacheCharsThreshold) 
{
  return outerInstance->super->loadFieldValues(fields, docIter,
                                               cacheCharsThreshold);
}

shared_ptr<FieldHighlighter>
TestUnifiedHighlighterExtensibility::UnifiedHighlighterAnonymousInnerClass::
    getFieldHighlighter(const wstring &field, shared_ptr<Query> query,
                        shared_ptr<Set<std::shared_ptr<Term>>> allTerms,
                        int maxPassages)
{
  // THIS IS A COPY of the superclass impl; but use CustomFieldHighlighter
  std::deque<std::shared_ptr<BytesRef>> terms =
      UnifiedHighlighter::filterExtractedTerms(getFieldMatcher(field),
                                               allTerms);
  shared_ptr<Set<HighlightFlag>> highlightFlags = getFlags(field);
  shared_ptr<PhraseHelper> phraseHelper =
      getPhraseHelper(field, query, highlightFlags);
  std::deque<std::shared_ptr<CharacterRunAutomaton>> automata =
      getAutomata(field, query, highlightFlags);
  OffsetSource offsetSource =
      getOptimizedOffsetSource(field, terms, phraseHelper, automata);
  return make_shared<CustomFieldHighlighter>(
      field,
      getOffsetStrategy(offsetSource, field, terms, phraseHelper, automata,
                        highlightFlags),
      make_shared<SplittingBreakIterator>(
          getBreakIterator(field), UnifiedHighlighter::MULTIVAL_SEP_CHAR),
      getScorer(field), maxPassages, getMaxNoHighlightPassages(field),
      getFormatter(field));
}

shared_ptr<FieldOffsetStrategy> TestUnifiedHighlighterExtensibility::
    UnifiedHighlighterAnonymousInnerClass::getOffsetStrategy(
        OffsetSource offsetSource, const wstring &field,
        std::deque<std::shared_ptr<BytesRef>> &terms,
        shared_ptr<PhraseHelper> phraseHelper,
        std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata,
        shared_ptr<Set<HighlightFlag>> highlightFlags)
{
  return outerInstance->super->getOffsetStrategy(
      offsetSource, field, terms, phraseHelper, automata, highlightFlags);
}

int TestUnifiedHighlighterExtensibility::UnifiedHighlighterAnonymousInnerClass::
    getMaxLength()
{
  return maxLength;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPassageFormatterExtensibility()
void TestUnifiedHighlighterExtensibility::testPassageFormatterExtensibility()
{
  constexpr any formattedResponse;
  shared_ptr<PassageFormatter> formatter =
      make_shared<PassageFormatterAnonymousInnerClass>(shared_from_this(),
                                                       formattedResponse);
  assertEquals(
      formattedResponse,
      formatter->format(std::deque<std::shared_ptr<Passage>>(0), L""));
}

TestUnifiedHighlighterExtensibility::PassageFormatterAnonymousInnerClass::
    PassageFormatterAnonymousInnerClass(
        shared_ptr<TestUnifiedHighlighterExtensibility> outerInstance,
        any formattedResponse)
{
  this->outerInstance = outerInstance;
  this->formattedResponse = formattedResponse;
}

any TestUnifiedHighlighterExtensibility::PassageFormatterAnonymousInnerClass::
    format(std::deque<std::shared_ptr<Passage>> &passages,
           const wstring &content)
{
  return formattedResponse;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFieldHiglighterExtensibility()
void TestUnifiedHighlighterExtensibility::testFieldHiglighterExtensibility()
{
  const wstring fieldName = L"fieldName";
  shared_ptr<FieldHighlighter> fieldHighlighter =
      make_shared<FieldHighlighterAnonymousInnerClass>(shared_from_this());

  assertEquals(fieldHighlighter->getField(), fieldName);
}

TestUnifiedHighlighterExtensibility::FieldHighlighterAnonymousInnerClass::
    FieldHighlighterAnonymousInnerClass(
        shared_ptr<TestUnifiedHighlighterExtensibility> outerInstance)
    : org::apache::lucene::search::uhighlight::FieldHighlighter(
          fieldName, nullptr, nullptr, nullptr, 1, 1, nullptr)
{
  this->outerInstance = outerInstance;
}

std::deque<std::shared_ptr<Passage>> TestUnifiedHighlighterExtensibility::
    FieldHighlighterAnonymousInnerClass::highlightOffsetsEnums(
        shared_ptr<OffsetsEnum> offsetsEnums) 
{
  return outerInstance->super->highlightOffsetsEnums(offsetsEnums);
}

TestUnifiedHighlighterExtensibility::CustomFieldHighlighter::
    CustomFieldHighlighter(const wstring &field,
                           shared_ptr<FieldOffsetStrategy> fieldOffsetStrategy,
                           shared_ptr<BreakIterator> breakIterator,
                           shared_ptr<PassageScorer> passageScorer,
                           int maxPassages, int maxNoHighlightPassages,
                           shared_ptr<PassageFormatter> passageFormatter)
    : org::apache::lucene::search::uhighlight::FieldHighlighter(
          field, fieldOffsetStrategy, breakIterator, passageScorer, maxPassages,
          maxNoHighlightPassages, passageFormatter)
{
}

any TestUnifiedHighlighterExtensibility::CustomFieldHighlighter::
    highlightFieldForDoc(shared_ptr<IndexReader> reader, int docId,
                         const wstring &content) 
{
  return FieldHighlighter::highlightFieldForDoc(reader, docId, content);
}

std::deque<std::shared_ptr<Passage>> TestUnifiedHighlighterExtensibility::
    CustomFieldHighlighter::highlightOffsetsEnums(
        shared_ptr<OffsetsEnum> offsetsEnums) 
{
  // TEST OffsetsEnums & Passage visibility

  // this code never runs; just for compilation
  shared_ptr<Passage> p;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (org.apache.lucene.search.uhighlight.OffsetsEnum oe = new
  // org.apache.lucene.search.uhighlight.OffsetsEnum.OfPostings(null, null))
  {
    org::apache::lucene::search::uhighlight::OffsetsEnum oe =
        org::apache::lucene::search::uhighlight::OffsetsEnum::OfPostings(
            nullptr, nullptr);
    oe->getTerm();
    oe->nextPosition();
    oe->startOffset();
    oe->endOffset();
    oe->freq();
  }

  p = make_shared<Passage>();
  p->setStartOffset(0);
  p->setEndOffset(9);
  p->addMatch(1, 2, make_shared<BytesRef>(), 1);
  p->reset();
  p->setScore(1);
  //... getters are all exposed; custom PassageFormatter impls uses them

  return FieldHighlighter::highlightOffsetsEnums(offsetsEnums);
}
} // namespace org::apache::lucene::search::uhighlight::visibility