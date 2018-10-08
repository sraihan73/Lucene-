using namespace std;

#include "TestRamUsageEstimator.h"

namespace org::apache::lucene::util
{
using namespace org::apache::lucene::util;
//    import static org.apache.lucene.util.RamUsageEstimator.*;
//    import static org.apache.lucene.util.RamUsageTester.sizeOf;

void TestRamUsageEstimator::testSanity()
{
  assertTrue(RamUsageEstimator::sizeOf(L"test string") >
             RamUsageEstimator::shallowSizeOfInstance(wstring::typeid));

  shared_ptr<Holder> holder = make_shared<Holder>();
  holder->holder = make_shared<Holder>(L"string2", 5000LL);
  assertTrue(RamUsageEstimator::sizeOf(holder) >
             RamUsageEstimator::shallowSizeOfInstance(Holder::typeid));
  assertTrue(RamUsageEstimator::sizeOf(holder) >
             RamUsageEstimator::sizeOf(holder->holder));

  assertTrue(RamUsageEstimator::shallowSizeOfInstance(HolderSubclass::typeid) >=
             RamUsageEstimator::shallowSizeOfInstance(Holder::typeid));
  assertTrue(RamUsageEstimator::shallowSizeOfInstance(Holder::typeid) ==
             RamUsageEstimator::shallowSizeOfInstance(HolderSubclass2::typeid));

  std::deque<wstring> strings = {L"test string", L"hollow", L"catchmaster"};
  assertTrue(RamUsageEstimator::sizeOf(strings) >
             RamUsageEstimator::shallowSizeOf(strings));
}

void TestRamUsageEstimator::testStaticOverloads()
{
  shared_ptr<Random> rnd = random();
  {
    std::deque<char> array_(rnd->nextInt(1024));
    assertEquals(RamUsageEstimator::sizeOf(array_),
                 RamUsageEstimator::sizeOf(static_cast<any>(array_)));
  }

  {
    std::deque<bool> array_(rnd->nextInt(1024));
    assertEquals(RamUsageEstimator::sizeOf(array_),
                 RamUsageEstimator::sizeOf(static_cast<any>(array_)));
  }

  {
    std::deque<wchar_t> array_(rnd->nextInt(1024));
    assertEquals(RamUsageEstimator::sizeOf(array_),
                 RamUsageEstimator::sizeOf(static_cast<any>(array_)));
  }

  {
    std::deque<short> array_(rnd->nextInt(1024));
    assertEquals(RamUsageEstimator::sizeOf(array_),
                 RamUsageEstimator::sizeOf(static_cast<any>(array_)));
  }

  {
    std::deque<int> array_(rnd->nextInt(1024));
    assertEquals(RamUsageEstimator::sizeOf(array_),
                 RamUsageEstimator::sizeOf(static_cast<any>(array_)));
  }

  {
    std::deque<float> array_(rnd->nextInt(1024));
    assertEquals(RamUsageEstimator::sizeOf(array_),
                 RamUsageEstimator::sizeOf(static_cast<any>(array_)));
  }

  {
    std::deque<int64_t> array_(rnd->nextInt(1024));
    assertEquals(RamUsageEstimator::sizeOf(array_),
                 RamUsageEstimator::sizeOf(static_cast<any>(array_)));
  }

  {
    std::deque<double> array_(rnd->nextInt(1024));
    assertEquals(RamUsageEstimator::sizeOf(array_),
                 RamUsageEstimator::sizeOf(static_cast<any>(array_)));
  }
}

void TestRamUsageEstimator::testReferenceSize()
{
  assertTrue(RamUsageEstimator::NUM_BYTES_OBJECT_REF == 4 ||
             RamUsageEstimator::NUM_BYTES_OBJECT_REF == 8);
  if (Constants::JRE_IS_64BIT) {
    assertEquals(L"For 64 bit JVMs, reference size must be 8, unless "
                 L"compressed references are enabled",
                 RamUsageEstimator::COMPRESSED_REFS_ENABLED ? 4 : 8,
                 RamUsageEstimator::NUM_BYTES_OBJECT_REF);
  } else {
    assertEquals(L"For 32bit JVMs, reference size must always be 4", 4,
                 RamUsageEstimator::NUM_BYTES_OBJECT_REF);
    assertFalse(L"For 32bit JVMs, compressed references can never be enabled",
                RamUsageEstimator::COMPRESSED_REFS_ENABLED);
  }
}

void TestRamUsageEstimator::testHotspotBean()
{
  assumeTrue(L"testHotspotBean only works on 64bit JVMs.",
             Constants::JRE_IS_64BIT);
  try {
    type_info::forName(RamUsageEstimator::MANAGEMENT_FACTORY_CLASS);
  } catch (const ClassNotFoundException &e) {
    assumeNoException(
        L"testHotspotBean does not work on Java 8+ compact profile.", e);
  }
  try {
    type_info::forName(RamUsageEstimator::HOTSPOT_BEAN_CLASS);
  } catch (const ClassNotFoundException &e) {
    assumeNoException(L"testHotspotBean only works on Hotspot (OpenJDK, "
                      L"Oracle) virtual machines.",
                      e);
  }

  assertTrue(L"We should have been able to detect Hotspot's internal settings "
             L"from the management bean.",
             RamUsageEstimator::JVM_IS_HOTSPOT_64BIT);
}

void TestRamUsageEstimator::testPrintValues()
{
  assumeTrue(
      L"Specify -Dtests.verbose=true to print constants of RamUsageEstimator.",
      VERBOSE);
  wcout << L"JVM_IS_HOTSPOT_64BIT = " << RamUsageEstimator::JVM_IS_HOTSPOT_64BIT
        << endl;
  wcout << L"COMPRESSED_REFS_ENABLED = "
        << RamUsageEstimator::COMPRESSED_REFS_ENABLED << endl;
  wcout << L"NUM_BYTES_OBJECT_ALIGNMENT = "
        << RamUsageEstimator::NUM_BYTES_OBJECT_ALIGNMENT << endl;
  wcout << L"NUM_BYTES_OBJECT_REF = " << RamUsageEstimator::NUM_BYTES_OBJECT_REF
        << endl;
  wcout << L"NUM_BYTES_OBJECT_HEADER = "
        << RamUsageEstimator::NUM_BYTES_OBJECT_HEADER << endl;
  wcout << L"NUM_BYTES_ARRAY_HEADER = "
        << RamUsageEstimator::NUM_BYTES_ARRAY_HEADER << endl;
  wcout << L"LONG_SIZE = " << RamUsageEstimator::LONG_SIZE << endl;
  wcout << L"LONG_CACHE_MIN_VALUE = " << RamUsageEstimator::LONG_CACHE_MIN_VALUE
        << endl;
  wcout << L"LONG_CACHE_MAX_VALUE = " << RamUsageEstimator::LONG_CACHE_MAX_VALUE
        << endl;
}

TestRamUsageEstimator::Holder::Holder() {}

TestRamUsageEstimator::Holder::Holder(const wstring &name, int64_t field1)
{
  this->name = name;
  this->field1 = field1;
}
} // namespace org::apache::lucene::util