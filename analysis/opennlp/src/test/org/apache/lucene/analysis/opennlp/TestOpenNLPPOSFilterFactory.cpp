using namespace std;

#include "TestOpenNLPPOSFilterFactory.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/custom/CustomAnalyzer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/payloads/TypeAsPayloadTokenFilterFactory.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ClasspathResourceLoader.h"

namespace org::apache::lucene::analysis::opennlp
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CustomAnalyzer = org::apache::lucene::analysis::custom::CustomAnalyzer;
using TypeAsPayloadTokenFilterFactory =
    org::apache::lucene::analysis::payloads::TypeAsPayloadTokenFilterFactory;
using ClasspathResourceLoader =
    org::apache::lucene::analysis::util::ClasspathResourceLoader;
const wstring TestOpenNLPPOSFilterFactory::SENTENCES =
    L"Sentence number 1 has 6 words. Sentence number 2, 5 words.";
std::deque<wstring> const TestOpenNLPPOSFilterFactory::SENTENCES_punc = {
    L"Sentence", L"number", L"1", L"has", L"6", L"words", L".",
    L"Sentence", L"number", L"2", L",",   L"5", L"words", L"."};
std::deque<int> const TestOpenNLPPOSFilterFactory::SENTENCES_startOffsets = {
    0, 9, 16, 18, 22, 24, 29, 31, 40, 47, 48, 50, 52, 57};
std::deque<int> const TestOpenNLPPOSFilterFactory::SENTENCES_endOffsets = {
    8, 15, 17, 21, 23, 29, 30, 39, 46, 48, 49, 51, 57, 58};
std::deque<wstring> const TestOpenNLPPOSFilterFactory::SENTENCES_posTags = {
    L"NN", L"NN", L"CD", L"VBZ", L"CD", L"NNS", L".",
    L"NN", L"NN", L"CD", L",",   L"CD", L"NNS", L"."};
const wstring TestOpenNLPPOSFilterFactory::NO_BREAK = L"No period";
std::deque<wstring> const TestOpenNLPPOSFilterFactory::NO_BREAK_terms = {
    L"No", L"period"};
std::deque<int> const TestOpenNLPPOSFilterFactory::NO_BREAK_startOffsets = {0,
                                                                             3};
std::deque<int> const TestOpenNLPPOSFilterFactory::NO_BREAK_endOffsets = {2,
                                                                           9};
const wstring TestOpenNLPPOSFilterFactory::sentenceModelFile =
    L"en-test-sent.bin";
const wstring TestOpenNLPPOSFilterFactory::tokenizerModelFile =
    L"en-test-tokenizer.bin";
const wstring TestOpenNLPPOSFilterFactory::posTaggerModelFile =
    L"en-test-pos-maxent.bin";

std::deque<std::deque<char>>
TestOpenNLPPOSFilterFactory::toPayloads(deque<wstring> &strings)
{
  return Arrays::stream(strings)
      .map_obj([&](any s) {
        return s == nullptr
                   ? nullptr
                   : s::getBytes(java::nio::charset::StandardCharsets::UTF_8);
      })
      .toArray(char[][] ::new);
}

void TestOpenNLPPOSFilterFactory::testBasic() 
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp", {L"tokenizerModel", tokenizerModelFile,
                                       L"sentenceModel", sentenceModelFile})
          ->addTokenFilter(L"opennlpPOS",
                           {L"posTaggerModel", posTaggerModelFile})
          ->build();
  assertAnalyzesTo(analyzer, SENTENCES, SENTENCES_punc, SENTENCES_startOffsets,
                   SENTENCES_endOffsets);
}

void TestOpenNLPPOSFilterFactory::testPOS() 
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp", {L"tokenizerModel", tokenizerModelFile,
                                       L"sentenceModel", sentenceModelFile})
          ->addTokenFilter(L"opennlpPOS",
                           {L"posTaggerModel", posTaggerModelFile})
          ->build();
  assertAnalyzesTo(analyzer, SENTENCES, SENTENCES_punc, SENTENCES_startOffsets,
                   SENTENCES_endOffsets, SENTENCES_posTags, nullptr, nullptr,
                   true);

  analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp", {L"tokenizerModel", tokenizerModelFile,
                                       L"sentenceModel", sentenceModelFile})
          ->addTokenFilter(L"opennlpPOS",
                           {L"posTaggerModel", posTaggerModelFile})
          ->addTokenFilter(TypeAsPayloadTokenFilterFactory::typeid)
          ->build();
  assertAnalyzesTo(analyzer, SENTENCES, SENTENCES_punc, SENTENCES_startOffsets,
                   SENTENCES_endOffsets, nullptr, nullptr, nullptr, true,
                   toPayloads(SENTENCES_posTags));
}

void TestOpenNLPPOSFilterFactory::testNoBreak() 
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp", {L"tokenizerModel", tokenizerModelFile,
                                       L"sentenceModel", sentenceModelFile})
          ->addTokenFilter(L"opennlpPOS",
                           {L"posTaggerModel", posTaggerModelFile})
          ->build();
  assertAnalyzesTo(analyzer, NO_BREAK, NO_BREAK_terms, NO_BREAK_startOffsets,
                   NO_BREAK_endOffsets, nullptr, nullptr, nullptr, true);
}
} // namespace org::apache::lucene::analysis::opennlp