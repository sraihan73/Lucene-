using namespace std;

#include "TestTypeTokenFilterFactory.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/CannedTokenStream.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/TypeTokenFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/TokenFilterFactory.h"

namespace org::apache::lucene::analysis::core
{
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

void TestTypeTokenFilterFactory::testInform() 
{
  shared_ptr<TypeTokenFilterFactory> factory =
      std::static_pointer_cast<TypeTokenFilterFactory>(
          tokenFilterFactory(L"Type", {L"types", L"stoptypes-1.txt"}));
  shared_ptr<Set<wstring>> types = factory->getStopTypes();
  assertTrue(L"types is null and it shouldn't be", types != nullptr);
  assertTrue(L"types Size: " + types->size() + L" is not: " + to_wstring(2),
             types->size() == 2);

  factory = std::static_pointer_cast<TypeTokenFilterFactory>(tokenFilterFactory(
      L"Type", {L"types", L"stoptypes-1.txt, stoptypes-2.txt", L"useWhitelist",
                L"true"}));
  types = factory->getStopTypes();
  assertTrue(L"types is null and it shouldn't be", types != nullptr);
  assertTrue(L"types Size: " + types->size() + L" is not: " + to_wstring(4),
             types->size() == 4);
}

void TestTypeTokenFilterFactory::testCreationWithBlackList() throw(
    runtime_error)
{
  shared_ptr<TokenFilterFactory> factory = tokenFilterFactory(
      L"Type", {L"types", L"stoptypes-1.txt, stoptypes-2.txt"});
  shared_ptr<CannedTokenStream> input = make_shared<CannedTokenStream>();
  factory->create(input);
}

void TestTypeTokenFilterFactory::testCreationWithWhiteList() throw(
    runtime_error)
{
  shared_ptr<TokenFilterFactory> factory = tokenFilterFactory(
      L"Type", {L"types", L"stoptypes-1.txt, stoptypes-2.txt", L"useWhitelist",
                L"true"});
  shared_ptr<CannedTokenStream> input = make_shared<CannedTokenStream>();
  factory->create(input);
}

void TestTypeTokenFilterFactory::testMissingTypesParameter() throw(
    runtime_error)
{
  // not supplying 'types' parameter should cause an IllegalArgumentException
  expectThrows(invalid_argument::typeid,
               [&]() { tokenFilterFactory(L"Type"); });
}

void TestTypeTokenFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(
        L"Type", {L"types", L"stoptypes-1.txt", L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::core