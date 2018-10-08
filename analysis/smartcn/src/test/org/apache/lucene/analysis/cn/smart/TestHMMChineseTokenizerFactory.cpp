using namespace std;

#include "TestHMMChineseTokenizerFactory.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../common/src/java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include "../../../../../../../java/org/apache/lucene/analysis/cn/smart/HMMChineseTokenizerFactory.h"

namespace org::apache::lucene::analysis::cn::smart
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;

void TestHMMChineseTokenizerFactory::testSimple() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"我购买了道具和服装。");
  shared_ptr<TokenizerFactory> factory =
      make_shared<HMMChineseTokenizerFactory>(
          unordered_map<wstring, wstring>());
  shared_ptr<Tokenizer> tokenizer = factory->create(newAttributeFactory());
  tokenizer->setReader(reader);
  // TODO: fix smart chinese to not emit punctuation tokens
  // at the moment: you have to clean up with WDF, or use the stoplist, etc
  assertTokenStreamContents(tokenizer,
                            std::deque<wstring>{L"我", L"购买", L"了", L"道具",
                                                 L"和", L"服装", L","});
}

void TestHMMChineseTokenizerFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<HMMChineseTokenizerFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestHMMChineseTokenizerFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestHMMChineseTokenizerFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::cn::smart