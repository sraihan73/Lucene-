using namespace std;

#include "TestKeywordTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizerFactory.h"

namespace org::apache::lucene::analysis::core
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

void TestKeywordTokenizer::testSimple() 
{
  shared_ptr<StringReader> reader =
      make_shared<StringReader>(L"Tokenizer \ud801\udc1ctest");
  shared_ptr<KeywordTokenizer> tokenizer = make_shared<KeywordTokenizer>();
  tokenizer->setReader(reader);
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"Tokenizer \ud801\udc1ctest"});
}

void TestKeywordTokenizer::testFactory()
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  shared_ptr<KeywordTokenizerFactory> factory =
      make_shared<KeywordTokenizerFactory>(args);
  shared_ptr<AttributeFactory> attributeFactory = newAttributeFactory();
  shared_ptr<Tokenizer> tokenizer = factory->create(attributeFactory);
  assertEquals(KeywordTokenizer::typeid, tokenizer->getClass());
}

unordered_map<wstring, wstring>
TestKeywordTokenizer::makeArgs(deque<wstring> &args)
{
  unordered_map<wstring, wstring> ret = unordered_map<wstring, wstring>();
  for (int idx = 0; idx < args->length; idx += 2) {
    ret.emplace(args[idx], args[idx + 1]);
  }
  return ret;
}

void TestKeywordTokenizer::testParamsFactory() 
{
  // negative maxTokenLen
  invalid_argument iae = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<KeywordTokenizerFactory>(makeArgs({L"maxTokenLen", L"-1"}));
  });
  assertEquals(
      L"maxTokenLen must be greater than 0 and less than 1048576 passed: -1",
      iae.what());

  // zero maxTokenLen
  iae = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<KeywordTokenizerFactory>(makeArgs({L"maxTokenLen", L"0"}));
  });
  assertEquals(
      L"maxTokenLen must be greater than 0 and less than 1048576 passed: 0",
      iae.what());

  // Added random param, should throw illegal error
  iae = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<KeywordTokenizerFactory>(
        makeArgs({L"maxTokenLen", L"255", L"randomParam", L"rValue"}));
  });
  assertEquals(L"Unknown parameters: {randomParam=rValue}", iae.what());

  // tokeniser will never split, no matter what is passed,
  // but the buffer will not be more than length of the token

  shared_ptr<KeywordTokenizerFactory> factory =
      make_shared<KeywordTokenizerFactory>(makeArgs({L"maxTokenLen", L"5"}));
  shared_ptr<AttributeFactory> attributeFactory = newAttributeFactory();
  shared_ptr<Tokenizer> tokenizer = factory->create(attributeFactory);
  shared_ptr<StringReader> reader = make_shared<StringReader>(L"Tokenizertest");
  tokenizer->setReader(reader);
  assertTokenStreamContents(tokenizer, std::deque<wstring>{L"Tokenizertest"});

  // tokeniser will never split, no matter what is passed,
  // but the buffer will not be more than length of the token
  factory =
      make_shared<KeywordTokenizerFactory>(makeArgs({L"maxTokenLen", L"2"}));
  attributeFactory = newAttributeFactory();
  tokenizer = factory->create(attributeFactory);
  reader = make_shared<StringReader>(L"Tokenizer\u00A0test");
  tokenizer->setReader(reader);
  assertTokenStreamContents(tokenizer,
                            std::deque<wstring>{L"Tokenizer\u00A0test"});
}
} // namespace org::apache::lucene::analysis::core