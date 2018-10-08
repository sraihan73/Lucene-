using namespace std;

#include "TestCJKBigramFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"

namespace org::apache::lucene::analysis::cjk
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestCJKBigramFilterFactory::testDefaults() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"多くの学生が試験に落ちた。");
  shared_ptr<TokenStream> stream = tokenizerFactory(L"standard").create();
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"CJKBigram").create(stream);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"多く", L"くの", L"の学", L"学生", L"生が", L"が試",
                           L"試験", L"験に", L"に落", L"落ち", L"ちた"});
}

void TestCJKBigramFilterFactory::testHanOnly() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"多くの学生が試験に落ちた。");
  shared_ptr<TokenStream> stream = tokenizerFactory(L"standard").create();
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream =
      tokenFilterFactory(L"CJKBigram", {L"hiragana", L"false"})->create(stream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"多", L"く", L"の", L"学生", L"が", L"試験",
                                   L"に", L"落", L"ち", L"た"});
}

void TestCJKBigramFilterFactory::testHanOnlyUnigrams() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"多くの学生が試験に落ちた。");
  shared_ptr<TokenStream> stream = tokenizerFactory(L"standard").create();
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"CJKBigram", {L"hiragana", L"false",
                                             L"outputUnigrams", L"true"})
               ->create(stream);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"多", L"く", L"の", L"学", L"学生", L"生", L"が",
                           L"試", L"試験", L"験", L"に", L"落", L"ち", L"た"});
}

void TestCJKBigramFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"CJKBigram", {L"bogusArg", L"bogusValue"});
  });
}
} // namespace org::apache::lucene::analysis::cjk