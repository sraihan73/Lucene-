using namespace std;

#include "TestRuleSetupAndRestoreInstanceEnv.h"

namespace org::apache::lucene::util
{
using BooleanQuery = org::apache::lucene::search::BooleanQuery;

void TestRuleSetupAndRestoreInstanceEnv::before()
{
  savedBoolMaxClauseCount = BooleanQuery::getMaxClauseCount();
}

void TestRuleSetupAndRestoreInstanceEnv::after()
{
  BooleanQuery::setMaxClauseCount(savedBoolMaxClauseCount);
}
} // namespace org::apache::lucene::util