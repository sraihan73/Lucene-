using namespace std;

#include "TestJapaneseTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseTokenizerFactory.h"
#include "StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::ja
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

void TestJapaneseTokenizerFactory::testSimple() 
{
  shared_ptr<JapaneseTokenizerFactory> factory =
      make_shared<JapaneseTokenizerFactory>(unordered_map<wstring, wstring>());
  factory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<TokenStream> ts = factory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"これは本ではない"));
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"これ", L"は", L"本", L"で", L"は", L"ない"},
      std::deque<int>{0, 2, 3, 4, 5, 6}, std::deque<int>{2, 3, 4, 5, 6, 8});
}

void TestJapaneseTokenizerFactory::testDefaults() 
{
  shared_ptr<JapaneseTokenizerFactory> factory =
      make_shared<JapaneseTokenizerFactory>(unordered_map<wstring, wstring>());
  factory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<TokenStream> ts = factory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"シニアソフトウェアエンジニア"));
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"シニア", L"シニアソフトウェアエンジニア",
                               L"ソフトウェア", L"エンジニア"});
}

void TestJapaneseTokenizerFactory::testMode() 
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"mode", L"normal");
  shared_ptr<JapaneseTokenizerFactory> factory =
      make_shared<JapaneseTokenizerFactory>(args);
  factory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<TokenStream> ts = factory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"シニアソフトウェアエンジニア"));
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"シニアソフトウェアエンジニア"});
}

void TestJapaneseTokenizerFactory::testUserDict() 
{
  wstring userDict =
      wstring(L"# Custom segmentation for long entries\n") +
      L"日本経済新聞,日本 経済 新聞,ニホン ケイザイ シンブン,カスタム名詞\n" +
      L"関西国際空港,関西 国際 空港,カンサイ コクサイ クウコウ,テスト名詞\n" +
      L"# Custom reading for sumo wrestler\n" +
      L"朝青龍,朝青龍,アサショウリュウ,カスタム人名\n";
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"userDictionary", L"userdict.txt");
  shared_ptr<JapaneseTokenizerFactory> factory =
      make_shared<JapaneseTokenizerFactory>(args);
  factory->inform(make_shared<StringMockResourceLoader>(userDict));
  shared_ptr<TokenStream> ts = factory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"関西国際空港に行った"));
  assertTokenStreamContents(ts, std::deque<wstring>{L"関西", L"国際", L"空港",
                                                     L"に", L"行っ", L"た"});
}

void TestJapaneseTokenizerFactory::testPreservePunctuation() 
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"discardPunctuation", L"false");
  shared_ptr<JapaneseTokenizerFactory> factory =
      make_shared<JapaneseTokenizerFactory>(args);
  factory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<TokenStream> ts = factory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(
          L"今ノルウェーにいますが、来週の頭日本に戻ります。楽しみにしています"
          L"！お寿司が食べたいな。。。"));
  assertTokenStreamContents(
      ts,
      std::deque<wstring>{
          L"今",   L"ノルウェー", L"に",   L"い",   L"ます", L"が",   L"、",
          L"来週", L"の",         L"頭",   L"日本", L"に",   L"戻り", L"ます",
          L"。",   L"楽しみ",     L"に",   L"し",   L"て",   L"い",   L"ます",
          L"！",   L"お",         L"寿司", L"が",   L"食べ", L"たい", L"な",
          L"。",   L"。",         L"。"});
}

void TestJapaneseTokenizerFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<JapaneseTokenizerFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestJapaneseTokenizerFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestJapaneseTokenizerFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}

shared_ptr<TokenStream> TestJapaneseTokenizerFactory::makeTokenStream(
    unordered_map<wstring, wstring> &args,
    const wstring &in_) 
{
  shared_ptr<JapaneseTokenizerFactory> factory =
      make_shared<JapaneseTokenizerFactory>(args);
  factory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<Tokenizer> t = factory->create(newAttributeFactory());
  t->setReader(make_shared<StringReader>(in_));
  return t;
}

void TestJapaneseTokenizerFactory::testNbestCost() 
{
  assertTokenStreamContents(
      makeTokenStream(
          make_shared<HashMapAnonymousInnerClass>(shared_from_this()),
          L"鳩山積み"),
      std::deque<wstring>{L"鳩", L"鳩山", L"山積み", L"積み"});
}

TestJapaneseTokenizerFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestJapaneseTokenizerFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"nBestCost", L"2000");
}

void TestJapaneseTokenizerFactory::testNbestExample() 
{
  assertTokenStreamContents(
      makeTokenStream(
          make_shared<HashMapAnonymousInnerClass2>(shared_from_this()),
          L"鳩山積み"),
      std::deque<wstring>{L"鳩", L"鳩山", L"山積み", L"積み"});
}

TestJapaneseTokenizerFactory::HashMapAnonymousInnerClass2::
    HashMapAnonymousInnerClass2(
        shared_ptr<TestJapaneseTokenizerFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"nBestExamples", L"/鳩山積み-鳩山/鳩山積み-鳩/");
}
} // namespace org::apache::lucene::analysis::ja