using namespace std;

#include "TestCloseableThreadLocal.h"

namespace org::apache::lucene::util
{

const wstring TestCloseableThreadLocal::TEST_VALUE = L"initvaluetest";

void TestCloseableThreadLocal::testInitValue()
{
  shared_ptr<InitValueThreadLocal> tl = make_shared<InitValueThreadLocal>();
  wstring str = any_cast<wstring>(tl->get());
  assertEquals(TEST_VALUE, str);
}

void TestCloseableThreadLocal::testNullValue() 
{
  // Tests that null can be set as a valid value (LUCENE-1805). This
  // previously failed in get().
  shared_ptr<CloseableThreadLocal<any>> ctl =
      make_shared<CloseableThreadLocal<any>>();
  ctl->set(nullptr);
  assertNull(ctl->get());
}

void TestCloseableThreadLocal::testDefaultValueWithoutSetting() throw(
    runtime_error)
{
  // LUCENE-1805: make sure default get returns null,
  // twice in a row
  shared_ptr<CloseableThreadLocal<any>> ctl =
      make_shared<CloseableThreadLocal<any>>();
  assertNull(ctl->get());
}

any TestCloseableThreadLocal::InitValueThreadLocal::initialValue()
{
  return TEST_VALUE;
}
} // namespace org::apache::lucene::util