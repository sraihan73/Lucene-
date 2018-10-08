using namespace std;

#include "TestFilterSpans.h"

namespace org::apache::lucene::search::spans
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOverrides() throws Exception
void TestFilterSpans::testOverrides() 
{
  // verify that all methods of Spans are overridden by FilterSpans,
  for (shared_ptr<Method> m : FilterSpans::typeid->getMethods()) {
    if (m->getDeclaringClass() == Spans::typeid) {
      fail(L"method " + m->getName() + L" not overridden!");
    }
  }
}
} // namespace org::apache::lucene::search::spans