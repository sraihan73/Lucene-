using namespace std;

#include "TestKeepFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/KeepWordFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/ClasspathResourceLoader.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/ResourceLoader.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;
using ClasspathResourceLoader =
    org::apache::lucene::analysis::util::ClasspathResourceLoader;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;

void TestKeepFilterFactory::testInform() 
{
  shared_ptr<ResourceLoader> loader =
      make_shared<ClasspathResourceLoader>(getClass());
  assertTrue(L"loader is null and it shouldn't be", loader != nullptr);
  shared_ptr<KeepWordFilterFactory> factory =
      std::static_pointer_cast<KeepWordFilterFactory>(tokenFilterFactory(
          L"KeepWord", {L"words", L"keep-1.txt", L"ignoreCase", L"true"}));
  shared_ptr<CharArraySet> words = factory->getWords();
  assertTrue(L"words is null and it shouldn't be", words != nullptr);
  assertTrue(L"words Size: " + to_wstring(words->size()) + L" is not: " +
                 to_wstring(2),
             words->size() == 2);

  factory = std::static_pointer_cast<KeepWordFilterFactory>(
      tokenFilterFactory(L"KeepWord", {L"words", L"keep-1.txt, keep-2.txt",
                                       L"ignoreCase", L"true"}));
  words = factory->getWords();
  assertTrue(L"words is null and it shouldn't be", words != nullptr);
  assertTrue(L"words Size: " + to_wstring(words->size()) + L" is not: " +
                 to_wstring(4),
             words->size() == 4);
}

void TestKeepFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"KeepWord", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::miscellaneous