using namespace std;

#include "QueryCachingPolicy.h"

namespace org::apache::lucene::search
{

const shared_ptr<QueryCachingPolicy> QueryCachingPolicy::ALWAYS_CACHE =
    make_shared<QueryCachingPolicyAnonymousInnerClass>();

QueryCachingPolicyAnonymousInnerClass::QueryCachingPolicyAnonymousInnerClass()
{
}

void QueryCachingPolicyAnonymousInnerClass::onUse(shared_ptr<Query> query) {}

bool QueryCachingPolicyAnonymousInnerClass::shouldCache(
    shared_ptr<Query> query) 
{
  return true;
}
} // namespace org::apache::lucene::search