using namespace std;

#include "TestKoreanTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockGraphTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/KoreanReadingFormFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/KoreanTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/dict/UserDictionary.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/tokenattributes/PartOfSpeechAttribute.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/tokenattributes/ReadingAttribute.h"

namespace org::apache::lucene::analysis::ko
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockGraphTokenFilter =
    org::apache::lucene::analysis::MockGraphTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using DecompoundMode =
    org::apache::lucene::analysis::ko::KoreanTokenizer::DecompoundMode;
using UserDictionary = org::apache::lucene::analysis::ko::dict::UserDictionary;
using PartOfSpeechAttribute =
    org::apache::lucene::analysis::ko::tokenattributes::PartOfSpeechAttribute;
using ReadingAttribute =
    org::apache::lucene::analysis::ko::tokenattributes::ReadingAttribute;

shared_ptr<UserDictionary> TestKoreanTokenizer::readDict()
{
  shared_ptr<InputStream> is =
      TestKoreanTokenizer::typeid->getResourceAsStream(L"userdict.txt");
  if (is == nullptr) {
    throw runtime_error(L"Cannot find userdict.txt in test classpath!");
  }
  try {
    try {
      shared_ptr<Reader> reader =
          make_shared<InputStreamReader>(is, StandardCharsets::UTF_8);
      return UserDictionary::open(reader);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      is->close();
    }
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

void TestKoreanTokenizer::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  shared_ptr<UserDictionary> userDictionary = readDict();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(),
                                                      userDictionary);
  analyzerUnigram = make_shared<AnalyzerAnonymousInnerClass2>(
      shared_from_this(), userDictionary);
  analyzerDecompound = make_shared<AnalyzerAnonymousInnerClass3>(
      shared_from_this(), userDictionary);
  analyzerDecompoundKeep = make_shared<AnalyzerAnonymousInnerClass4>(
      shared_from_this(), userDictionary);
  analyzerReading = make_shared<AnalyzerAnonymousInnerClass5>(
      shared_from_this(), userDictionary);
}

TestKoreanTokenizer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestKoreanTokenizer> outerInstance,
    shared_ptr<UserDictionary> userDictionary)
{
  this->outerInstance = outerInstance;
  this->userDictionary = userDictionary;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestKoreanTokenizer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KoreanTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), userDictionary,
      DecompoundMode::NONE, false);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

TestKoreanTokenizer::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<TestKoreanTokenizer> outerInstance,
    shared_ptr<UserDictionary> userDictionary)
{
  this->outerInstance = outerInstance;
  this->userDictionary = userDictionary;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestKoreanTokenizer::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KoreanTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), userDictionary,
      DecompoundMode::NONE, true);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

TestKoreanTokenizer::AnalyzerAnonymousInnerClass3::AnalyzerAnonymousInnerClass3(
    shared_ptr<TestKoreanTokenizer> outerInstance,
    shared_ptr<UserDictionary> userDictionary)
{
  this->outerInstance = outerInstance;
  this->userDictionary = userDictionary;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestKoreanTokenizer::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KoreanTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), userDictionary,
      DecompoundMode::DISCARD, false);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer);
}

TestKoreanTokenizer::AnalyzerAnonymousInnerClass4::AnalyzerAnonymousInnerClass4(
    shared_ptr<TestKoreanTokenizer> outerInstance,
    shared_ptr<UserDictionary> userDictionary)
{
  this->outerInstance = outerInstance;
  this->userDictionary = userDictionary;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestKoreanTokenizer::AnalyzerAnonymousInnerClass4::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KoreanTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), userDictionary,
      DecompoundMode::MIXED, false);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer);
}

TestKoreanTokenizer::AnalyzerAnonymousInnerClass5::AnalyzerAnonymousInnerClass5(
    shared_ptr<TestKoreanTokenizer> outerInstance,
    shared_ptr<UserDictionary> userDictionary)
{
  this->outerInstance = outerInstance;
  this->userDictionary = userDictionary;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestKoreanTokenizer::AnalyzerAnonymousInnerClass5::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KoreanTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), userDictionary,
      DecompoundMode::NONE, false);
  shared_ptr<KoreanReadingFormFilter> filter =
      make_shared<KoreanReadingFormFilter>(tokenizer);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}

void TestKoreanTokenizer::testSpaces() 
{
  assertAnalyzesTo(analyzer, L"화학        이외의         것",
                   std::deque<wstring>{L"화학", L"이외", L"의", L"것"},
                   std::deque<int>{0, 10, 12, 22},
                   std::deque<int>{2, 12, 13, 23},
                   std::deque<int>{1, 1, 1, 1});
  assertPartsOfSpeech(
      analyzer, L"화학 이외의         것",
      std::deque<POS::Type>{POS::Type::MORPHEME, POS::Type::MORPHEME,
                             POS::Type::MORPHEME, POS::Type::MORPHEME},
      std::deque<POS::Tag>{POS::Tag::NNG, POS::Tag::NNG, POS::Tag::J,
                            POS::Tag::NNB},
      std::deque<POS::Tag>{POS::Tag::NNG, POS::Tag::NNG, POS::Tag::J,
                            POS::Tag::NNB});
}

void TestKoreanTokenizer::testPartOfSpeechs() 
{
  assertAnalyzesTo(analyzer, L"화학 이외의 것",
                   std::deque<wstring>{L"화학", L"이외", L"의", L"것"},
                   std::deque<int>{0, 3, 5, 7}, std::deque<int>{2, 5, 6, 8},
                   std::deque<int>{1, 1, 1, 1});
  assertPartsOfSpeech(
      analyzer, L"화학 이외의 것",
      std::deque<POS::Type>{POS::Type::MORPHEME, POS::Type::MORPHEME,
                             POS::Type::MORPHEME, POS::Type::MORPHEME},
      std::deque<POS::Tag>{POS::Tag::NNG, POS::Tag::NNG, POS::Tag::J,
                            POS::Tag::NNB},
      std::deque<POS::Tag>{POS::Tag::NNG, POS::Tag::NNG, POS::Tag::J,
                            POS::Tag::NNB});
}

void TestKoreanTokenizer::testPartOfSpeechsWithCompound() 
{
  assertAnalyzesTo(
      analyzer, L"가락지나물은 한국, 중국, 일본",
      std::deque<wstring>{L"가락지나물", L"은", L"한국", L"중국", L"일본"},
      std::deque<int>{0, 5, 7, 11, 15}, std::deque<int>{5, 6, 9, 13, 17},
      std::deque<int>{1, 1, 1, 1, 1});

  assertPartsOfSpeech(
      analyzer, L"가락지나물은 한국, 중국, 일본",
      std::deque<POS::Type>{POS::Type::COMPOUND, POS::Type::MORPHEME,
                             POS::Type::MORPHEME, POS::Type::MORPHEME,
                             POS::Type::MORPHEME},
      std::deque<POS::Tag>{POS::Tag::NNG, POS::Tag::J, POS::Tag::NNP,
                            POS::Tag::NNP, POS::Tag::NNP},
      std::deque<POS::Tag>{POS::Tag::NNG, POS::Tag::J, POS::Tag::NNP,
                            POS::Tag::NNP, POS::Tag::NNP});

  assertAnalyzesTo(analyzerDecompound, L"가락지나물은 한국, 중국, 일본",
                   std::deque<wstring>{L"가락지", L"나물", L"은", L"한국",
                                        L"중국", L"일본"},
                   std::deque<int>{0, 3, 5, 7, 11, 15},
                   std::deque<int>{3, 5, 6, 9, 13, 17},
                   std::deque<int>{1, 1, 1, 1, 1, 1});

  assertAnalyzesTo(analyzerDecompoundKeep, L"가락지나물은 한국, 중국, 일본",
                   std::deque<wstring>{L"가락지나물", L"가락지", L"나물",
                                        L"은", L"한국", L"중국", L"일본"},
                   std::deque<int>{0, 0, 3, 5, 7, 11, 15},
                   std::deque<int>{5, 3, 5, 6, 9, 13, 17}, nullptr,
                   std::deque<int>{1, 0, 1, 1, 1, 1, 1},
                   std::deque<int>{2, 1, 1, 1, 1, 1, 1});

  assertPartsOfSpeech(
      analyzerDecompound, L"가락지나물은 한국, 중국, 일본",
      std::deque<POS::Type>{POS::Type::MORPHEME, POS::Type::MORPHEME,
                             POS::Type::MORPHEME, POS::Type::MORPHEME,
                             POS::Type::MORPHEME, POS::Type::MORPHEME},
      std::deque<POS::Tag>{POS::Tag::NNG, POS::Tag::NNG, POS::Tag::J,
                            POS::Tag::NNP, POS::Tag::NNP, POS::Tag::NNP},
      std::deque<POS::Tag>{POS::Tag::NNG, POS::Tag::NNG, POS::Tag::J,
                            POS::Tag::NNP, POS::Tag::NNP, POS::Tag::NNP});

  assertPartsOfSpeech(
      analyzerDecompoundKeep, L"가락지나물은 한국, 중국, 일본",
      std::deque<POS::Type>{POS::Type::COMPOUND, POS::Type::MORPHEME,
                             POS::Type::MORPHEME, POS::Type::MORPHEME,
                             POS::Type::MORPHEME, POS::Type::MORPHEME,
                             POS::Type::MORPHEME},
      std::deque<POS::Tag>{POS::Tag::NNG, POS::Tag::NNG, POS::Tag::NNG,
                            POS::Tag::J, POS::Tag::NNP, POS::Tag::NNP,
                            POS::Tag::NNP},
      std::deque<POS::Tag>{POS::Tag::NNG, POS::Tag::NNG, POS::Tag::NNG,
                            POS::Tag::J, POS::Tag::NNP, POS::Tag::NNP,
                            POS::Tag::NNP});
}

void TestKoreanTokenizer::testPartOfSpeechsWithInflects() 
{
  assertAnalyzesTo(analyzer, L"감싸여", std::deque<wstring>{L"감싸여"},
                   std::deque<int>{0}, std::deque<int>{3},
                   std::deque<int>{1});

  assertPartsOfSpeech(
      analyzer, L"감싸여", std::deque<POS::Type>{POS::Type::INFLECT},
      std::deque<POS::Tag>{POS::Tag::VV}, std::deque<POS::Tag>{POS::Tag::E});

  assertAnalyzesTo(
      analyzerDecompound, L"감싸여", std::deque<wstring>{L"감싸이", L"어"},
      std::deque<int>{0, 0}, std::deque<int>{3, 3}, std::deque<int>{1, 1});

  assertAnalyzesTo(analyzerDecompoundKeep, L"감싸여",
                   std::deque<wstring>{L"감싸여", L"감싸이", L"어"},
                   std::deque<int>{0, 0, 0}, std::deque<int>{3, 3, 3},
                   nullptr, std::deque<int>{1, 0, 1},
                   std::deque<int>{2, 1, 1});

  assertPartsOfSpeech(
      analyzerDecompound, L"감싸여",
      std::deque<POS::Type>{POS::Type::MORPHEME, POS::Type::MORPHEME},
      std::deque<POS::Tag>{POS::Tag::VV, POS::Tag::E},
      std::deque<POS::Tag>{POS::Tag::VV, POS::Tag::E});

  assertPartsOfSpeech(
      analyzerDecompoundKeep, L"감싸여",
      std::deque<POS::Type>{POS::Type::INFLECT, POS::Type::MORPHEME,
                             POS::Type::MORPHEME},
      std::deque<POS::Tag>{POS::Tag::VV, POS::Tag::VV, POS::Tag::E},
      std::deque<POS::Tag>{POS::Tag::E, POS::Tag::VV, POS::Tag::E});
}

void TestKoreanTokenizer::testUnknownWord() 
{
  assertAnalyzesTo(
      analyzer, L"2018 평창 동계올림픽대회",
      std::deque<wstring>{L"2018", L"평창", L"동계", L"올림픽", L"대회"},
      std::deque<int>{0, 5, 8, 10, 13}, std::deque<int>{4, 7, 10, 13, 15},
      std::deque<int>{1, 1, 1, 1, 1});

  assertPartsOfSpeech(
      analyzer, L"2018 평창 동계올림픽대회",
      std::deque<POS::Type>{POS::Type::MORPHEME, POS::Type::MORPHEME,
                             POS::Type::MORPHEME, POS::Type::MORPHEME,
                             POS::Type::MORPHEME},
      std::deque<POS::Tag>{POS::Tag::SN, POS::Tag::NNP, POS::Tag::NNP,
                            POS::Tag::NNP, POS::Tag::NNG},
      std::deque<POS::Tag>{POS::Tag::SN, POS::Tag::NNP, POS::Tag::NNP,
                            POS::Tag::NNP, POS::Tag::NNG});

  assertAnalyzesTo(analyzerUnigram, L"2018 평창 동계올림픽대회",
                   std::deque<wstring>{L"2", L"0", L"1", L"8", L"평창",
                                        L"동계", L"올림픽", L"대회"},
                   std::deque<int>{0, 1, 2, 3, 5, 8, 10, 13},
                   std::deque<int>{1, 2, 3, 4, 7, 10, 13, 15},
                   std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1});

  assertPartsOfSpeech(
      analyzerUnigram, L"2018 평창 동계올림픽대회",
      std::deque<POS::Type>{POS::Type::MORPHEME, POS::Type::MORPHEME,
                             POS::Type::MORPHEME, POS::Type::MORPHEME,
                             POS::Type::MORPHEME, POS::Type::MORPHEME,
                             POS::Type::MORPHEME, POS::Type::MORPHEME},
      std::deque<POS::Tag>{POS::Tag::SY, POS::Tag::SY, POS::Tag::SY,
                            POS::Tag::SY, POS::Tag::NNP, POS::Tag::NNP,
                            POS::Tag::NNP, POS::Tag::NNG},
      std::deque<POS::Tag>{POS::Tag::SY, POS::Tag::SY, POS::Tag::SY,
                            POS::Tag::SY, POS::Tag::NNP, POS::Tag::NNP,
                            POS::Tag::NNP, POS::Tag::NNG});
}

void TestKoreanTokenizer::testReading() 
{
  assertReadings(analyzer, L"喜悲哀歡", {L"희비애환"});
  assertReadings(analyzer, L"五朔居廬", {L"오삭거려"});
  assertReadings(analyzer, L"가늘라", std::deque<wstring>{L""});
  assertAnalyzesTo(analyzerReading, L"喜悲哀歡",
                   std::deque<wstring>{L"희비애환"}, std::deque<int>{0},
                   std::deque<int>{4}, std::deque<int>{1});
  assertAnalyzesTo(analyzerReading, L"五朔居廬",
                   std::deque<wstring>{L"오삭거려"}, std::deque<int>{0},
                   std::deque<int>{4}, std::deque<int>{1});
  assertAnalyzesTo(analyzerReading, L"가늘라", std::deque<wstring>{L"가늘라"},
                   std::deque<int>{0}, std::deque<int>{3},
                   std::deque<int>{1});
}

void TestKoreanTokenizer::testUserDict() 
{
  assertAnalyzesTo(analyzer, L"c++ 프로그래밍 언어",
                   std::deque<wstring>{L"c++", L"프로그래밍", L"언어"},
                   std::deque<int>{0, 4, 10}, std::deque<int>{3, 9, 12},
                   std::deque<int>{1, 1, 1});

  assertPartsOfSpeech(
      analyzer, L"c++ 프로그래밍 언어",
      std::deque<POS::Type>{POS::Type::MORPHEME, POS::Type::MORPHEME,
                             POS::Type::MORPHEME},
      std::deque<POS::Tag>{POS::Tag::NNG, POS::Tag::NNG, POS::Tag::NNG},
      std::deque<POS::Tag>{POS::Tag::NNG, POS::Tag::NNG, POS::Tag::NNG});

  assertAnalyzesTo(analyzerDecompound, L"정부세종청사",
                   std::deque<wstring>{L"정부", L"세종", L"청사"},
                   std::deque<int>{0, 2, 4}, std::deque<int>{2, 4, 6},
                   std::deque<int>{1, 1, 1});

  assertPartsOfSpeech(
      analyzerDecompound, L"정부세종청사",
      std::deque<POS::Type>{POS::Type::MORPHEME, POS::Type::MORPHEME,
                             POS::Type::MORPHEME},
      std::deque<POS::Tag>{POS::Tag::NNG, POS::Tag::NNG, POS::Tag::NNG},
      std::deque<POS::Tag>{POS::Tag::NNG, POS::Tag::NNG, POS::Tag::NNG});
}

void TestKoreanTokenizer::testRandomStrings() 
{
  checkRandomData(random(), analyzer, 500 * RANDOM_MULTIPLIER);
  checkRandomData(random(), analyzerUnigram, 500 * RANDOM_MULTIPLIER);
  checkRandomData(random(), analyzerDecompound, 500 * RANDOM_MULTIPLIER);
}

void TestKoreanTokenizer::testRandomHugeStrings() 
{
  shared_ptr<Random> random = TestKoreanTokenizer::random();
  checkRandomData(random, analyzer, 20 * RANDOM_MULTIPLIER, 8192);
  checkRandomData(random, analyzerUnigram, 20 * RANDOM_MULTIPLIER, 8192);
  checkRandomData(random, analyzerDecompound, 20 * RANDOM_MULTIPLIER, 8192);
}

void TestKoreanTokenizer::testRandomHugeStringsMockGraphAfter() throw(
    runtime_error)
{
  // Randomly inject graph tokens after KoreanTokenizer:
  shared_ptr<Random> random = TestKoreanTokenizer::random();
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass6>(shared_from_this());
  checkRandomData(random, analyzer, 20 * RANDOM_MULTIPLIER, 8192);
  delete analyzer;
}

TestKoreanTokenizer::AnalyzerAnonymousInnerClass6::AnalyzerAnonymousInnerClass6(
    shared_ptr<TestKoreanTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestKoreanTokenizer::AnalyzerAnonymousInnerClass6::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KoreanTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), nullptr,
      DecompoundMode::MIXED, false);
  shared_ptr<TokenStream> graph =
      make_shared<MockGraphTokenFilter>(random(), tokenizer);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, graph);
}

void TestKoreanTokenizer::assertReadings(
    shared_ptr<Analyzer> analyzer, const wstring &input,
    deque<wstring> &readings) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream("ignored", input))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(L"ignored", input);
    shared_ptr<ReadingAttribute> readingAtt =
        ts->addAttribute(ReadingAttribute::typeid);
    ts->reset();
    for (wstring reading : readings) {
      assertTrue(ts->incrementToken());
      assertEquals(reading, readingAtt->getReading());
    }
    assertFalse(ts->incrementToken());
    ts->end();
  }
}

void TestKoreanTokenizer::assertPartsOfSpeech(
    shared_ptr<Analyzer> analyzer, const wstring &input,
    std::deque<POS::Type> &posTypes, std::deque<POS::Tag> &leftPosTags,
    std::deque<POS::Tag> &rightPosTags) 
{
  assert(posTypes.size() == leftPosTags.size() &&
         posTypes.size() == rightPosTags.size());
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream("ignored", input))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(L"ignored", input);
    shared_ptr<PartOfSpeechAttribute> partOfSpeechAtt =
        ts->addAttribute(PartOfSpeechAttribute::typeid);
    ts->reset();
    for (int i = 0; i < posTypes.size(); i++) {
      POS::Type posType = posTypes[i];
      POS::Tag leftTag = leftPosTags[i];
      POS::Tag rightTag = rightPosTags[i];
      assertTrue(ts->incrementToken());
      assertEquals(posType, partOfSpeechAtt->getPOSType());
      assertEquals(leftTag, partOfSpeechAtt->getLeftPOS());
      assertEquals(rightTag, partOfSpeechAtt->getRightPOS());
    }
    assertFalse(ts->incrementToken());
    ts->end();
  }
}
} // namespace org::apache::lucene::analysis::ko