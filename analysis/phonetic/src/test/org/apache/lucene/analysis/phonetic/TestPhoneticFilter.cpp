using namespace std;

#include "TestPhoneticFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/phonetic/PhoneticFilter.h"

namespace org::apache::lucene::analysis::phonetic
{
using org::apache::commons::codec::Encoder;
using org::apache::commons::codec::language::Caverphone2;
using org::apache::commons::codec::language::DoubleMetaphone;
using org::apache::commons::codec::language::Metaphone;
using org::apache::commons::codec::language::Nysiis;
using org::apache::commons::codec::language::RefinedSoundex;
using org::apache::commons::codec::language::Soundex;
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestPhoneticFilter::testAlgorithms() 
{
  assertAlgorithm(make_shared<Metaphone>(), true, L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A", L"aaa", L"B", L"bbb", L"KKK",
                                       L"ccc", L"ESKS", L"easgasg"});
  assertAlgorithm(make_shared<Metaphone>(), false, L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A", L"B", L"KKK", L"ESKS"});

  assertAlgorithm(make_shared<DoubleMetaphone>(), true, L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A", L"aaa", L"PP", L"bbb", L"KK",
                                       L"ccc", L"ASKS", L"easgasg"});
  assertAlgorithm(make_shared<DoubleMetaphone>(), false, L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A", L"PP", L"KK", L"ASKS"});

  assertAlgorithm(make_shared<Soundex>(), true, L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A000", L"aaa", L"B000", L"bbb",
                                       L"C000", L"ccc", L"E220", L"easgasg"});
  assertAlgorithm(make_shared<Soundex>(), false, L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A000", L"B000", L"C000", L"E220"});

  assertAlgorithm(make_shared<RefinedSoundex>(), true, L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A0", L"aaa", L"B1", L"bbb", L"C3",
                                       L"ccc", L"E034034", L"easgasg"});
  assertAlgorithm(make_shared<RefinedSoundex>(), false, L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A0", L"B1", L"C3", L"E034034"});

  assertAlgorithm(
      make_shared<Caverphone2>(), true, L"Darda Karleen Datha Carlene",
      std::deque<wstring>{L"TTA1111111", L"Darda", L"KLN1111111", L"Karleen",
                           L"TTA1111111", L"Datha", L"KLN1111111", L"Carlene"});
  assertAlgorithm(make_shared<Caverphone2>(), false,
                  L"Darda Karleen Datha Carlene",
                  std::deque<wstring>{L"TTA1111111", L"KLN1111111",
                                       L"TTA1111111", L"KLN1111111"});

  assertAlgorithm(make_shared<Nysiis>(), true, L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A", L"aaa", L"B", L"bbb", L"C", L"ccc",
                                       L"EASGAS", L"easgasg"});
  assertAlgorithm(make_shared<Nysiis>(), false, L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A", L"B", L"C", L"EASGAS"});
}

void TestPhoneticFilter::assertAlgorithm(
    shared_ptr<Encoder> encoder, bool inject, const wstring &input,
    std::deque<wstring> &expected) 
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setReader(make_shared<StringReader>(input));
  shared_ptr<PhoneticFilter> filter =
      make_shared<PhoneticFilter>(tokenizer, encoder, inject);
  assertTokenStreamContents(filter, expected);
}

void TestPhoneticFilter::testRandomStrings() 
{
  std::deque<std::shared_ptr<Encoder>> encoders = {
      make_shared<Metaphone>(), make_shared<DoubleMetaphone>(),
      make_shared<Soundex>(), make_shared<RefinedSoundex>(),
      make_shared<Caverphone2>()};

  for (auto e : encoders) {
    shared_ptr<Analyzer> a =
        make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), e);

    checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
    delete a;

    shared_ptr<Analyzer> b =
        make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this(), e);

    checkRandomData(random(), b, 1000 * RANDOM_MULTIPLIER);
    delete b;
  }
}

TestPhoneticFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestPhoneticFilter> outerInstance, shared_ptr<Encoder> e)
{
  this->outerInstance = outerInstance;
  this->e = e;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPhoneticFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<PhoneticFilter>(tokenizer, e, false));
}

TestPhoneticFilter::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<TestPhoneticFilter> outerInstance, shared_ptr<Encoder> e)
{
  this->outerInstance = outerInstance;
  this->e = e;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPhoneticFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<PhoneticFilter>(tokenizer, e, false));
}

void TestPhoneticFilter::testEmptyTerm() 
{
  std::deque<std::shared_ptr<Encoder>> encoders = {
      make_shared<Metaphone>(), make_shared<DoubleMetaphone>(),
      make_shared<Soundex>(), make_shared<RefinedSoundex>(),
      make_shared<Caverphone2>()};
  for (auto e : encoders) {
    shared_ptr<Analyzer> a =
        make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this(), e);
    checkOneTerm(a, L"", L"");
    delete a;
  }
}

TestPhoneticFilter::AnalyzerAnonymousInnerClass3::AnalyzerAnonymousInnerClass3(
    shared_ptr<TestPhoneticFilter> outerInstance, shared_ptr<Encoder> e)
{
  this->outerInstance = outerInstance;
  this->e = e;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPhoneticFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer,
      make_shared<PhoneticFilter>(tokenizer, e, random()->nextBoolean()));
}
} // namespace org::apache::lucene::analysis::phonetic