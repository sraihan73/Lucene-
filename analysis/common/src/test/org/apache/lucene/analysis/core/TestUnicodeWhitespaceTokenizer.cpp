using namespace std;

#include "TestUnicodeWhitespaceTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/UnicodeWhitespaceTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/WhitespaceTokenizerFactory.h"

namespace org::apache::lucene::analysis::core
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

void TestUnicodeWhitespaceTokenizer::testSimple() 
{
  shared_ptr<StringReader> reader =
      make_shared<StringReader>(L"Tokenizer \ud801\udc1ctest");
  shared_ptr<UnicodeWhitespaceTokenizer> tokenizer =
      make_shared<UnicodeWhitespaceTokenizer>();
  tokenizer->setReader(reader);
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"Tokenizer", L"\ud801\udc1ctest"});
}

void TestUnicodeWhitespaceTokenizer::testNBSP() 
{
  shared_ptr<StringReader> reader =
      make_shared<StringReader>(L"Tokenizer\u00A0test");
  shared_ptr<UnicodeWhitespaceTokenizer> tokenizer =
      make_shared<UnicodeWhitespaceTokenizer>();
  tokenizer->setReader(reader);
  assertTokenStreamContents(tokenizer,
                            std::deque<wstring>{L"Tokenizer", L"test"});
}

void TestUnicodeWhitespaceTokenizer::testFactory()
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"rule", L"unicode");
  shared_ptr<WhitespaceTokenizerFactory> factory =
      make_shared<WhitespaceTokenizerFactory>(args);
  shared_ptr<AttributeFactory> attributeFactory = newAttributeFactory();
  shared_ptr<Tokenizer> tokenizer = factory->create(attributeFactory);
  assertEquals(UnicodeWhitespaceTokenizer::typeid, tokenizer->getClass());
}

unordered_map<wstring, wstring>
TestUnicodeWhitespaceTokenizer::makeArgs(deque<wstring> &args)
{
  unordered_map<wstring, wstring> ret = unordered_map<wstring, wstring>();
  for (int idx = 0; idx < args->length; idx += 2) {
    ret.emplace(args[idx], args[idx + 1]);
  }
  return ret;
}

void TestUnicodeWhitespaceTokenizer::testParamsFactory() 
{

  // negative maxTokenLen
  invalid_argument iae = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<WhitespaceTokenizerFactory>(
        makeArgs({L"rule", L"unicode", L"maxTokenLen", L"-1"}));
  });
  assertEquals(
      L"maxTokenLen must be greater than 0 and less than 1048576 passed: -1",
      iae.what());

  // zero maxTokenLen
  iae = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<WhitespaceTokenizerFactory>(
        makeArgs({L"rule", L"unicode", L"maxTokenLen", L"0"}));
  });
  assertEquals(
      L"maxTokenLen must be greater than 0 and less than 1048576 passed: 0",
      iae.what());

  // Added random param, should throw illegal error
  iae = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<WhitespaceTokenizerFactory>(
        makeArgs({L"rule", L"unicode", L"maxTokenLen", L"255", L"randomParam",
                  L"rValue"}));
  });
  assertEquals(L"Unknown parameters: {randomParam=rValue}", iae.what());

  // tokeniser will split at 5, Token | izer, no matter what happens
  shared_ptr<WhitespaceTokenizerFactory> factory =
      make_shared<WhitespaceTokenizerFactory>(
          makeArgs({L"rule", L"unicode", L"maxTokenLen", L"5"}));
  shared_ptr<AttributeFactory> attributeFactory = newAttributeFactory();
  shared_ptr<Tokenizer> tokenizer = factory->create(attributeFactory);
  shared_ptr<StringReader> reader =
      make_shared<StringReader>(L"Tokenizer \ud801\udc1ctest");
  tokenizer->setReader(reader);
  assertTokenStreamContents(
      tokenizer,
      std::deque<wstring>{L"Token", L"izer", L"\ud801\udc1ctes", L"t"});

  // tokeniser will split at 2, To | ke | ni | ze | r, no matter what happens
  factory = make_shared<WhitespaceTokenizerFactory>(
      makeArgs({L"rule", L"unicode", L"maxTokenLen", L"2"}));
  attributeFactory = newAttributeFactory();
  tokenizer = factory->create(attributeFactory);
  reader = make_shared<StringReader>(L"Tokenizer\u00A0test");
  tokenizer->setReader(reader);
  assertTokenStreamContents(
      tokenizer,
      std::deque<wstring>{L"To", L"ke", L"ni", L"ze", L"r", L"te", L"st"});

  // tokeniser will split at 10, no matter what happens,
  // but tokens' length are less than that
  factory = make_shared<WhitespaceTokenizerFactory>(
      makeArgs({L"rule", L"unicode", L"maxTokenLen", L"10"}));
  attributeFactory = newAttributeFactory();
  tokenizer = factory->create(attributeFactory);
  reader = make_shared<StringReader>(L"Tokenizer\u00A0test");
  tokenizer->setReader(reader);
  assertTokenStreamContents(tokenizer,
                            std::deque<wstring>{L"Tokenizer", L"test"});
}
} // namespace org::apache::lucene::analysis::core