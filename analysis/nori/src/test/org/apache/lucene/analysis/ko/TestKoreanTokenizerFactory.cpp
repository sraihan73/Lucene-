using namespace std;

#include "TestKoreanTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/KoreanTokenizerFactory.h"
#include "StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::ko
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

void TestKoreanTokenizerFactory::testSimple() 
{
  shared_ptr<KoreanTokenizerFactory> factory =
      make_shared<KoreanTokenizerFactory>(Collections::emptyMap());
  factory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<TokenStream> ts = factory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"안녕하세요"));
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"안녕", L"하", L"시", L"어요"},
      std::deque<int>{0, 2, 3, 3}, std::deque<int>{2, 3, 5, 5});
}

void TestKoreanTokenizerFactory::testDiscardDecompound() 
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"decompoundMode", L"discard");
  shared_ptr<KoreanTokenizerFactory> factory =
      make_shared<KoreanTokenizerFactory>(args);
  factory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<TokenStream> ts = factory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"갠지스강"));
  assertTokenStreamContents(ts, std::deque<wstring>{L"갠지스", L"강"});
}

void TestKoreanTokenizerFactory::testNoDecompound() 
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"decompoundMode", L"none");
  shared_ptr<KoreanTokenizerFactory> factory =
      make_shared<KoreanTokenizerFactory>(args);
  factory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<TokenStream> ts = factory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"갠지스강"));
  assertTokenStreamContents(ts, std::deque<wstring>{L"갠지스강"});
}

void TestKoreanTokenizerFactory::testMixedDecompound() 
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"decompoundMode", L"mixed");
  shared_ptr<KoreanTokenizerFactory> factory =
      make_shared<KoreanTokenizerFactory>(args);
  factory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<TokenStream> ts = factory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"갠지스강"));
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"갠지스강", L"갠지스", L"강"});
}

void TestKoreanTokenizerFactory::testUserDict() 
{
  wstring userDict = wstring(L"# Additional nouns\n") + L"세종시 세종 시\n" +
                     L"# \n" + L"c++\n";
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"userDictionary", L"userdict.txt");
  shared_ptr<KoreanTokenizerFactory> factory =
      make_shared<KoreanTokenizerFactory>(args);
  factory->inform(make_shared<StringMockResourceLoader>(userDict));
  shared_ptr<TokenStream> ts = factory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"세종시"));
  assertTokenStreamContents(ts, std::deque<wstring>{L"세종", L"시"});
}

void TestKoreanTokenizerFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<KoreanTokenizerFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestKoreanTokenizerFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestKoreanTokenizerFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::ko