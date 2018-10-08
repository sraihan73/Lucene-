using namespace std;

#include "TestThaiTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/th/ThaiTokenizer.h"

namespace org::apache::lucene::analysis::th
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestThaiTokenizerFactory::testWordBreak() 
{
  assumeTrue(L"JRE does not support Thai dictionary-based BreakIterator",
             ThaiTokenizer::DBBI_AVAILABLE);
  shared_ptr<Tokenizer> tokenizer =
      tokenizerFactory(L"Thai").create(newAttributeFactory());
  tokenizer->setReader(make_shared<StringReader>(L"การที่ได้ต้องแสดงว่างานดี"));
  assertTokenStreamContents(tokenizer,
                            std::deque<wstring>{L"การ", L"ที่", L"ได้", L"ต้อง",
                                                 L"แสดง", L"ว่า", L"งาน", L"ดี"});
}

void TestThaiTokenizerFactory::testBogusArguments() 
{
  assumeTrue(L"JRE does not support Thai dictionary-based BreakIterator",
             ThaiTokenizer::DBBI_AVAILABLE);
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenizerFactory(L"Thai", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::th