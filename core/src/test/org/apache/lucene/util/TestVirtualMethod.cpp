using namespace std;

#include "TestVirtualMethod.h"

namespace org::apache::lucene::util
{

const shared_ptr<VirtualMethod<std::shared_ptr<TestVirtualMethod>>>
    TestVirtualMethod::publicTestMethod =
        make_shared<VirtualMethod<std::shared_ptr<TestVirtualMethod>>>(
            TestVirtualMethod::typeid, L"publicTest", wstring::typeid);
const shared_ptr<VirtualMethod<std::shared_ptr<TestVirtualMethod>>>
    TestVirtualMethod::protectedTestMethod =
        make_shared<VirtualMethod<std::shared_ptr<TestVirtualMethod>>>(
            TestVirtualMethod::typeid, L"protectedTest", int ::typeid);

void TestVirtualMethod::publicTest(const wstring &test) {}

void TestVirtualMethod::protectedTest(int test) {}

void TestVirtualMethod::TestClass1::publicTest(const wstring &test) {}

void TestVirtualMethod::TestClass1::protectedTest(int test) {}

void TestVirtualMethod::TestClass2::protectedTest(int test) {}

void TestVirtualMethod::TestClass3::publicTest(const wstring &test) {}

void TestVirtualMethod::testGeneral()
{
  assertEquals(0,
               publicTestMethod->getImplementationDistance(this->getClass()));
  assertEquals(1,
               publicTestMethod->getImplementationDistance(TestClass1::typeid));
  assertEquals(1,
               publicTestMethod->getImplementationDistance(TestClass2::typeid));
  assertEquals(3,
               publicTestMethod->getImplementationDistance(TestClass3::typeid));
  assertFalse(publicTestMethod->isOverriddenAsOf(TestClass4::typeid));
  assertFalse(publicTestMethod->isOverriddenAsOf(TestClass5::typeid));

  assertEquals(
      0, protectedTestMethod->getImplementationDistance(this->getClass()));
  assertEquals(
      1, protectedTestMethod->getImplementationDistance(TestClass1::typeid));
  assertEquals(
      2, protectedTestMethod->getImplementationDistance(TestClass2::typeid));
  assertEquals(
      2, protectedTestMethod->getImplementationDistance(TestClass3::typeid));
  assertFalse(protectedTestMethod->isOverriddenAsOf(TestClass4::typeid));
  assertFalse(protectedTestMethod->isOverriddenAsOf(TestClass5::typeid));

  assertTrue(VirtualMethod::compareImplementationDistance(
                 TestClass3::typeid, publicTestMethod, protectedTestMethod) >
             0);
  assertEquals(0,
               VirtualMethod::compareImplementationDistance(
                   TestClass5::typeid, publicTestMethod, protectedTestMethod));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) public void
// testExceptions()
void TestVirtualMethod::testExceptions()
{
  // LuceneTestCase is not a subclass and can never override publicTest(std::wstring)
  expectThrows(invalid_argument::typeid, [&]() {
    publicTestMethod->getImplementationDistance(
        static_cast<type_info>(LuceneTestCase::typeid));
  });

  // Method bogus() does not exist, so IAE should be thrown
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<VirtualMethod<>>(TestVirtualMethod::typeid, L"bogus");
  });

  // Method publicTest(std::wstring) is not declared in TestClass2, so IAE should be
  // thrown
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<VirtualMethod<>>(TestClass2::typeid, L"publicTest",
                                 wstring::typeid);
  });

  // try to create a second instance of the same baseClass / method combination
  expectThrows(UnsupportedOperationException::typeid, [&]() {
    make_shared<VirtualMethod<>>(TestVirtualMethod::typeid, L"publicTest",
                                 wstring::typeid);
  });
}
} // namespace org::apache::lucene::util