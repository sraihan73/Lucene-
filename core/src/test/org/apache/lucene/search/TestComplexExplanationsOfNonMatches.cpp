using namespace std;

#include "TestComplexExplanationsOfNonMatches.h"

namespace org::apache::lucene::search
{

void TestComplexExplanationsOfNonMatches::qtest(
    shared_ptr<Query> q, std::deque<int> &expDocNrs) 
{
  CheckHits::checkNoMatchExplanations(q, FIELD, searcher, expDocNrs);
}
} // namespace org::apache::lucene::search