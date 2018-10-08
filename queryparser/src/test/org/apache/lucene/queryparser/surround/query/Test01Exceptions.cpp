using namespace std;

#include "Test01Exceptions.h"

namespace org::apache::lucene::queryparser::surround::query
{
using junit::framework::TestSuite;
using junit::textui::TestRunner;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void Test01Exceptions::main(std::deque<wstring> &args)
{
  TestRunner::run(make_shared<TestSuite>(Test01Exceptions::typeid));
}

void Test01Exceptions::test01Exceptions() 
{
  wstring m = ExceptionQueryTst::getFailQueries(exceptionQueries, verbose);
  if (m.length() > 0) {
    fail(L"No ParseException for:\n" + m);
  }
}
} // namespace org::apache::lucene::queryparser::surround::query