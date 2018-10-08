using namespace std;

#include "MultiTermQueryConstantScoreWrapper.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BytesRef = org::apache::lucene::util::BytesRef;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;

MultiTermQueryConstantScoreWrapper<Q>::TermAndState::TermAndState(
    shared_ptr<BytesRef> term, shared_ptr<TermState> state, int docFreq,
    int64_t totalTermFreq)
    : term(term), state(state), docFreq(docFreq), totalTermFreq(totalTermFreq)
{
}

MultiTermQueryConstantScoreWrapper<Q>::WeightOrDocIdSet::WeightOrDocIdSet(
    shared_ptr<Weight> weight)
    : weight(Objects::requireNonNull(weight)), set(this->set.reset())
{
}

MultiTermQueryConstantScoreWrapper<Q>::WeightOrDocIdSet::WeightOrDocIdSet(
    shared_ptr<DocIdSet> bitset)
    : weight(this->weight.reset()), set(bitset)
{
}

MultiTermQueryConstantScoreWrapper<Q>::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<MultiTermQueryConstantScoreWrapper<std::shared_ptr<Q>>>
            outerInstance,
        float boost,
        shared_ptr<org::apache::lucene::search::IndexSearcher> searcher,
        bool needsScores)
    : ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
  this->searcher = searcher;
  this->needsScores = needsScores;
}

bool MultiTermQueryConstantScoreWrapper<Q>::
    ConstantScoreWeightAnonymousInnerClass::collectTerms(
        shared_ptr<LeafReaderContext> context, shared_ptr<TermsEnum> termsEnum,
        deque<std::shared_ptr<TermAndState>> &terms) 
{
  constexpr int threshold = min(BOOLEAN_REWRITE_TERM_COUNT_THRESHOLD,
                                BooleanQuery::getMaxClauseCount());
  for (int i = 0; i < threshold; ++i) {
    shared_ptr<BytesRef> *const term = termsEnum->next();
    if (term == nullptr) {
      return true;
    }
    shared_ptr<TermState> state = termsEnum->termState();
    terms.push_back(make_shared<TermAndState>(BytesRef::deepCopyOf(term), state,
                                              termsEnum->docFreq(),
                                              termsEnum->totalTermFreq()));
  }
  return termsEnum->next() == nullptr;
}

shared_ptr<WeightOrDocIdSet>
MultiTermQueryConstantScoreWrapper<Q>::ConstantScoreWeightAnonymousInnerClass::
    rewrite(shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Terms> *const terms =
      context->reader()->terms(outerInstance->query->field);
  if (terms == nullptr) {
    // field does not exist
    return make_shared<WeightOrDocIdSet>(
        std::static_pointer_cast<DocIdSet>(nullptr));
  }

  shared_ptr<TermsEnum> *const termsEnum =
      outerInstance->query->getTermsEnum(terms);
  assert(termsEnum != nullptr);

  shared_ptr<PostingsEnum> docs = nullptr;

  const deque<std::shared_ptr<TermAndState>> collectedTerms =
      deque<std::shared_ptr<TermAndState>>();
  if (collectTerms(context, termsEnum, collectedTerms)) {
    // build a bool query
    shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
    for (auto t : collectedTerms) {
      shared_ptr<TermContext> *const termContext =
          make_shared<TermContext>(searcher->getTopReaderContext());
      termContext->register_(t->state, context->ord, t->docFreq,
                             t->totalTermFreq);
      bq->add(make_shared<TermQuery>(
                  make_shared<Term>(outerInstance->query->field, t->term),
                  termContext),
              Occur::SHOULD);
    }
    shared_ptr<Query> q = make_shared<ConstantScoreQuery>(bq->build());
    shared_ptr<Weight> *const weight =
        searcher->rewrite(q)->createWeight(searcher, needsScores, score());
    return make_shared<WeightOrDocIdSet>(weight);
  }

  // Too many terms: go back to the terms we already collected and start
  // building the bit set
  shared_ptr<DocIdSetBuilder> builder =
      make_shared<DocIdSetBuilder>(context->reader()->maxDoc(), terms);
  if (collectedTerms.empty() == false) {
    shared_ptr<TermsEnum> termsEnum2 = terms->begin();
    for (auto t : collectedTerms) {
      termsEnum2->seekExact(t->term, t->state);
      docs = termsEnum2->postings(docs, PostingsEnum::NONE);
      builder->add(docs);
    }
  }

  // Then keep filling the bit set with remaining terms
  do {
    docs = termsEnum->postings(docs, PostingsEnum::NONE);
    builder->add(docs);
  } while (termsEnum->next() != nullptr);

  return make_shared<WeightOrDocIdSet>(builder->build());
}

shared_ptr<Scorer>
MultiTermQueryConstantScoreWrapper<Q>::ConstantScoreWeightAnonymousInnerClass::
    scorer(shared_ptr<DocIdSet> set) 
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
MultiTermQueryConstantScoreWrapper<Q>::ConstantScoreWeightAnonymousInnerClass::
    bulkScorer(shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<WeightOrDocIdSet> *const weightOrBitSet = rewrite(context);
  if (weightOrBitSet->weight != nullptr) {
    return weightOrBitSet->weight->bulkScorer(context);
  } else {
    shared_ptr<Scorer> *const scorer = scorer(weightOrBitSet->set);
    if (scorer == nullptr) {
      return nullptr;
    }
    return make_shared<DefaultBulkScorer>(scorer);
  }
}

shared_ptr<Matches>
MultiTermQueryConstantScoreWrapper<Q>::ConstantScoreWeightAnonymousInnerClass::
    matches(shared_ptr<LeafReaderContext> context, int doc) 
{
  shared_ptr<Terms> *const terms =
      context->reader()->terms(outerInstance->query->field);
  if (terms == nullptr) {
    return nullptr;
  }
  if (terms->hasPositions() == false) {
    return ConstantScoreWeight::matches(context, doc);
  }
  return Matches::forField(outerInstance->query->field, [&]() {
    DisjunctionMatchesIterator::fromTermsEnum(
        context, doc, outerInstance->query->field,
        outerInstance->query->getTermsEnum(terms));
  });
}

shared_ptr<Scorer>
MultiTermQueryConstantScoreWrapper<Q>::ConstantScoreWeightAnonymousInnerClass::
    scorer(shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<WeightOrDocIdSet> *const weightOrBitSet = rewrite(context);
  if (weightOrBitSet->weight != nullptr) {
    return weightOrBitSet->weight->scorer(context);
  } else {
    return scorer(weightOrBitSet->set);
  }
}

bool MultiTermQueryConstantScoreWrapper<Q>::
    ConstantScoreWeightAnonymousInnerClass::isCacheable(
        shared_ptr<LeafReaderContext> ctx)
{
  return true;
}
} // namespace org::apache::lucene::search