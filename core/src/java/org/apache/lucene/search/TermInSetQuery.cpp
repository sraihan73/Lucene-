using namespace std;

#include "TermInSetQuery.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using PrefixCodedTerms = org::apache::lucene::index::PrefixCodedTerms;
using TermIterator = org::apache::lucene::index::PrefixCodedTerms::TermIterator;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Accountable = org::apache::lucene::util::Accountable;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

TermInSetQuery::TermInSetQuery(
    const wstring &field,
    shared_ptr<deque<std::shared_ptr<BytesRef>>> terms)
    : field(field), termData(builder::finish()),
      termDataHashCode(termData->hashCode())
{
  std::deque<std::shared_ptr<BytesRef>> sortedTerms =
      terms->toArray(std::deque<std::shared_ptr<BytesRef>>(terms->size()));
  // already sorted if we are a SortedSet with natural order
  bool sorted =
      std::dynamic_pointer_cast<SortedSet>(terms) != nullptr &&
      (std::static_pointer_cast<SortedSet<std::shared_ptr<BytesRef>>>(terms))
              ->comparator() == nullptr;
  if (!sorted) {
    ArrayUtil::timSort(sortedTerms);
  }
  shared_ptr<PrefixCodedTerms::Builder> builder =
      make_shared<PrefixCodedTerms::Builder>();
  shared_ptr<BytesRefBuilder> previous = nullptr;
  for (auto term : sortedTerms) {
    if (previous == nullptr) {
      previous = make_shared<BytesRefBuilder>();
    } else if (previous->get().equals(term)) {
      continue; // deduplicate
    }
    builder->add(field, term);
    previous->copyBytes(term);
  }
}

TermInSetQuery::TermInSetQuery(const wstring &field, deque<BytesRef> &terms)
    : TermInSetQuery(field, Arrays::asList(terms))
{
}

shared_ptr<Query>
TermInSetQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  constexpr int threshold = min(BOOLEAN_REWRITE_TERM_COUNT_THRESHOLD,
                                BooleanQuery::getMaxClauseCount());
  if (termData->size() <= threshold) {
    shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
    shared_ptr<PrefixCodedTerms::TermIterator> iterator = termData->begin();
    for (shared_ptr<BytesRef> term = iterator->next(); term != nullptr;
         term = iterator->next()) {
      bq->add(make_shared<TermQuery>(make_shared<Term>(
                  iterator->field(), BytesRef::deepCopyOf(term))),
              Occur::SHOULD);
    }
    return make_shared<ConstantScoreQuery>(bq->build());
  }
  return Query::rewrite(reader);
}

bool TermInSetQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool TermInSetQuery::equalsTo(shared_ptr<TermInSetQuery> other)
{
  // no need to check 'field' explicitly since it is encoded in 'termData'
  // termData might be heavy to compare so check the hash code first
  return termDataHashCode == other->termDataHashCode &&
         termData->equals(other->termData);
}

int TermInSetQuery::hashCode() { return 31 * classHash() + termDataHashCode; }

shared_ptr<PrefixCodedTerms> TermInSetQuery::getTermData() { return termData; }

wstring TermInSetQuery::toString(const wstring &defaultField)
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  builder->append(field);
  builder->append(L":(");

  shared_ptr<PrefixCodedTerms::TermIterator> iterator = termData->begin();
  bool first = true;
  for (shared_ptr<BytesRef> term = iterator->next(); term != nullptr;
       term = iterator->next()) {
    if (!first) {
      builder->append(L' ');
    }
    first = false;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    builder->append(Term::toString(term));
  }
  builder->append(L')');

  return builder->toString();
}

int64_t TermInSetQuery::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED + termData->ramBytesUsed();
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
TermInSetQuery::getChildResources()
{
  return Collections::emptyList();
}

TermInSetQuery::TermAndState::TermAndState(
    const wstring &field, shared_ptr<TermsEnum> termsEnum) 
    : field(field), termsEnum(termsEnum),
      term(BytesRef::deepCopyOf(termsEnum->term())),
      state(termsEnum->termState()), docFreq(termsEnum->docFreq()),
      totalTermFreq(termsEnum->totalTermFreq())
{
}

TermInSetQuery::WeightOrDocIdSet::WeightOrDocIdSet(shared_ptr<Weight> weight)
    : weight(Objects::requireNonNull(weight)), set(this->set.reset())
{
}

TermInSetQuery::WeightOrDocIdSet::WeightOrDocIdSet(shared_ptr<DocIdSet> bitset)
    : weight(this->weight.reset()), set(bitset)
{
}

shared_ptr<Weight>
TermInSetQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                             bool needsScores, float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(
      shared_from_this(), boost, searcher, needsScores);
}

TermInSetQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<TermInSetQuery> outerInstance, float boost,
        shared_ptr<org::apache::lucene::search::IndexSearcher> searcher,
        bool needsScores)
    : ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
  this->searcher = searcher;
  this->needsScores = needsScores;
}

void TermInSetQuery::ConstantScoreWeightAnonymousInnerClass::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  // no-op
  // This query is for abuse cases when the number of terms is too high to
  // run efficiently as a BooleanQuery. So likewise we hide its terms in
  // order to protect highlighters
}

shared_ptr<Matches>
TermInSetQuery::ConstantScoreWeightAnonymousInnerClass::matches(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  shared_ptr<Terms> terms = context->reader()->terms(outerInstance->field);
  if (terms == nullptr || terms->hasPositions() == false) {
    return outerInstance->super->matches(context, doc);
  }
  return Matches::forField(outerInstance->field, [&]() {
    DisjunctionMatchesIterator::fromTermsEnum(
        context, doc, outerInstance->field, outerInstance->termData->begin());
  });
}

shared_ptr<WeightOrDocIdSet>
TermInSetQuery::ConstantScoreWeightAnonymousInnerClass::rewrite(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<LeafReader> *const reader = context->reader();

  shared_ptr<Terms> terms = reader->terms(outerInstance->field);
  if (terms == nullptr) {
    return nullptr;
  }
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  shared_ptr<PostingsEnum> docs = nullptr;
  shared_ptr<PrefixCodedTerms::TermIterator> iterator =
      outerInstance->termData->begin();

  // We will first try to collect up to 'threshold' terms into 'matchingTerms'
  // if there are two many terms, we will fall back to building the 'builder'
  constexpr int threshold = min(BOOLEAN_REWRITE_TERM_COUNT_THRESHOLD,
                                BooleanQuery::getMaxClauseCount());
  assert((outerInstance->termData->size() > threshold,
          L"Query should have been rewritten"));
  deque<std::shared_ptr<TermAndState>> matchingTerms =
      deque<std::shared_ptr<TermAndState>>(threshold);
  shared_ptr<DocIdSetBuilder> builder = nullptr;

  for (shared_ptr<BytesRef> term = iterator->next(); term != nullptr;
       term = iterator->next()) {
    assert(outerInstance->field == iterator->field());
    if (termsEnum->seekExact(term)) {
      if (matchingTerms.empty()) {
        docs = termsEnum->postings(docs, PostingsEnum::NONE);
        builder->add(docs);
      } else if (matchingTerms.size() < threshold) {
        matchingTerms.push_back(
            make_shared<TermAndState>(outerInstance->field, termsEnum));
      } else {
        assert(matchingTerms.size() == threshold);
        builder = make_shared<DocIdSetBuilder>(reader->maxDoc(), terms);
        docs = termsEnum->postings(docs, PostingsEnum::NONE);
        builder->add(docs);
        for (auto t : matchingTerms) {
          t->termsEnum->seekExact(t->term, t->state);
          docs = t->termsEnum->postings(docs, PostingsEnum::NONE);
          builder->add(docs);
        }
        matchingTerms.clear();
      }
    }
  }
  if (matchingTerms.size() > 0) {
    assert(builder == nullptr);
    shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
    for (auto t : matchingTerms) {
      shared_ptr<TermContext> *const termContext =
          make_shared<TermContext>(searcher->getTopReaderContext());
      termContext->register_(t->state, context->ord, t->docFreq,
                             t->totalTermFreq);
      bq->add(make_shared<TermQuery>(make_shared<Term>(t->field, t->term),
                                     termContext),
              Occur::SHOULD);
    }
    shared_ptr<Query> q = make_shared<ConstantScoreQuery>(bq->build());
    shared_ptr<Weight> *const weight =
        searcher->rewrite(q)->createWeight(searcher, needsScores, score());
    return make_shared<WeightOrDocIdSet>(weight);
  } else {
    assert(builder != nullptr);
    return make_shared<WeightOrDocIdSet>(builder->build());
  }
}

shared_ptr<Scorer>
TermInSetQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<DocIdSet> set) 
{
  if (set == nullptr) {
    return nullptr;
  }
  shared_ptr<DocIdSetIterator> *const disi = set->begin();
  if (disi == nullptr) {
    return nullptr;
  }
  return make_shared<ConstantScoreScorer>(shared_from_this(), score(), disi);
}

shared_ptr<BulkScorer>
TermInSetQuery::ConstantScoreWeightAnonymousInnerClass::bulkScorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<WeightOrDocIdSet> *const weightOrBitSet =
      outerInstance->rewrite(context);
  if (weightOrBitSet == nullptr) {
    return nullptr;
  } else if (weightOrBitSet->weight != nullptr) {
    return weightOrBitSet->weight->bulkScorer(context);
  } else {
    shared_ptr<Scorer> *const scorer = scorer(weightOrBitSet->set);
    if (scorer == nullptr) {
      return nullptr;
    }
    return make_shared<DefaultBulkScorer>(scorer);
  }
}

shared_ptr<Scorer>
TermInSetQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<WeightOrDocIdSet> *const weightOrBitSet =
      outerInstance->rewrite(context);
  if (weightOrBitSet == nullptr) {
    return nullptr;
  } else if (weightOrBitSet->weight != nullptr) {
    return weightOrBitSet->weight->scorer(context);
  } else {
    return scorer(weightOrBitSet->set);
  }
}

bool TermInSetQuery::ConstantScoreWeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  // Only cache instances that have a reasonable size. Otherwise it might cause
  // memory issues with the query cache if most memory ends up being spent on
  // queries rather than doc id sets.
  return outerInstance->ramBytesUsed() <=
         LRUQueryCache::QUERY_DEFAULT_RAM_BYTES_USED;
}
} // namespace org::apache::lucene::search