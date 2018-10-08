using namespace std;

#include "TestBeiderMorseFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../java/org/apache/lucene/analysis/phonetic/BeiderMorseFilterFactory.h"

namespace org::apache::lucene::analysis::phonetic
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

void TestBeiderMorseFilterFactory::testBasics() 
{
  shared_ptr<BeiderMorseFilterFactory> factory =
      make_shared<BeiderMorseFilterFactory>(unordered_map<wstring, wstring>());
  shared_ptr<TokenStream> ts =
      factory->create(whitespaceMockTokenizer(L"Weinberg"));
  assertTokenStreamContents(
      ts,
      std::deque<wstring>{L"vDnbYrk", L"vDnbirk", L"vanbYrk", L"vanbirk",
                           L"vinbYrk", L"vinbirk", L"wDnbirk", L"wanbirk",
                           L"winbirk"},
      std::deque<int>{0, 0, 0, 0, 0, 0, 0, 0, 0},
      std::deque<int>{8, 8, 8, 8, 8, 8, 8, 8, 8},
      std::deque<int>{1, 0, 0, 0, 0, 0, 0, 0, 0});
}

void TestBeiderMorseFilterFactory::testLanguageSet() 
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"languageSet", L"polish");
  shared_ptr<BeiderMorseFilterFactory> factory =
      make_shared<BeiderMorseFilterFactory>(args);
  shared_ptr<TokenStream> ts =
      factory->create(whitespaceMockTokenizer(L"Weinberg"));
  assertTokenStreamContents(
      ts,
      std::deque<wstring>{L"vDmbYrk", L"vDmbirk", L"vambYrk", L"vambirk",
                           L"vimbYrk", L"vimbirk"},
      std::deque<int>{0, 0, 0, 0, 0, 0}, std::deque<int>{8, 8, 8, 8, 8, 8},
      std::deque<int>{1, 0, 0, 0, 0, 0});
}

void TestBeiderMorseFilterFactory::testOptions() 
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"nameType", L"ASHKENAZI");
  args.emplace(L"ruleType", L"EXACT");
  shared_ptr<BeiderMorseFilterFactory> factory =
      make_shared<BeiderMorseFilterFactory>(args);
  shared_ptr<TokenStream> ts =
      factory->create(whitespaceMockTokenizer(L"Weinberg"));
  assertTokenStreamContents(ts, std::deque<wstring>{L"vajnberk"},
                            std::deque<int>{0}, std::deque<int>{8},
                            std::deque<int>{1});
}

void TestBeiderMorseFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<BeiderMorseFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestBeiderMorseFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestBeiderMorseFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::phonetic