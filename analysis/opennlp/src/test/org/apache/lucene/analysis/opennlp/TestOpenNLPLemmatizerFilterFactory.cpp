using namespace std;

#include "TestOpenNLPLemmatizerFilterFactory.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/custom/CustomAnalyzer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/miscellaneous/KeywordRepeatFilterFactory.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/miscellaneous/RemoveDuplicatesTokenFilterFactory.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ClasspathResourceLoader.h"

namespace org::apache::lucene::analysis::opennlp
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CustomAnalyzer = org::apache::lucene::analysis::custom::CustomAnalyzer;
using KeywordRepeatFilterFactory =
    org::apache::lucene::analysis::miscellaneous::KeywordRepeatFilterFactory;
using RemoveDuplicatesTokenFilterFactory = org::apache::lucene::analysis::
    miscellaneous::RemoveDuplicatesTokenFilterFactory;
using ClasspathResourceLoader =
    org::apache::lucene::analysis::util::ClasspathResourceLoader;
const wstring TestOpenNLPLemmatizerFilterFactory::SENTENCE =
    L"They sent him running in the evening.";
std::deque<wstring> const
    TestOpenNLPLemmatizerFilterFactory::SENTENCE_dict_punc = {
        L"they", L"send", L"he", L"run", L"in", L"the", L"evening", L"."};
std::deque<wstring> const
    TestOpenNLPLemmatizerFilterFactory::SENTENCE_maxent_punc = {
        L"they", L"send", L"he", L"runn", L"in", L"the", L"evening", L"."};
std::deque<wstring> const
    TestOpenNLPLemmatizerFilterFactory::SENTENCE_posTags = {
        L"NNP", L"VBD", L"PRP", L"VBG", L"IN", L"DT", L"NN", L"."};
const wstring TestOpenNLPLemmatizerFilterFactory::SENTENCES =
    L"They sent him running in the evening. He did not come back.";
std::deque<wstring> const
    TestOpenNLPLemmatizerFilterFactory::SENTENCES_dict_punc = {
        L"they", L"send", L"he", L"run", L"in",   L"the",  L"evening",
        L".",    L"he",   L"do", L"not", L"come", L"back", L"."};
std::deque<wstring> const
    TestOpenNLPLemmatizerFilterFactory::SENTENCES_maxent_punc = {
        L"they", L"send", L"he", L"runn", L"in",   L"the",  L"evening",
        L".",    L"he",   L"do", L"not",  L"come", L"back", L"."};
std::deque<wstring> const
    TestOpenNLPLemmatizerFilterFactory::SENTENCES_posTags = {
        L"NNP", L"VBD", L"PRP", L"VBG", L"IN", L"DT", L"NN",
        L".",   L"PRP", L"VBD", L"RB",  L"VB", L"RB", L"."};
const wstring TestOpenNLPLemmatizerFilterFactory::SENTENCE_both =
    L"Konstantin Kalashnitsov constantly caliphed.";
std::deque<wstring> const
    TestOpenNLPLemmatizerFilterFactory::SENTENCE_both_punc = {
        L"konstantin", L"kalashnitsov", L"constantly", L"caliph", L"."};
std::deque<wstring> const
    TestOpenNLPLemmatizerFilterFactory::SENTENCE_both_posTags = {
        L"IN", L"JJ", L"NN", L"VBN", L"."};
const wstring TestOpenNLPLemmatizerFilterFactory::SENTENCES_both =
    L"Konstantin Kalashnitsov constantly caliphed. Coreena could care, "
    L"completely.";
std::deque<wstring> const
    TestOpenNLPLemmatizerFilterFactory::SENTENCES_both_punc = {
        L"konstantin", L"kalashnitsov", L"constantly", L"caliph",
        L".",          L"coreena",      L"could",      L"care",
        L",",          L"completely",   L"."};
std::deque<wstring> const
    TestOpenNLPLemmatizerFilterFactory::SENTENCES_both_posTags = {
        L"IN",  L"JJ", L"NN", L"VBN", L".", L"NNP",
        L"VBN", L"NN", L",",  L"NN",  L"."};
std::deque<wstring> const
    TestOpenNLPLemmatizerFilterFactory::SENTENCES_dict_keep_orig_punc = {
        L"They", L"they", L"sent", L"send",    L"him",  L"he", L"running",
        L"run",  L"in",   L"the",  L"evening", L".",    L"He", L"he",
        L"did",  L"do",   L"not",  L"come",    L"back", L"."};
std::deque<wstring> const
    TestOpenNLPLemmatizerFilterFactory::SENTENCES_max_ent_keep_orig_punc = {
        L"They", L"they", L"sent", L"send",    L"him",  L"he", L"running",
        L"runn", L"in",   L"the",  L"evening", L".",    L"He", L"he",
        L"did",  L"do",   L"not",  L"come",    L"back", L"."};
std::deque<wstring> const
    TestOpenNLPLemmatizerFilterFactory::SENTENCES_keep_orig_posTags = {
        L"NNP", L"NNP", L"VBD", L"VBD", L"PRP", L"PRP", L"VBG",
        L"VBG", L"IN",  L"DT",  L"NN",  L".",   L"PRP", L"PRP",
        L"VBD", L"VBD", L"RB",  L"VB",  L"RB",  L"."};
std::deque<wstring> const
    TestOpenNLPLemmatizerFilterFactory::SENTENCES_both_keep_orig_punc = {
        L"Konstantin",
        L"konstantin",
        L"Kalashnitsov",
        L"kalashnitsov",
        L"constantly",
        L"caliphed",
        L"caliph",
        L".",
        L"Coreena",
        L"coreena",
        L"could",
        L"care",
        L",",
        L"completely",
        L"."};
std::deque<wstring> const
    TestOpenNLPLemmatizerFilterFactory::SENTENCES_both_keep_orig_posTags = {
        L"IN",  L"IN",  L"JJ",  L"JJ", L"NN", L"VBN", L"VBN", L".",
        L"NNP", L"NNP", L"VBN", L"NN", L",",  L"NN",  L"."};
const wstring TestOpenNLPLemmatizerFilterFactory::tokenizerModelFile =
    L"en-test-tokenizer.bin";
const wstring TestOpenNLPLemmatizerFilterFactory::sentenceModelFile =
    L"en-test-sent.bin";
const wstring TestOpenNLPLemmatizerFilterFactory::posTaggerModelFile =
    L"en-test-pos-maxent.bin";
const wstring TestOpenNLPLemmatizerFilterFactory::lemmatizerModelFile =
    L"en-test-lemmatizer.bin";
const wstring TestOpenNLPLemmatizerFilterFactory::lemmatizerDictFile =
    L"en-test-lemmas.dict";

void TestOpenNLPLemmatizerFilterFactory::test1SentenceDictionaryOnly() throw(
    runtime_error)
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp", {L"tokenizerModel", tokenizerModelFile,
                                       L"sentenceModel", sentenceModelFile})
          ->addTokenFilter(L"opennlpPOS",
                           {L"posTaggerModel", L"en-test-pos-maxent.bin"})
          ->addTokenFilter(L"opennlplemmatizer",
                           {L"dictionary", L"en-test-lemmas.dict"})
          ->build();
  assertAnalyzesTo(analyzer, SENTENCE, SENTENCE_dict_punc, nullptr, nullptr,
                   SENTENCE_posTags, nullptr, nullptr, true);
}

void TestOpenNLPLemmatizerFilterFactory::test2SentencesDictionaryOnly() throw(
    runtime_error)
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp", {L"tokenizerModel", tokenizerModelFile,
                                       L"sentenceModel", sentenceModelFile})
          ->addTokenFilter(L"opennlpPOS",
                           {L"posTaggerModel", posTaggerModelFile})
          ->addTokenFilter(L"opennlplemmatizer",
                           {L"dictionary", lemmatizerDictFile})
          ->build();
  assertAnalyzesTo(analyzer, SENTENCES, SENTENCES_dict_punc, nullptr, nullptr,
                   SENTENCES_posTags, nullptr, nullptr, true);
}

void TestOpenNLPLemmatizerFilterFactory::test1SentenceMaxEntOnly() throw(
    runtime_error)
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp", {L"tokenizerModel", tokenizerModelFile,
                                       L"sentenceModel", sentenceModelFile})
          ->addTokenFilter(L"opennlpPOS",
                           {L"posTaggerModel", posTaggerModelFile})
          ->addTokenFilter(L"opennlplemmatizer",
                           {L"lemmatizerModel", lemmatizerModelFile})
          ->build();
  assertAnalyzesTo(analyzer, SENTENCE, SENTENCE_maxent_punc, nullptr, nullptr,
                   SENTENCE_posTags, nullptr, nullptr, true);
}

void TestOpenNLPLemmatizerFilterFactory::test2SentencesMaxEntOnly() throw(
    runtime_error)
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp", {L"tokenizerModel", tokenizerModelFile,
                                       L"sentenceModel", sentenceModelFile})
          ->addTokenFilter(L"opennlpPOS",
                           {L"posTaggerModel", posTaggerModelFile})
          ->addTokenFilter(L"OpenNLPLemmatizer",
                           {L"lemmatizerModel", lemmatizerModelFile})
          ->build();
  assertAnalyzesTo(analyzer, SENTENCES, SENTENCES_maxent_punc, nullptr, nullptr,
                   SENTENCES_posTags, nullptr, nullptr, true);
}

void TestOpenNLPLemmatizerFilterFactory::
    test1SentenceDictionaryAndMaxEnt() 
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp", {L"tokenizerModel", tokenizerModelFile,
                                       L"sentenceModel", sentenceModelFile})
          ->addTokenFilter(L"opennlpPOS",
                           {L"posTaggerModel", L"en-test-pos-maxent.bin"})
          ->addTokenFilter(L"opennlplemmatizer",
                           {L"dictionary", L"en-test-lemmas.dict",
                            L"lemmatizerModel", lemmatizerModelFile})
          ->build();
  assertAnalyzesTo(analyzer, SENTENCE_both, SENTENCE_both_punc, nullptr,
                   nullptr, SENTENCE_both_posTags, nullptr, nullptr, true);
}

void TestOpenNLPLemmatizerFilterFactory::
    test2SentencesDictionaryAndMaxEnt() 
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp", {L"tokenizerModel", tokenizerModelFile,
                                       L"sentenceModel", sentenceModelFile})
          ->addTokenFilter(L"opennlpPOS",
                           {L"posTaggerModel", posTaggerModelFile})
          ->addTokenFilter(L"opennlplemmatizer",
                           {L"dictionary", lemmatizerDictFile,
                            L"lemmatizerModel", lemmatizerModelFile})
          ->build();
  assertAnalyzesTo(analyzer, SENTENCES_both, SENTENCES_both_punc, nullptr,
                   nullptr, SENTENCES_both_posTags, nullptr, nullptr, true);
}

void TestOpenNLPLemmatizerFilterFactory::
    testKeywordAttributeAwarenessDictionaryOnly() 
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp", {L"tokenizerModel", tokenizerModelFile,
                                       L"sentenceModel", sentenceModelFile})
          ->addTokenFilter(L"opennlpPOS",
                           {L"posTaggerModel", posTaggerModelFile})
          ->addTokenFilter(KeywordRepeatFilterFactory::typeid)
          ->addTokenFilter(L"opennlplemmatizer",
                           {L"dictionary", lemmatizerDictFile})
          ->addTokenFilter(RemoveDuplicatesTokenFilterFactory::typeid)
          ->build();
  assertAnalyzesTo(analyzer, SENTENCES, SENTENCES_dict_keep_orig_punc, nullptr,
                   nullptr, SENTENCES_keep_orig_posTags, nullptr, nullptr,
                   true);
}

void TestOpenNLPLemmatizerFilterFactory::
    testKeywordAttributeAwarenessMaxEntOnly() 
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp", {L"tokenizerModel", tokenizerModelFile,
                                       L"sentenceModel", sentenceModelFile})
          ->addTokenFilter(L"opennlpPOS",
                           {L"posTaggerModel", posTaggerModelFile})
          ->addTokenFilter(KeywordRepeatFilterFactory::typeid)
          ->addTokenFilter(L"opennlplemmatizer",
                           {L"lemmatizerModel", lemmatizerModelFile})
          ->addTokenFilter(RemoveDuplicatesTokenFilterFactory::typeid)
          ->build();
  assertAnalyzesTo(analyzer, SENTENCES, SENTENCES_max_ent_keep_orig_punc,
                   nullptr, nullptr, SENTENCES_keep_orig_posTags, nullptr,
                   nullptr, true);
}

void TestOpenNLPLemmatizerFilterFactory::
    testKeywordAttributeAwarenessDictionaryAndMaxEnt() 
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder(make_shared<ClasspathResourceLoader>(getClass()))
          ->withTokenizer(L"opennlp", {L"tokenizerModel", tokenizerModelFile,
                                       L"sentenceModel", sentenceModelFile})
          ->addTokenFilter(L"opennlpPOS",
                           {L"posTaggerModel", posTaggerModelFile})
          ->addTokenFilter(KeywordRepeatFilterFactory::typeid)
          ->addTokenFilter(L"opennlplemmatizer",
                           {L"dictionary", lemmatizerDictFile,
                            L"lemmatizerModel", lemmatizerModelFile})
          ->addTokenFilter(RemoveDuplicatesTokenFilterFactory::typeid)
          ->build();
  assertAnalyzesTo(analyzer, SENTENCES_both, SENTENCES_both_keep_orig_punc,
                   nullptr, nullptr, SENTENCES_both_keep_orig_posTags, nullptr,
                   nullptr, true);
}
} // namespace org::apache::lucene::analysis::opennlp