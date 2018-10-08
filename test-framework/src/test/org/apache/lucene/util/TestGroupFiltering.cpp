using namespace std;

#include "TestGroupFiltering.h"

namespace org::apache::lucene::util
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using com::carrotsearch::randomizedtesting::annotations::TestGroup;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Foo public void testFoo()
void TestGroupFiltering::testFoo() {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Foo @Bar public void testFooBar()
void TestGroupFiltering::testFooBar() {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Bar public void testBar()
void TestGroupFiltering::testBar() {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Jira(bug = "JIRA bug reference") public void testJira()
void TestGroupFiltering::testJira() {}
} // namespace org::apache::lucene::util