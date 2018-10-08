using namespace std;

#include "TestStopFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/StopAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/StopFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/ClasspathResourceLoader.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/ResourceLoader.h"

namespace org::apache::lucene::analysis::core
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;
using ClasspathResourceLoader =
    org::apache::lucene::analysis::util::ClasspathResourceLoader;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;

void TestStopFilterFactory::testInform() 
{
  shared_ptr<ResourceLoader> loader =
      make_shared<ClasspathResourceLoader>(getClass());
  assertTrue(L"loader is null and it shouldn't be", loader != nullptr);
  shared_ptr<StopFilterFactory> factory =
      std::static_pointer_cast<StopFilterFactory>(tokenFilterFactory(
          L"Stop", {L"words", L"stop-1.txt", L"ignoreCase", L"true"}));
  shared_ptr<CharArraySet> words = factory->getStopWords();
  assertTrue(L"words is null and it shouldn't be", words != nullptr);
  assertTrue(L"words Size: " + to_wstring(words->size()) + L" is not: " +
                 to_wstring(2),
             words->size() == 2);
  assertTrue(StringHelper::toString(factory->isIgnoreCase()) +
                 L" does not equal: " + StringHelper::toString(true),
             factory->isIgnoreCase() == true);

  factory = std::static_pointer_cast<StopFilterFactory>(tokenFilterFactory(
      L"Stop", {L"words", L"stop-1.txt, stop-2.txt", L"ignoreCase", L"true"}));
  words = factory->getStopWords();
  assertTrue(L"words is null and it shouldn't be", words != nullptr);
  assertTrue(L"words Size: " + to_wstring(words->size()) + L" is not: " +
                 to_wstring(4),
             words->size() == 4);
  assertTrue(StringHelper::toString(factory->isIgnoreCase()) +
                 L" does not equal: " + StringHelper::toString(true),
             factory->isIgnoreCase() == true);

  factory = std::static_pointer_cast<StopFilterFactory>(
      tokenFilterFactory(L"Stop", {L"words", L"stop-snowball.txt", L"format",
                                   L"snowball", L"ignoreCase", L"true"}));
  words = factory->getStopWords();
  assertEquals(8, words->size());
  assertTrue(words->contains(L"he"));
  assertTrue(words->contains(L"him"));
  assertTrue(words->contains(L"his"));
  assertTrue(words->contains(L"himself"));
  assertTrue(words->contains(L"she"));
  assertTrue(words->contains(L"her"));
  assertTrue(words->contains(L"hers"));
  assertTrue(words->contains(L"herself"));

  // defaults
  factory =
      std::static_pointer_cast<StopFilterFactory>(tokenFilterFactory(L"Stop"));
  assertEquals(StopAnalyzer::ENGLISH_STOP_WORDS_SET, factory->getStopWords());
  assertEquals(false, factory->isIgnoreCase());
}

void TestStopFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"Stop", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

void TestStopFilterFactory::testBogusFormats() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"Stop",
                       {L"words", L"stop-snowball.txt", L"format", L"bogus"});
  });
  wstring msg = expected.what();
  assertTrue(msg, msg.find(L"Unknown") != wstring::npos);
  assertTrue(msg, msg.find(L"format") != wstring::npos);
  assertTrue(msg, msg.find(L"bogus") != wstring::npos);

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"Stop", {L"format", L"bogus"});
    fail();
  });
  msg = expected.what();
  assertTrue(msg, msg.find(L"can not be specified") != wstring::npos);
  assertTrue(msg, msg.find(L"format") != wstring::npos);
  assertTrue(msg, msg.find(L"bogus") != wstring::npos);
}
} // namespace org::apache::lucene::analysis::core