using namespace std;

#include "TestMappingCharFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/MappingCharFilterFactory.h"

namespace org::apache::lucene::analysis::charfilter
{
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestMappingCharFilterFactory::testParseString() 
{

  shared_ptr<MappingCharFilterFactory> f =
      std::static_pointer_cast<MappingCharFilterFactory>(
          charFilterFactory(L"Mapping"));

  expectThrows(invalid_argument::typeid, [&]() { f->parseString(L"\\"); });

  assertEquals(L"unexpected escaped characters", L"\\\"\n\t\r\b\f",
               f->parseString(L"\\\\\\\"\\n\\t\\r\\b\\f"));
  assertEquals(L"unexpected escaped characters", L"A",
               f->parseString(L"\\u0041"));
  assertEquals(L"unexpected escaped characters", L"AB",
               f->parseString(L"\\u0041\\u0042"));

  expectThrows(invalid_argument::typeid, [&]() { f->parseString(L"\\u000"); });

  // invalid hex number
  expectThrows(NumberFormatException::typeid,
               [&]() { f->parseString(L"\\u123x"); });
}

void TestMappingCharFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    charFilterFactory(L"Mapping", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::charfilter