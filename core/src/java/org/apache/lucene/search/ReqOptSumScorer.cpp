using namespace std;

#include "ReqOptSumScorer.h"

namespace org::apache::lucene::search
{

ReqOptSumScorer::ReqOptSumScorer(shared_ptr<Scorer> reqScorer,
                                 shared_ptr<Scorer> optScorer)
    : Scorer(reqScorer->weight), reqScorer(reqScorer), optScorer(optScorer),
      optIterator(optScorer->begin())
{
  assert(reqScorer != nullptr);
  assert(optScorer != nullptr);
}

shared_ptr<TwoPhaseIterator> ReqOptSumScorer::twoPhaseIterator()
{
  return reqScorer->twoPhaseIterator();
}

shared_ptr<DocIdSetIterator> ReqOptSumScorer::iterator()
{
  return reqScorer->begin();
}

int ReqOptSumScorer::docID() { return reqScorer->docID(); }

float ReqOptSumScorer::score() 
{
  // TODO: sum into a double and cast to float if we ever send required clauses
  // to BS1
  int curDoc = reqScorer->docID();
  float score = reqScorer->score();

  int optScorerDoc = optIterator->docID();
  if (optScorerDoc < curDoc) {
    optScorerDoc = optIterator->advance(curDoc);
  }

  if (optScorerDoc == curDoc) {
    score += optScorer->score();
  }

  return score;
}

shared_ptr<deque<std::shared_ptr<ChildScorer>>>
ReqOptSumScorer::getChildren()
{
  deque<std::shared_ptr<ChildScorer>> children =
      deque<std::shared_ptr<ChildScorer>>(2);
  children.push_back(make_shared<ChildScorer>(reqScorer, L"MUST"));
  children.push_back(make_shared<ChildScorer>(optScorer, L"SHOULD"));
  return children;
}
} // namespace org::apache::lucene::search