using namespace std;

#include "PhraseHelper.h"

namespace org::apache::lucene::search::uhighlight
{
using FieldInfos = org::apache::lucene::index::FieldInfos;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using Query = org::apache::lucene::search::Query;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using WeightedSpanTerm =
    org::apache::lucene::search::highlight::WeightedSpanTerm;
using WeightedSpanTermExtractor =
    org::apache::lucene::search::highlight::WeightedSpanTermExtractor;
using SpanCollector = org::apache::lucene::search::spans::SpanCollector;
using SpanMultiTermQueryWrapper =
    org::apache::lucene::search::spans::SpanMultiTermQueryWrapper;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanScorer = org::apache::lucene::search::spans::SpanScorer;
using Spans = org::apache::lucene::search::spans::Spans;
using BytesRef = org::apache::lucene::util::BytesRef;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
const shared_ptr<PhraseHelper> PhraseHelper::NONE = make_shared<PhraseHelper>(
    make_shared<org::apache::lucene::search::MatchAllDocsQuery>(), L"_ignored_",
    [&](s) { false; }, [&](any spanQuery) { nullptr; },
    [&](any query) { nullptr; }, true);

PhraseHelper::PhraseHelper(
    shared_ptr<Query> query, const wstring &field,
    function<bool(const wstring &)> &fieldMatcher,
    function<bool(SpanQuery *)> &rewriteQueryPred,
    function<deque<std::shared_ptr<Query>>> &preExtractRewriteFunction,
    bool ignoreQueriesNeedingRewrite)
    : fieldName(field), positionInsensitiveTerms(unordered_set<>()),
      spanQueries(unordered_set<>()), willRewrite(mustRewriteHolder[0]),
      fieldMatcher(fieldMatcher)
{
  // filter terms to those we want

  // TODO Have toSpanQuery(query) Function as an extension point for those with
  // custom Query impls

  std::deque<bool> mustRewriteHolder = {
      false}; // bool wrapped in 1-ary array so it's mutable from inner class

  // When we call Weight.extractTerms, we do it on clauses that are NOT position
  // sensitive. We only want the to track a Set of bytes for the Term, not Term
  // class with field part.
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  shared_ptr<Set<std::shared_ptr<Term>>> extractPosInsensitiveTermsTarget =
      make_shared<TreeSetAnonymousInnerClass>(shared_from_this(), fieldMatcher);

  // For TermQueries or other position insensitive queries, collect the Terms.
  // For other Query types, WSTE will convert to an equivalent SpanQuery.  NOT
  // extracting position spans here.
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  make_shared<WeightedSpanTermExtractorAnonymousInnerClass>(
      shared_from_this(), field, query, fieldMatcher, rewriteQueryPred,
      preExtractRewriteFunction, ignoreQueriesNeedingRewrite, mustRewriteHolder,
      extractPosInsensitiveTermsTarget); // calling the constructor triggered
                                         // the extraction/visiting we want::
                                         // Hacky; yes.
}

PhraseHelper::TreeSetAnonymousInnerClass::TreeSetAnonymousInnerClass(
    shared_ptr<PhraseHelper> outerInstance,
    function<bool(const wstring &)> &fieldMatcher)
{
  this->outerInstance = outerInstance;
  this->fieldMatcher = fieldMatcher;
}

bool PhraseHelper::TreeSetAnonymousInnerClass::add(shared_ptr<Term> term)
{
  // don't call super.add; we don't actually use the superclass
  if (fieldMatcher(term->field())) {
    return outerInstance->positionInsensitiveTerms->add(term->bytes());
  } else {
    return false;
  }
}

PhraseHelper::WeightedSpanTermExtractorAnonymousInnerClass::
    WeightedSpanTermExtractorAnonymousInnerClass(
        shared_ptr<PhraseHelper> outerInstance, const wstring &field,
        shared_ptr<Query> query, function<bool(const wstring &)> &fieldMatcher,
        function<bool(SpanQuery *)> &rewriteQueryPred,
        function<deque<std::shared_ptr<Query>>> &preExtractRewriteFunction,
        bool ignoreQueriesNeedingRewrite, deque<bool> &mustRewriteHolder,
        shared_ptr<Set<std::shared_ptr<Term>>> extractPosInsensitiveTermsTarget)
    : org::apache::lucene::search::highlight::WeightedSpanTermExtractor(field)
{
  this->outerInstance = outerInstance;
  this->query = query;
  this->fieldMatcher = fieldMatcher;
  this->rewriteQueryPred = rewriteQueryPred;
  this->preExtractRewriteFunction = preExtractRewriteFunction;
  this->ignoreQueriesNeedingRewrite = ignoreQueriesNeedingRewrite;
  this->mustRewriteHolder = mustRewriteHolder;
  this->extractPosInsensitiveTermsTarget = extractPosInsensitiveTermsTarget;

  setExpandMultiTermQuery(
      true); // necessary for mustRewriteQuery(spanQuery) to work.

  try {
    extract(query, 1.0f, nullptr); // null because we won't actually extract
                                   // right now; we're not collecting
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

void PhraseHelper::WeightedSpanTermExtractorAnonymousInnerClass::extract(
    shared_ptr<Query> query, float boost,
    unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>>
        &terms) 
{
  shared_ptr<deque<std::shared_ptr<Query>>> newQueriesToExtract =
      preExtractRewriteFunction(query);
  if (newQueriesToExtract != nullptr) {
    for (auto newQuery : newQueriesToExtract) {
      extract(newQuery, boost, terms);
    }
  } else {
    WeightedSpanTermExtractor::extract(query, boost, terms);
  }
}

bool PhraseHelper::WeightedSpanTermExtractorAnonymousInnerClass::
    isQueryUnsupported(type_info clazz)
{
  if (clazz.isAssignableFrom(MultiTermQuery::typeid)) {
    return true; // We do MTQ processing separately in
                 // MultiTermHighlighting.java
  }
  return true; // TODO set to false and provide a hook to customize certain
               // queries.
}

void PhraseHelper::WeightedSpanTermExtractorAnonymousInnerClass::
    extractWeightedTerms(
        unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>> &terms,
        shared_ptr<Query> query, float boost) 
{
  query->createWeight(UnifiedHighlighter::EMPTY_INDEXSEARCHER, false, boost)
      ->extractTerms(extractPosInsensitiveTermsTarget);
}

void PhraseHelper::WeightedSpanTermExtractorAnonymousInnerClass::
    extractWeightedSpanTerms(
        unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>> &terms,
        shared_ptr<SpanQuery> spanQuery, float boost) 
{
  // if this span query isn't for this field, skip it.
  shared_ptr<Set<wstring>> fieldNameSet =
      unordered_set<wstring>(); // TODO reuse.  note: almost always size 1
  collectSpanQueryFields(spanQuery, fieldNameSet);
  for (auto spanField : fieldNameSet) {
    if (!fieldMatcher(spanField)) {
      return;
    }
  }

  bool mustRewriteQuery = mustRewriteQuery(spanQuery);
  if (ignoreQueriesNeedingRewrite && mustRewriteQuery) {
    return; // ignore this query
  }
  mustRewriteHolder[0] |= mustRewriteQuery;

  outerInstance->spanQueries->add(spanQuery);
}

bool PhraseHelper::WeightedSpanTermExtractorAnonymousInnerClass::
    mustRewriteQuery(shared_ptr<SpanQuery> spanQuery)
{
  optional<bool> rewriteQ = rewriteQueryPred(spanQuery); // allow to override
  return rewriteQ ? rewriteQ
                  : WeightedSpanTermExtractor::mustRewriteQuery(spanQuery);
}

shared_ptr<Set<std::shared_ptr<SpanQuery>>> PhraseHelper::getSpanQueries()
{
  return spanQueries;
}

bool PhraseHelper::hasPositionSensitivity()
{
  return spanQueries->isEmpty() == false;
}

bool PhraseHelper::willRewrite() { return willRewrite_; }

std::deque<std::shared_ptr<BytesRef>>
PhraseHelper::getAllPositionInsensitiveTerms()
{
  std::deque<std::shared_ptr<BytesRef>> result =
      positionInsensitiveTerms->toArray(std::deque<std::shared_ptr<BytesRef>>(
          positionInsensitiveTerms->size()));
  Arrays::sort(result);
  return result;
}

void PhraseHelper::createOffsetsEnumsForSpans(
    shared_ptr<LeafReader> leafReader, int docId,
    deque<std::shared_ptr<OffsetsEnum>> &results) 
{
  leafReader = make_shared<SingleFieldWithOffsetsFilterLeafReader>(leafReader,
                                                                   fieldName);
  // TODO avoid searcher and do what it does to rewrite & get weight?
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(leafReader);
  searcher->setQueryCache(nullptr);

  // for each SpanQuery, grab it's Spans and put it into a PriorityQueue
  shared_ptr<PriorityQueue<std::shared_ptr<Spans>>> spansPriorityQueue =
      make_shared<PriorityQueueAnonymousInnerClass>(shared_from_this(),
                                                    spanQueries->size());
  for (auto query : spanQueries) {
    shared_ptr<Weight> weight =
        searcher->createWeight(searcher->rewrite(query), false, 1);
    shared_ptr<Scorer> scorer = weight->scorer(leafReader->getContext());
    if (scorer == nullptr) {
      continue;
    }
    shared_ptr<TwoPhaseIterator> twoPhaseIterator = scorer->twoPhaseIterator();
    if (twoPhaseIterator != nullptr) {
      if (twoPhaseIterator->approximation()->advance(docId) != docId ||
          !twoPhaseIterator->matches()) {
        continue;
      }
    } else if (scorer->begin().advance(docId) !=
               docId) { // preposition, and return doing nothing if find none
      continue;
    }

    shared_ptr<Spans> spans =
        (std::static_pointer_cast<SpanScorer>(scorer))->getSpans();
    assert(spans->docID() == docId);
    if (spans->nextStartPosition() != Spans::NO_MORE_POSITIONS) {
      spansPriorityQueue->push_back(spans);
    }
  }

  // Iterate the Spans in the PriorityQueue, collecting as we go.  By using a
  // PriorityQueue ordered by position,
  //   the underlying offsets in our collector will be mostly appended to the
  //   end of arrays (efficient).
  // note: alternatively it'd interesting if we produced one OffsetsEnum that
  // internally advanced
  //   this PriorityQueue when nextPosition is called; it would cap what we have
  //   to cache for large docs and exiting early (due to maxLen) is easy. But at
  //   least we have an accurate "freq" and it shouldn't be too much data to
  //   collect.  Even SpanScorer navigates the spans fully to compute a good
  //   freq (and thus score)!
  shared_ptr<OffsetSpanCollector> spanCollector =
      make_shared<OffsetSpanCollector>(shared_from_this());
  while (spansPriorityQueue->size() > 0) {
    shared_ptr<Spans> spans = spansPriorityQueue->top();
    // TODO limit to a capped endOffset length somehow so we can break this loop
    // early
    spans->collect(spanCollector);

    if (spans->nextStartPosition() == Spans::NO_MORE_POSITIONS) {
      spansPriorityQueue->pop();
    } else {
      spansPriorityQueue->updateTop();
    }
  }
  results.addAll(spanCollector->termToOffsetsEnums.values());
}

PhraseHelper::PriorityQueueAnonymousInnerClass::
    PriorityQueueAnonymousInnerClass(shared_ptr<PhraseHelper> outerInstance,
                                     shared_ptr<UnknownType> size)
    : org::apache::lucene::util::PriorityQueue<
          org::apache::lucene::search::spans::Spans>(size)
{
  this->outerInstance = outerInstance;
}

bool PhraseHelper::PriorityQueueAnonymousInnerClass::lessThan(
    shared_ptr<Spans> a, shared_ptr<Spans> b)
{
  return a->startPosition() <= b->startPosition();
}

PhraseHelper::SingleFieldWithOffsetsFilterLeafReader::
    SingleFieldWithOffsetsFilterLeafReader(shared_ptr<LeafReader> in_,
                                           const wstring &fieldName)
    : org::apache::lucene::index::FilterLeafReader(in_), fieldName(fieldName)
{
}

shared_ptr<FieldInfos>
PhraseHelper::SingleFieldWithOffsetsFilterLeafReader::getFieldInfos()
{
  throw make_shared<UnsupportedOperationException>(); // TODO merge them
}

shared_ptr<Terms> PhraseHelper::SingleFieldWithOffsetsFilterLeafReader::terms(
    const wstring &field) 
{
  // ensure the underlying PostingsEnum returns offsets.  It's sad we have to do
  // this to use the SpanCollector.
  return make_shared<FilterTermsAnonymousInnerClass>(
      shared_from_this(), FilterLeafReader::terms(fieldName));
}

PhraseHelper::SingleFieldWithOffsetsFilterLeafReader::
    FilterTermsAnonymousInnerClass::FilterTermsAnonymousInnerClass(
        shared_ptr<SingleFieldWithOffsetsFilterLeafReader> outerInstance,
        shared_ptr<Terms> terms)
    : FilterTerms(terms)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TermsEnum> PhraseHelper::SingleFieldWithOffsetsFilterLeafReader::
    FilterTermsAnonymousInnerClass::iterator() 
{
  return make_shared<FilterTermsEnumAnonymousInnerClass>(
      shared_from_this(), outerInstance->in_->begin());
}

PhraseHelper::SingleFieldWithOffsetsFilterLeafReader::
    FilterTermsAnonymousInnerClass::FilterTermsEnumAnonymousInnerClass::
        FilterTermsEnumAnonymousInnerClass(
            shared_ptr<FilterTermsAnonymousInnerClass> outerInstance,
            shared_ptr<UnknownType> iterator)
    : FilterTermsEnum(iterator)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PostingsEnum> PhraseHelper::SingleFieldWithOffsetsFilterLeafReader::
    FilterTermsAnonymousInnerClass::FilterTermsEnumAnonymousInnerClass::
        postings(shared_ptr<PostingsEnum> reuse, int flags) 
{
  return outerInstance->outerInstance.super.postings(
      reuse, flags | PostingsEnum::OFFSETS);
}

shared_ptr<NumericDocValues>
PhraseHelper::SingleFieldWithOffsetsFilterLeafReader::getNormValues(
    const wstring &field) 
{
  return FilterLeafReader::getNormValues(fieldName);
}

shared_ptr<CacheHelper>
PhraseHelper::SingleFieldWithOffsetsFilterLeafReader::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<CacheHelper>
PhraseHelper::SingleFieldWithOffsetsFilterLeafReader::getReaderCacheHelper()
{
  return nullptr;
}

PhraseHelper::OffsetSpanCollector::OffsetSpanCollector(
    shared_ptr<PhraseHelper> outerInstance)
    : outerInstance(outerInstance)
{
}

void PhraseHelper::OffsetSpanCollector::collectLeaf(
    shared_ptr<PostingsEnum> postings, int position,
    shared_ptr<Term> term) 
{
  if (!outerInstance->fieldMatcher->test(term->field())) {
    return;
  }

  shared_ptr<SpanCollectedOffsetsEnum> offsetsEnum =
      termToOffsetsEnums[term->bytes()];
  if (offsetsEnum == nullptr) {
    // If it's pos insensitive we handle it outside of PhraseHelper.
    // term.field() is from the Query.
    if (outerInstance->positionInsensitiveTerms->contains(term->bytes())) {
      return;
    }
    offsetsEnum =
        make_shared<SpanCollectedOffsetsEnum>(term->bytes(), postings->freq());
    termToOffsetsEnums.emplace(term->bytes(), offsetsEnum);
  }
  offsetsEnum->add(postings->startOffset(), postings->endOffset());
}

void PhraseHelper::OffsetSpanCollector::reset()
{ // called when at a new position.  We don't care.
}

PhraseHelper::SpanCollectedOffsetsEnum::SpanCollectedOffsetsEnum(
    shared_ptr<BytesRef> term, int postingsFreq)
    : term(term),
      startOffsets(std::deque<int>(postingsFreq)) / *hopefully not wasteful
      ? At least we needn't resize it. */, endOffsets(std::deque<int>(postingsFreq))
{
}

void PhraseHelper::SpanCollectedOffsetsEnum::add(int startOffset, int endOffset)
{
  assert((enumIdx == -1, L"bad state"));

  // loop backwards since we expect a match at the end or close to it.  We
  // expect O(1) not O(N).
  int pairIdx = numPairs - 1;
  for (; pairIdx >= 0; pairIdx--) {
    int iStartOffset = startOffsets[pairIdx];
    int iEndOffset = endOffsets[pairIdx];
    int cmp = Integer::compare(iStartOffset, startOffset);
    if (cmp == 0) {
      cmp = Integer::compare(iEndOffset, endOffset);
    }
    if (cmp == 0) {
      return; // we already have this offset-pair for this term
    } else if (cmp < 0) {
      break; // we will insert offsetPair to the right of pairIdx
    }
  }
  // pairIdx is now one position to the left of where we insert the new pair
  // shift right any pairs by one to make room
  constexpr int shiftLen = numPairs - (pairIdx + 1);
  if (shiftLen > 0) {
    System::arraycopy(startOffsets, pairIdx + 2, startOffsets, pairIdx + 3,
                      shiftLen);
    System::arraycopy(endOffsets, pairIdx + 2, endOffsets, pairIdx + 3,
                      shiftLen);
  }
  // now we can place the offset pair
  startOffsets[pairIdx + 1] = startOffset;
  endOffsets[pairIdx + 1] = endOffset;
  numPairs++;
}

bool PhraseHelper::SpanCollectedOffsetsEnum::nextPosition() 
{
  return ++enumIdx < numPairs;
}

int PhraseHelper::SpanCollectedOffsetsEnum::freq() 
{
  return numPairs;
}

shared_ptr<BytesRef>
PhraseHelper::SpanCollectedOffsetsEnum::getTerm() 
{
  return term;
}

int PhraseHelper::SpanCollectedOffsetsEnum::startOffset() 
{
  return startOffsets[enumIdx];
}

int PhraseHelper::SpanCollectedOffsetsEnum::endOffset() 
{
  return endOffsets[enumIdx];
}
} // namespace org::apache::lucene::search::uhighlight