using namespace std;

#include "TestOpenNLPChunkerFilterFactory.h"
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
const wstring TestOpenNLPChunkerFilterFactory::SENTENCES =
    L"Sentence number 1 has 6 words. Sentence number 2, 5 words.";
std::deque<wstring> const TestOpenNLPChunkerFilterFactory::SENTENCES_punc = {
    L"Sentence", L"number", L"1", L"has", L"6", L"words", L".",
    L"Sentence", L"number", L"2", L",",   L"5", L"words", L"."};
std::deque<int> const TestOpenNLPChunkerFilterFactory::SENTENCES_startOffsets =
    {0, 9, 16, 18, 22, 24, 29, 31, 40, 47, 48, 50, 52, 57};
std::deque<int> const TestOpenNLPChunkerFilterFactory::SENTENCES_endOffsets = {
    8, 15, 17, 21, 23, 29, 30, 39, 46, 48, 49, 51, 57, 58};
std::deque<wstring> const TestOpenNLPChunkerFilterFactory::SENTENCES_chunks = {
    L"B-NP", L"I-NP", L"I-NP", L"B-VP", L"B-NP", L"I-NP", L"O",
    L"B-NP", L"I-NP", L"I-NP", L"O",    L"B-NP", L"I-NP", L"O"};
const wstring TestOpenNLPChunkerFilterFactory::sentenceModelFile =
    L"en-test-sent.bin";
const wstring TestOpenNLPChunkerFilterFactory::tokenizerModelFile =
    L"en-test-tokenizer.bin";
const wstring TestOpenNLPChunkerFilterFactory::posTaggerModelFile =
    L"en-test-pos-maxent.bin";
const wstring TestOpenNLPChunkerFilterFactory::chunkerModelFile =
    L"en-test-chunker.bin";

std::deque<std::deque<char>>
TestOpenNLPChunkerFilterFactory::toPayloads(deque<wstring> &strings)
{
  return Arrays::stream(strings)
      .map_obj([&](any s) {
        return s == nullptr
                   ? nullptr
                   : s::getBytes(java::nio::charset::StandardCharsets::UTF_8);
      })
      .toArray(char[][] ::new);
}

void TestOpenNLPChunkerFilterFactory::testBasic() 
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp", {L"tokenizerModel", tokenizerModelFile,
                                       L"sentenceModel", sentenceModelFile})
          ->addTokenFilter(L"opennlpPOS",
                           {L"posTaggerModel", posTaggerModelFile})
          ->addTokenFilter(L"opennlpChunker",
                           {L"chunkerModel", chunkerModelFile})
          ->build();
  assertAnalyzesTo(analyzer, SENTENCES, SENTENCES_punc, SENTENCES_startOffsets,
                   SENTENCES_endOffsets, SENTENCES_chunks, nullptr, nullptr,
                   true);
}

void TestOpenNLPChunkerFilterFactory::testPayloads() 
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp", {L"tokenizerModel", tokenizerModelFile,
                                       L"sentenceModel", sentenceModelFile})
          ->addTokenFilter(L"opennlpPOS",
                           {L"posTaggerModel", posTaggerModelFile})
          ->addTokenFilter(L"opennlpChunker",
                           {L"chunkerModel", chunkerModelFile})
          ->addTokenFilter(TypeAsPayloadTokenFilterFactory::typeid)
          ->build();
  assertAnalyzesTo(analyzer, SENTENCES, SENTENCES_punc, SENTENCES_startOffsets,
                   SENTENCES_endOffsets, nullptr, nullptr, nullptr, true,
                   toPayloads(SENTENCES_chunks));
}
} // namespace org::apache::lucene::analysis::opennlp