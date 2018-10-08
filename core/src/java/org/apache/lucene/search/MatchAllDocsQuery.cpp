using namespace std;

#include "MatchAllDocsQuery.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Bits = org::apache::lucene::util::Bits;

shared_ptr<Weight>
MatchAllDocsQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                bool needsScores, float boost)
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

MatchAllDocsQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<MatchAllDocsQuery> outerInstance, float boost)
    : ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

wstring MatchAllDocsQuery::ConstantScoreWeightAnonymousInnerClass::toString()
{
  return L"weight(" + outerInstance + L")";
}

shared_ptr<Scorer>
MatchAllDocsQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<ConstantScoreScorer>(
      shared_from_this(), score(),
      DocIdSetIterator::all(context->reader()->maxDoc()));
}

bool MatchAllDocsQuery::ConstantScoreWeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

shared_ptr<BulkScorer>
MatchAllDocsQuery::ConstantScoreWeightAnonymousInnerClass::bulkScorer(
    shared_ptr<LeafReaderContext> context) 
{
  constexpr float score = score();
  constexpr int maxDoc = context->reader()->maxDoc();
  return make_shared<BulkScorerAnonymousInnerClass>(shared_from_this(), score,
                                                    maxDoc);
}

MatchAllDocsQuery::ConstantScoreWeightAnonymousInnerClass::
    BulkScorerAnonymousInnerClass::BulkScorerAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        float score, int maxDoc)
{
  this->outerInstance = outerInstance;
  this->score = score;
  this->maxDoc = maxDoc;
}

int MatchAllDocsQuery::ConstantScoreWeightAnonymousInnerClass::
    BulkScorerAnonymousInnerClass::score(shared_ptr<LeafCollector> collector,
                                         shared_ptr<Bits> acceptDocs, int min,
                                         int max) 
{
  max = min(max, maxDoc);
  shared_ptr<FakeScorer> scorer = make_shared<FakeScorer>();
  scorer->score_ = score;
  collector->setScorer(scorer);
  for (int doc = min; doc < max; ++doc) {
    scorer->doc = doc;
    if (acceptDocs == nullptr || acceptDocs->get(doc)) {
      collector->collect(doc);
    }
  }
  return max == maxDoc ? DocIdSetIterator::NO_MORE_DOCS : max;
}

int64_t MatchAllDocsQuery::ConstantScoreWeightAnonymousInnerClass::
    BulkScorerAnonymousInnerClass::cost()
{
  return maxDoc;
}

wstring MatchAllDocsQuery::toString(const wstring &field) { return L"*:*"; }

bool MatchAllDocsQuery::equals(any o) { return sameClassAs(o); }

int MatchAllDocsQuery::hashCode() { return classHash(); }
} // namespace org::apache::lucene::search