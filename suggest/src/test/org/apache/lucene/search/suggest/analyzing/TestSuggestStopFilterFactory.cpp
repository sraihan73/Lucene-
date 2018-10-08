using namespace std;

#include "TestSuggestStopFilterFactory.h"

namespace org::apache::lucene::search::suggest::analyzing
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopAnalyzer = org::apache::lucene::analysis::core::StopAnalyzer;
using ClasspathResourceLoader =
    org::apache::lucene::analysis::util::ClasspathResourceLoader;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using Version = org::apache::lucene::util::Version;

void TestSuggestStopFilterFactory::testInform() 
{
  shared_ptr<ResourceLoader> loader =
      make_shared<ClasspathResourceLoader>(getClass());
  assertTrue(L"loader is null and it shouldn't be", loader != nullptr);
  shared_ptr<SuggestStopFilterFactory> factory =
      createFactory({L"words", L"stop-1.txt", L"ignoreCase", L"true"});
  shared_ptr<CharArraySet> words = factory->getStopWords();
  assertTrue(L"words is null and it shouldn't be", words != nullptr);
  assertTrue(L"words Size: " + to_wstring(words->size()) + L" is not: " +
                 to_wstring(2),
             words->size() == 2);
  assertTrue(StringHelper::toString(factory->isIgnoreCase()) +
                 L" does not equal: " + StringHelper::toString(true),
             factory->isIgnoreCase() == true);

  factory = createFactory(
      {L"words", L"stop-1.txt, stop-2.txt", L"ignoreCase", L"true"});
  words = factory->getStopWords();
  assertTrue(L"words is null and it shouldn't be", words != nullptr);
  assertTrue(L"words Size: " + to_wstring(words->size()) + L" is not: " +
                 to_wstring(4),
             words->size() == 4);
  assertTrue(StringHelper::toString(factory->isIgnoreCase()) +
                 L" does not equal: " + StringHelper::toString(true),
             factory->isIgnoreCase() == true);

  factory = createFactory({L"words", L"stop-snowball.txt", L"format",
                           L"snowball", L"ignoreCase", L"true"});
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
  factory = createFactory();
  assertEquals(StopAnalyzer::ENGLISH_STOP_WORDS_SET, factory->getStopWords());
  assertEquals(false, factory->isIgnoreCase());
}

void TestSuggestStopFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    createFactory({L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

void TestSuggestStopFilterFactory::testBogusFormats() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    createFactory({L"words", L"stop-snowball.txt", L"format", L"bogus"});
  });

  wstring msg = expected.what();
  assertTrue(msg, msg.find(L"Unknown") != wstring::npos);
  assertTrue(msg, msg.find(L"format") != wstring::npos);
  assertTrue(msg, msg.find(L"bogus") != wstring::npos);

  expected = expectThrows(invalid_argument::typeid, [&]() {
    createFactory({L"format", L"bogus"});
  });
  msg = expected.what();
  assertTrue(msg, msg.find(L"can not be specified") != wstring::npos);
  assertTrue(msg, msg.find(L"format") != wstring::npos);
  assertTrue(msg, msg.find(L"bogus") != wstring::npos);
}

shared_ptr<SuggestStopFilterFactory>
TestSuggestStopFilterFactory::createFactory(deque<wstring> &params) throw(
    IOException)
{
  if (params->length % 2 != 0) {
    throw invalid_argument(L"invalid keysAndValues map_obj");
  }
  unordered_map<wstring, wstring> args =
      unordered_map<wstring, wstring>(params->length / 2);
  for (int i = 0; i < params->length; i += 2) {
    wstring previous = args.emplace(params[i], params[i + 1]);
    assertNull(L"duplicate values for key: " + params[i], previous);
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  args.emplace(L"luceneMatchVersion", Version::LATEST->toString());

  shared_ptr<SuggestStopFilterFactory> factory =
      make_shared<SuggestStopFilterFactory>(args);
  factory->inform(make_shared<ClasspathResourceLoader>(getClass()));
  return factory;
}
} // namespace org::apache::lucene::search::suggest::analyzing