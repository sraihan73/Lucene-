using namespace std;

#include "TestSpanExplanationsOfNonMatches.h"

namespace org::apache::lucene::search::spans
{
using Query = org::apache::lucene::search::Query;
using CheckHits = org::apache::lucene::search::CheckHits;

void TestSpanExplanationsOfNonMatches::qtest(
    shared_ptr<Query> q, std::deque<int> &expDocNrs) 
{
  CheckHits::checkNoMatchExplanations(q, FIELD, searcher, expDocNrs);
}
} // namespace org::apache::lucene::search::spans