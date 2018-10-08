using namespace std;

#include "ToParentBlockJoinQuery.h"

namespace org::apache::lucene::search::join
{
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using FilterWeight = org::apache::lucene::search::FilterWeight;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Matches = org::apache::lucene::search::Matches;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using ScorerSupplier = org::apache::lucene::search::ScorerSupplier;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using BitSet = org::apache::lucene::util::BitSet;

ToParentBlockJoinQuery::ToParentBlockJoinQuery(
    shared_ptr<Query> childQuery, shared_ptr<BitSetProducer> parentsFilter,
    ScoreMode scoreMode)
    : org::apache::lucene::search::Query(), parentsFilter(parentsFilter),
      childQuery(childQuery), scoreMode(scoreMode)
{
}

shared_ptr<Weight>
ToParentBlockJoinQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                     bool needsScores,
                                     float boost) 
{
  return make_shared<BlockJoinWeight>(
      shared_from_this(),
      childQuery->createWeight(searcher, needsScores, boost), parentsFilter,
      needsScores ? scoreMode : ScoreMode::None);
}

shared_ptr<Query> ToParentBlockJoinQuery::getChildQuery() { return childQuery; }

ToParentBlockJoinQuery::BlockJoinWeight::BlockJoinWeight(
    shared_ptr<Query> joinQuery, shared_ptr<Weight> childWeight,
    shared_ptr<BitSetProducer> parentsFilter, ScoreMode scoreMode)
    : org::apache::lucene::search::FilterWeight(joinQuery, childWeight),
      parentsFilter(parentsFilter), scoreMode(scoreMode)
{
}

shared_ptr<Scorer> ToParentBlockJoinQuery::BlockJoinWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<ScorerSupplier> *const scorerSupplier =
      this->scorerSupplier(context);
  if (scorerSupplier == nullptr) {
    return nullptr;
  }
  return scorerSupplier->get(numeric_limits<int64_t>::max());
}

shared_ptr<ScorerSupplier>
ToParentBlockJoinQuery::BlockJoinWeight::scorerSupplier(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<ScorerSupplier> *const childScorerSupplier =
      in_->scorerSupplier(context);
  if (childScorerSupplier == nullptr) {
    return nullptr;
  }

  // NOTE: this does not take accept docs into account, the responsibility
  // to not match deleted docs is on the scorer
  shared_ptr<BitSet> *const parents = parentsFilter->getBitSet(context);
  if (parents == nullptr) {
    // No matches
    return nullptr;
  }

  return make_shared<ScorerSupplierAnonymousInnerClass>(
      shared_from_this(), childScorerSupplier, parents);
}

ToParentBlockJoinQuery::BlockJoinWeight::ScorerSupplierAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass(
        shared_ptr<BlockJoinWeight> outerInstance,
        shared_ptr<ScorerSupplier> childScorerSupplier,
        shared_ptr<BitSet> parents)
{
  this->outerInstance = outerInstance;
  this->childScorerSupplier = childScorerSupplier;
  this->parents = parents;
}

shared_ptr<Scorer>
ToParentBlockJoinQuery::BlockJoinWeight::ScorerSupplierAnonymousInnerClass::get(
    int64_t leadCost) 
{
  return make_shared<BlockJoinScorer>(outerInstance,
                                      childScorerSupplier->get(leadCost),
                                      parents, outerInstance->scoreMode);
}

int64_t ToParentBlockJoinQuery::BlockJoinWeight::
    ScorerSupplierAnonymousInnerClass::cost()
{
  return childScorerSupplier->cost();
}

shared_ptr<Explanation> ToParentBlockJoinQuery::BlockJoinWeight::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  shared_ptr<BlockJoinScorer> scorer =
      std::static_pointer_cast<BlockJoinScorer>(this->scorer(context));
  if (scorer != nullptr && scorer->begin().advance(doc) == doc) {
    return scorer->explain(context, in_);
  }
  return Explanation::noMatch(L"Not a match");
}

shared_ptr<Matches> ToParentBlockJoinQuery::BlockJoinWeight::matches(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  // The default implementation would delegate to the joinQuery's Weight, which
  // matches on children.  We need to match on the parent instead
  shared_ptr<Scorer> scorer = this->scorer(context);
  if (scorer == nullptr) {
    return nullptr;
  }
  shared_ptr<TwoPhaseIterator> *const twoPhase = scorer->twoPhaseIterator();
  if (twoPhase == nullptr) {
    if (scorer->begin().advance(doc) != doc) {
      return nullptr;
    }
  } else {
    if (twoPhase->approximation()->advance(doc) != doc ||
        twoPhase->matches() == false) {
      return nullptr;
    }
  }
  return Matches::MATCH_WITH_NO_TERMS;
}

ToParentBlockJoinQuery::ParentApproximation::ParentApproximation(
    shared_ptr<DocIdSetIterator> childApproximation,
    shared_ptr<BitSet> parentBits)
    : childApproximation(childApproximation), parentBits(parentBits)
{
}

int ToParentBlockJoinQuery::ParentApproximation::docID() { return doc; }

int ToParentBlockJoinQuery::ParentApproximation::nextDoc() 
{
  return advance(doc + 1);
}

int ToParentBlockJoinQuery::ParentApproximation::advance(int target) throw(
    IOException)
{
  if (target >= parentBits->length()) {
    return doc = NO_MORE_DOCS;
  }
  constexpr int firstChildTarget =
      target == 0 ? 0 : parentBits->prevSetBit(target - 1) + 1;
  int childDoc = childApproximation->docID();
  if (childDoc < firstChildTarget) {
    childDoc = childApproximation->advance(firstChildTarget);
  }
  if (childDoc >= parentBits->length() - 1) {
    return doc = NO_MORE_DOCS;
  }
  return doc = parentBits->nextSetBit(childDoc + 1);
}

int64_t ToParentBlockJoinQuery::ParentApproximation::cost()
{
  return childApproximation->cost();
}

ToParentBlockJoinQuery::ParentTwoPhase::ParentTwoPhase(
    shared_ptr<ParentApproximation> parentApproximation,
    shared_ptr<TwoPhaseIterator> childTwoPhase)
    : org::apache::lucene::search::TwoPhaseIterator(parentApproximation),
      parentApproximation(parentApproximation),
      childApproximation(childTwoPhase->approximation()),
      childTwoPhase(childTwoPhase)
{
}

bool ToParentBlockJoinQuery::ParentTwoPhase::matches() 
{
  assert(childApproximation->docID() < parentApproximation->docID());
  do {
    if (childTwoPhase->matches()) {
      return true;
    }
  } while (childApproximation->nextDoc() < parentApproximation->docID());
  return false;
}

float ToParentBlockJoinQuery::ParentTwoPhase::matchCost()
{
  // TODO: how could we compute a match cost?
  return childTwoPhase->matchCost() + 10;
}

ToParentBlockJoinQuery::BlockJoinScorer::BlockJoinScorer(
    shared_ptr<Weight> weight, shared_ptr<Scorer> childScorer,
    shared_ptr<BitSet> parentBits, ScoreMode scoreMode)
    : org::apache::lucene::search::Scorer(weight), childScorer(childScorer),
      parentBits(parentBits), scoreMode(scoreMode),
      childTwoPhase(childScorer->twoPhaseIterator())
{
  // System.out.println("Q.init firstChildDoc=" + firstChildDoc);
  if (childTwoPhase == nullptr) {
    childApproximation = childScorer->begin();
    parentApproximation =
        make_shared<ParentApproximation>(childApproximation, parentBits);
    parentTwoPhase.reset();
  } else {
    childApproximation = childTwoPhase->approximation();
    parentApproximation = make_shared<ParentApproximation>(
        childTwoPhase->approximation(), parentBits);
    parentTwoPhase =
        make_shared<ParentTwoPhase>(parentApproximation, childTwoPhase);
  }
}

shared_ptr<deque<std::shared_ptr<Scorer::ChildScorer>>>
ToParentBlockJoinQuery::BlockJoinScorer::getChildren()
{
  return Collections::singleton(
      make_shared<Scorer::ChildScorer>(childScorer, L"BLOCK_JOIN"));
}

shared_ptr<DocIdSetIterator> ToParentBlockJoinQuery::BlockJoinScorer::iterator()
{
  if (parentTwoPhase == nullptr) {
    // the approximation is exact
    return parentApproximation;
  } else {
    return TwoPhaseIterator::asDocIdSetIterator(parentTwoPhase);
  }
}

shared_ptr<TwoPhaseIterator>
ToParentBlockJoinQuery::BlockJoinScorer::twoPhaseIterator()
{
  return parentTwoPhase;
}

int ToParentBlockJoinQuery::BlockJoinScorer::docID()
{
  return parentApproximation->docID();
}

float ToParentBlockJoinQuery::BlockJoinScorer::score() 
{
  setScoreAndFreq();
  return score_;
}

void ToParentBlockJoinQuery::BlockJoinScorer::setScoreAndFreq() throw(
    IOException)
{
  if (childApproximation->docID() >= parentApproximation->docID()) {
    return;
  }
  double score = scoreMode == ScoreMode::None ? 0 : childScorer->score();
  int freq = 1;
  while (childApproximation->nextDoc() < parentApproximation->docID()) {
    if (childTwoPhase == nullptr || childTwoPhase->matches()) {
      constexpr float childScore = childScorer->score();
      freq += 1;
      switch (scoreMode) {
      case org::apache::lucene::search::join::ScoreMode::Total:
      case org::apache::lucene::search::join::ScoreMode::Avg:
        score += childScore;
        break;
      case org::apache::lucene::search::join::ScoreMode::Min:
        score = min(score, childScore);
        break;
      case org::apache::lucene::search::join::ScoreMode::Max:
        score = max(score, childScore);
        break;
      case org::apache::lucene::search::join::ScoreMode::None:
        break;
      default:
        throw make_shared<AssertionError>();
      }
    }
  }
  if (childApproximation->docID() == parentApproximation->docID() &&
      (childTwoPhase == nullptr || childTwoPhase->matches())) {
    throw make_shared<IllegalStateException>(
        wstring(L"Child query must not match same docs with parent filter. ") +
        L"Combine them as must clauses (+) to find a problem doc. " +
        L"docId=" + to_wstring(parentApproximation->docID()) + L", " +
        childScorer->getClass());
  }
  if (scoreMode == ScoreMode::Avg) {
    score /= freq;
  }
  this->score_ = static_cast<float>(score);
}

shared_ptr<Explanation> ToParentBlockJoinQuery::BlockJoinScorer::explain(
    shared_ptr<LeafReaderContext> context,
    shared_ptr<Weight> childWeight) 
{
  int prevParentDoc = parentBits->prevSetBit(parentApproximation->docID() - 1);
  int start = context->docBase + prevParentDoc +
              1; // +1 b/c prevParentDoc is previous parent doc
  int end = context->docBase + parentApproximation->docID() -
            1; // -1 b/c parentDoc is parent doc

  shared_ptr<Explanation> bestChild = nullptr;
  int matches = 0;
  for (int childDoc = start; childDoc <= end; childDoc++) {
    shared_ptr<Explanation> child =
        childWeight->explain(context, childDoc - context->docBase);
    if (child->isMatch()) {
      matches++;
      if (bestChild == nullptr || child->getValue() > bestChild->getValue()) {
        bestChild = child;
      }
    }
  }

  return Explanation::match(
      score(),
      wstring::format(
          Locale::ROOT,
          L"Score based on %d child docs in range from %d to %d, best match:",
          matches, start, end),
      bestChild);
}

shared_ptr<Query> ToParentBlockJoinQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> *const childRewrite = childQuery->rewrite(reader);
  if (childRewrite != childQuery) {
    return make_shared<ToParentBlockJoinQuery>(childRewrite, parentsFilter,
                                               scoreMode);
  } else {
    return Query::rewrite(reader);
  }
}

wstring ToParentBlockJoinQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"ToParentBlockJoinQuery (" + childQuery->toString() + L")";
}

bool ToParentBlockJoinQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool ToParentBlockJoinQuery::equalsTo(shared_ptr<ToParentBlockJoinQuery> other)
{
  return childQuery->equals(other->childQuery) &&
         parentsFilter->equals(other->parentsFilter) &&
         scoreMode == other->scoreMode;
}

int ToParentBlockJoinQuery::hashCode()
{
  constexpr int prime = 31;
  int hash = classHash();
  hash = prime * hash + childQuery->hashCode();
  hash = prime * hash + scoreMode.hashCode();
  hash = prime * hash + parentsFilter->hashCode();
  return hash;
}
} // namespace org::apache::lucene::search::join