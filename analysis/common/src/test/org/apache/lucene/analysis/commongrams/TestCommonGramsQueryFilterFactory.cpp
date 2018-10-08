using namespace std;

#include "TestCommonGramsQueryFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../../../../../../java/org/apache/lucene/analysis/commongrams/CommonGramsQueryFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/ClasspathResourceLoader.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "../core/TestStopFilterFactory.h"

namespace org::apache::lucene::analysis::commongrams
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TestStopFilterFactory =
    org::apache::lucene::analysis::core::TestStopFilterFactory;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;
using ClasspathResourceLoader =
    org::apache::lucene::analysis::util::ClasspathResourceLoader;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using Version = org::apache::lucene::util::Version;

void TestCommonGramsQueryFilterFactory::testInform() 
{
  shared_ptr<ResourceLoader> loader =
      make_shared<ClasspathResourceLoader>(TestStopFilterFactory::typeid);
  assertTrue(L"loader is null and it shouldn't be", loader != nullptr);
  shared_ptr<CommonGramsQueryFilterFactory> factory =
      std::static_pointer_cast<CommonGramsQueryFilterFactory>(
          tokenFilterFactory(L"CommonGramsQuery", Version::LATEST, loader,
                             L"words", L"stop-1.txt", L"ignoreCase", L"true"));
  shared_ptr<CharArraySet> words = factory->getCommonWords();
  assertTrue(L"words is null and it shouldn't be", words != nullptr);
  assertTrue(L"words Size: " + to_wstring(words->size()) + L" is not: " +
                 to_wstring(2),
             words->size() == 2);
  assertTrue(StringHelper::toString(factory->isIgnoreCase()) +
                 L" does not equal: " + StringHelper::toString(true),
             factory->isIgnoreCase() == true);

  factory = std::static_pointer_cast<CommonGramsQueryFilterFactory>(
      tokenFilterFactory(L"CommonGramsQuery", Version::LATEST, loader, L"words",
                         L"stop-1.txt, stop-2.txt", L"ignoreCase", L"true"));
  words = factory->getCommonWords();
  assertTrue(L"words is null and it shouldn't be", words != nullptr);
  assertTrue(L"words Size: " + to_wstring(words->size()) + L" is not: " +
                 to_wstring(4),
             words->size() == 4);
  assertTrue(StringHelper::toString(factory->isIgnoreCase()) +
                 L" does not equal: " + StringHelper::toString(true),
             factory->isIgnoreCase() == true);

  factory = std::static_pointer_cast<CommonGramsQueryFilterFactory>(
      tokenFilterFactory(L"CommonGramsQuery", Version::LATEST, loader, L"words",
                         L"stop-snowball.txt", L"format", L"snowball",
                         L"ignoreCase", L"true"));
  words = factory->getCommonWords();
  assertEquals(8, words->size());
  assertTrue(words->contains(L"he"));
  assertTrue(words->contains(L"him"));
  assertTrue(words->contains(L"his"));
  assertTrue(words->contains(L"himself"));
  assertTrue(words->contains(L"she"));
  assertTrue(words->contains(L"her"));
  assertTrue(words->contains(L"hers"));
  assertTrue(words->contains(L"herself"));
}

void TestCommonGramsQueryFilterFactory::testDefaults() 
{
  shared_ptr<CommonGramsQueryFilterFactory> factory =
      std::static_pointer_cast<CommonGramsQueryFilterFactory>(
          tokenFilterFactory(L"CommonGramsQuery"));
  shared_ptr<CharArraySet> words = factory->getCommonWords();
  assertTrue(L"words is null and it shouldn't be", words != nullptr);
  assertTrue(words->contains(L"the"));
  shared_ptr<Tokenizer> tokenizer =
      whitespaceMockTokenizer(L"testing the factory");
  shared_ptr<TokenStream> stream = factory->create(tokenizer);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"testing_the", L"the_factory"});
}

void TestCommonGramsQueryFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"CommonGramsQuery", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

void TestCommonGramsQueryFilterFactory::testCompleteGraph() 
{
  shared_ptr<CommonGramsQueryFilterFactory> factory =
      std::static_pointer_cast<CommonGramsQueryFilterFactory>(
          tokenFilterFactory(L"CommonGramsQuery"));
  shared_ptr<CharArraySet> words = factory->getCommonWords();
  assertTrue(L"words is null and it shouldn't be", words != nullptr);
  assertTrue(words->contains(L"the"));
  shared_ptr<Tokenizer> tokenizer =
      whitespaceMockTokenizer(L"testing the factory works");
  shared_ptr<TokenStream> stream = factory->create(tokenizer);
  assertGraphStrings(stream, {L"testing_the the_factory factory works"});
}
} // namespace org::apache::lucene::analysis::commongrams