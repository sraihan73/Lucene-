using namespace std;

#include "TestConfig.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/utils/Config.h"

namespace org::apache::lucene::benchmark::byTask::utils
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAbsolutePathNamesWindows() throws
// Exception
void TestConfig::testAbsolutePathNamesWindows() 
{
  shared_ptr<Properties> props = make_shared<Properties>();
  props->setProperty(L"work.dir1", L"c:\\temp");
  props->setProperty(L"work.dir2", L"c:/temp");
  shared_ptr<Config> conf = make_shared<Config>(props);
  assertEquals(L"c:\\temp", conf->get(L"work.dir1", L""));
  assertEquals(L"c:/temp", conf->get(L"work.dir2", L""));
}
} // namespace org::apache::lucene::benchmark::byTask::utils