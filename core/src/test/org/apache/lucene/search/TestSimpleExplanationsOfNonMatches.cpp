using namespace std;

#include "TestSimpleExplanationsOfNonMatches.h"

namespace org::apache::lucene::search
{

void TestSimpleExplanationsOfNonMatches::qtest(
    shared_ptr<Query> q, std::deque<int> &expDocNrs) 
{
  CheckHits::checkNoMatchExplanations(q, FIELD, searcher, expDocNrs);
}
} // namespace org::apache::lucene::search