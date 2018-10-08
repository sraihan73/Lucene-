using namespace std;

#include "TestNamedSPILoader.h"

namespace org::apache::lucene::util
{
using Codec = org::apache::lucene::codecs::Codec;

void TestNamedSPILoader::testLookup()
{
  wstring currentName = TestUtil::getDefaultCodec()->getName();
  shared_ptr<Codec> codec = Codec::forName(currentName);
  assertEquals(currentName, codec->getName());
}

void TestNamedSPILoader::testBogusLookup()
{
  expectThrows(invalid_argument::typeid,
               [&]() { Codec::forName(L"dskfdskfsdfksdfdsf"); });
}

void TestNamedSPILoader::testAvailableServices()
{
  shared_ptr<Set<wstring>> codecs = Codec::availableCodecs();
  assertTrue(codecs->contains(TestUtil::getDefaultCodec()->getName()));
}
} // namespace org::apache::lucene::util