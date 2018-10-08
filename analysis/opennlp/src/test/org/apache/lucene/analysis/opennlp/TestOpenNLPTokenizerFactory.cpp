using namespace std;

#include "TestOpenNLPTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/custom/CustomAnalyzer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ClasspathResourceLoader.h"
#include "../../../../../../java/org/apache/lucene/analysis/opennlp/OpenNLPTokenizerFactory.h"

namespace org::apache::lucene::analysis::opennlp
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CustomAnalyzer = org::apache::lucene::analysis::custom::CustomAnalyzer;
using ClasspathResourceLoader =
    org::apache::lucene::analysis::util::ClasspathResourceLoader;
using org::junit::Test;
wstring TestOpenNLPTokenizerFactory::SENTENCES =
    L"Sentence number 1 has 6 words. Sentence number 2, 5 words.";
std::deque<wstring> TestOpenNLPTokenizerFactory::SENTENCES_punc = {
    L"Sentence", L"number", L"1", L"has", L"6", L"words", L".",
    L"Sentence", L"number", L"2", L",",   L"5", L"words", L"."};
std::deque<int> TestOpenNLPTokenizerFactory::SENTENCES_startOffsets = {
    0, 9, 16, 18, 22, 24, 29, 31, 40, 47, 48, 50, 52, 57};
std::deque<int> TestOpenNLPTokenizerFactory::SENTENCES_endOffsets = {
    8, 15, 17, 21, 23, 29, 30, 39, 46, 48, 49, 51, 57, 58};
wstring TestOpenNLPTokenizerFactory::SENTENCE1 =
    L"Sentence number 1 has 6 words.";
std::deque<wstring> TestOpenNLPTokenizerFactory::SENTENCE1_punc = {
    L"Sentence", L"number", L"1", L"has", L"6", L"words", L"."};

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTokenizer() throws java.io.IOException
void TestOpenNLPTokenizerFactory::testTokenizer() 
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp",
                          {L"sentenceModel", L"en-test-sent.bin",
                           L"tokenizerModel", L"en-test-tokenizer.bin"})
          ->build();
  assertAnalyzesTo(analyzer, SENTENCES, SENTENCES_punc, SENTENCES_startOffsets,
                   SENTENCES_endOffsets);
  assertAnalyzesTo(analyzer, SENTENCE1, SENTENCE1_punc);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTokenizerNoSentenceDetector() throws
// java.io.IOException
void TestOpenNLPTokenizerFactory::testTokenizerNoSentenceDetector() throw(
    IOException)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    shared_ptr<CustomAnalyzer> analyzer =
        CustomAnalyzer::builder(
            make_shared<ClasspathResourceLoader>(getClass()))
            ->withTokenizer(L"opennlp",
                            {L"tokenizerModel", L"en-test-tokenizer.bin"})
            ->build();
  });
  assertTrue(expected.what()->contains(
      L"Configuration Error: missing parameter 'sentenceModel'"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTokenizerNoTokenizer() throws
// java.io.IOException
void TestOpenNLPTokenizerFactory::testTokenizerNoTokenizer() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    shared_ptr<CustomAnalyzer> analyzer =
        CustomAnalyzer::builder(
            make_shared<ClasspathResourceLoader>(getClass()))
            ->withTokenizer(L"opennlp", {L"sentenceModel", L"en-test-sent.bin"})
            ->build();
  });
  assertTrue(expected.what()->contains(
      L"Configuration Error: missing parameter 'tokenizerModel'"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testClose() throws java.io.IOException
void TestOpenNLPTokenizerFactory::testClose() 
{
  unordered_map<wstring, wstring> args =
      make_shared<HashMapAnonymousInnerClass>(shared_from_this());
  shared_ptr<OpenNLPTokenizerFactory> factory =
      make_shared<OpenNLPTokenizerFactory>(args);
  factory->inform(make_shared<ClasspathResourceLoader>(getClass()));

  shared_ptr<Tokenizer> ts = factory->create(newAttributeFactory());
  ts->setReader(make_shared<StringReader>(SENTENCES));

  ts->reset();
  delete ts;
  ts->reset();
  ts->setReader(make_shared<StringReader>(SENTENCES));
  assertTokenStreamContents(ts, SENTENCES_punc);
  delete ts;
  ts->reset();
  ts->setReader(make_shared<StringReader>(SENTENCES));
  assertTokenStreamContents(ts, SENTENCES_punc);
}

TestOpenNLPTokenizerFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestOpenNLPTokenizerFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"sentenceModel", L"en-test-sent.bin");
  this->put(L"tokenizerModel", L"en-test-tokenizer.bin");
}
} // namespace org::apache::lucene::analysis::opennlp