using namespace std;

#include "ConstantScoreWeight.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;

ConstantScoreWeight::ConstantScoreWeight(shared_ptr<Query> query, float score)
    : Weight(query), score(score)
{
}

void ConstantScoreWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  // most constant-score queries don't wrap index terms
  // eg. geo filters, doc values queries, ...
  // override if your constant-score query does wrap terms
}

float ConstantScoreWeight::score() { return score_; }

shared_ptr<Explanation>
ConstantScoreWeight::explain(shared_ptr<LeafReaderContext> context,
                             int doc) 
{
  shared_ptr<Scorer> *const s = scorer(context);
  constexpr bool exists;
  if (s == nullptr) {
    exists = false;
  } else {
    shared_ptr<TwoPhaseIterator> *const twoPhase = s->twoPhaseIterator();
    if (twoPhase == nullptr) {
      exists = s->begin().advance(doc) == doc;
    } else {
      exists =
          twoPhase->approximation()->advance(doc) == doc && twoPhase->matches();
    }
  }

  if (exists) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return Explanation::match(
        score_, getQuery()->toString() +
                    (score_ == 1.0f ? L"" : L"^" + to_wstring(score_)));
  } else {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return Explanation::noMatch(getQuery()->toString() + L" doesn't match id " +
                                to_wstring(doc));
  }
}
} // namespace org::apache::lucene::search