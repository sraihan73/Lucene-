using namespace std;

#include "ScoreCachingWrappingScorer.h"

namespace org::apache::lucene::search
{

ScoreCachingWrappingScorer::ScoreCachingWrappingScorer(
    shared_ptr<Scorer> scorer)
    : FilterScorer(scorer)
{
}

float ScoreCachingWrappingScorer::score() 
{
  int doc = in_->docID();
  if (doc != curDoc) {
    curScore = in_->score();
    curDoc = doc;
  }

  return curScore;
}

shared_ptr<deque<std::shared_ptr<ChildScorer>>>
ScoreCachingWrappingScorer::getChildren()
{
  return Collections::singleton(make_shared<ChildScorer>(in_, L"CACHED"));
}
} // namespace org::apache::lucene::search