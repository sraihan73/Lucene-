using namespace std;

#include "CommonGramsFilterTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/commongrams/CommonGramsFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/commongrams/CommonGramsQueryFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/WhitespaceTokenizer.h"

namespace org::apache::lucene::analysis::commongrams
{
using namespace org::apache::lucene::analysis;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using WhitespaceTokenizer =
    org::apache::lucene::analysis::core::WhitespaceTokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    CommonGramsFilterTest::commonWords =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(L"s", L"a", L"b", L"c", L"d", L"the",
                                       L"of"),
            false);

void CommonGramsFilterTest::testReset() 
{
  const wstring input = L"How the s a brown s cow d like A B thing?";
  shared_ptr<WhitespaceTokenizer> wt = make_shared<WhitespaceTokenizer>();
  wt->setReader(make_shared<StringReader>(input));
  shared_ptr<CommonGramsFilter> cgf =
      make_shared<CommonGramsFilter>(wt, commonWords);

  shared_ptr<CharTermAttribute> term =
      cgf->addAttribute(CharTermAttribute::typeid);
  cgf->reset();
  assertTrue(cgf->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"How", term->toString());
  assertTrue(cgf->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"How_the", term->toString());
  assertTrue(cgf->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"the", term->toString());
  assertTrue(cgf->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"the_s", term->toString());
  delete cgf;

  wt->setReader(make_shared<StringReader>(input));
  cgf->reset();
  assertTrue(cgf->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"How", term->toString());
}

void CommonGramsFilterTest::testQueryReset() 
{
  const wstring input = L"How the s a brown s cow d like A B thing?";
  shared_ptr<WhitespaceTokenizer> wt = make_shared<WhitespaceTokenizer>();
  wt->setReader(make_shared<StringReader>(input));
  shared_ptr<CommonGramsFilter> cgf =
      make_shared<CommonGramsFilter>(wt, commonWords);
  shared_ptr<CommonGramsQueryFilter> nsf =
      make_shared<CommonGramsQueryFilter>(cgf);

  shared_ptr<CharTermAttribute> term =
      wt->addAttribute(CharTermAttribute::typeid);
  nsf->reset();
  assertTrue(nsf->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"How_the", term->toString());
  assertTrue(nsf->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"the_s", term->toString());
  delete nsf;

  wt->setReader(make_shared<StringReader>(input));
  nsf->reset();
  assertTrue(nsf->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"How_the", term->toString());
}

void CommonGramsFilterTest::testCommonGramsQueryFilter() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  // Stop words used below are "of" "the" and "s"

  // two word queries
  assertAnalyzesTo(a, L"brown fox", std::deque<wstring>{L"brown", L"fox"});
  assertAnalyzesTo(a, L"the fox", std::deque<wstring>{L"the_fox"});
  assertAnalyzesTo(a, L"fox of", std::deque<wstring>{L"fox_of"});
  assertAnalyzesTo(a, L"of the", std::deque<wstring>{L"of_the"});

  // one word queries
  assertAnalyzesTo(a, L"the", std::deque<wstring>{L"the"});
  assertAnalyzesTo(a, L"foo", std::deque<wstring>{L"foo"});

  // 3 word combinations s=stopword/common word n=not a stop word
  assertAnalyzesTo(a, L"n n n", std::deque<wstring>{L"n", L"n", L"n"});
  assertAnalyzesTo(a, L"quick brown fox",
                   std::deque<wstring>{L"quick", L"brown", L"fox"});

  assertAnalyzesTo(a, L"n n s", std::deque<wstring>{L"n", L"n_s"});
  assertAnalyzesTo(a, L"quick brown the",
                   std::deque<wstring>{L"quick", L"brown_the"});

  assertAnalyzesTo(a, L"n s n", std::deque<wstring>{L"n_s", L"s_n"});
  assertAnalyzesTo(a, L"quick the brown",
                   std::deque<wstring>{L"quick_the", L"the_brown"});

  assertAnalyzesTo(a, L"n s s", std::deque<wstring>{L"n_s", L"s_s"});
  assertAnalyzesTo(a, L"fox of the",
                   std::deque<wstring>{L"fox_of", L"of_the"});

  assertAnalyzesTo(a, L"s n n", std::deque<wstring>{L"s_n", L"n", L"n"});
  assertAnalyzesTo(a, L"the quick brown",
                   std::deque<wstring>{L"the_quick", L"quick", L"brown"});

  assertAnalyzesTo(a, L"s n s", std::deque<wstring>{L"s_n", L"n_s"});
  assertAnalyzesTo(a, L"the fox of",
                   std::deque<wstring>{L"the_fox", L"fox_of"});

  assertAnalyzesTo(a, L"s s n", std::deque<wstring>{L"s_s", L"s_n"});
  assertAnalyzesTo(a, L"of the fox",
                   std::deque<wstring>{L"of_the", L"the_fox"});

  assertAnalyzesTo(a, L"s s s", std::deque<wstring>{L"s_s", L"s_s"});
  assertAnalyzesTo(a, L"of the of", std::deque<wstring>{L"of_the", L"the_of"});
  delete a;
}

CommonGramsFilterTest::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<CommonGramsFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
CommonGramsFilterTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<CommonGramsQueryFilter>(
                     make_shared<CommonGramsFilter>(tokenizer, commonWords)));
}

void CommonGramsFilterTest::testCommonGramsFilter() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());

  // Stop words used below are "of" "the" and "s"
  // one word queries
  assertAnalyzesTo(a, L"the", std::deque<wstring>{L"the"});
  assertAnalyzesTo(a, L"foo", std::deque<wstring>{L"foo"});

  // two word queries
  assertAnalyzesTo(a, L"brown fox", std::deque<wstring>{L"brown", L"fox"},
                   std::deque<int>{1, 1});
  assertAnalyzesTo(a, L"the fox",
                   std::deque<wstring>{L"the", L"the_fox", L"fox"},
                   std::deque<int>{1, 0, 1});
  assertAnalyzesTo(a, L"fox of", std::deque<wstring>{L"fox", L"fox_of", L"of"},
                   std::deque<int>{1, 0, 1});
  assertAnalyzesTo(a, L"of the", std::deque<wstring>{L"of", L"of_the", L"the"},
                   std::deque<int>{1, 0, 1});

  // 3 word combinations s=stopword/common word n=not a stop word
  assertAnalyzesTo(a, L"n n n", std::deque<wstring>{L"n", L"n", L"n"},
                   std::deque<int>{1, 1, 1});
  assertAnalyzesTo(a, L"quick brown fox",
                   std::deque<wstring>{L"quick", L"brown", L"fox"},
                   std::deque<int>{1, 1, 1});

  assertAnalyzesTo(a, L"n n s", std::deque<wstring>{L"n", L"n", L"n_s", L"s"},
                   std::deque<int>{1, 1, 0, 1});
  assertAnalyzesTo(
      a, L"quick brown the",
      std::deque<wstring>{L"quick", L"brown", L"brown_the", L"the"},
      std::deque<int>{1, 1, 0, 1});

  assertAnalyzesTo(a, L"n s n",
                   std::deque<wstring>{L"n", L"n_s", L"s", L"s_n", L"n"},
                   std::deque<int>{1, 0, 1, 0, 1});
  assertAnalyzesTo(
      a, L"quick the fox",
      std::deque<wstring>{L"quick", L"quick_the", L"the", L"the_fox", L"fox"},
      std::deque<int>{1, 0, 1, 0, 1});

  assertAnalyzesTo(a, L"n s s",
                   std::deque<wstring>{L"n", L"n_s", L"s", L"s_s", L"s"},
                   std::deque<int>{1, 0, 1, 0, 1});
  assertAnalyzesTo(
      a, L"fox of the",
      std::deque<wstring>{L"fox", L"fox_of", L"of", L"of_the", L"the"},
      std::deque<int>{1, 0, 1, 0, 1});

  assertAnalyzesTo(a, L"s n n", std::deque<wstring>{L"s", L"s_n", L"n", L"n"},
                   std::deque<int>{1, 0, 1, 1});
  assertAnalyzesTo(
      a, L"the quick brown",
      std::deque<wstring>{L"the", L"the_quick", L"quick", L"brown"},
      std::deque<int>{1, 0, 1, 1});

  assertAnalyzesTo(a, L"s n s",
                   std::deque<wstring>{L"s", L"s_n", L"n", L"n_s", L"s"},
                   std::deque<int>{1, 0, 1, 0, 1});
  assertAnalyzesTo(
      a, L"the fox of",
      std::deque<wstring>{L"the", L"the_fox", L"fox", L"fox_of", L"of"},
      std::deque<int>{1, 0, 1, 0, 1});

  assertAnalyzesTo(a, L"s s n",
                   std::deque<wstring>{L"s", L"s_s", L"s", L"s_n", L"n"},
                   std::deque<int>{1, 0, 1, 0, 1});
  assertAnalyzesTo(
      a, L"of the fox",
      std::deque<wstring>{L"of", L"of_the", L"the", L"the_fox", L"fox"},
      std::deque<int>{1, 0, 1, 0, 1});

  assertAnalyzesTo(a, L"s s s",
                   std::deque<wstring>{L"s", L"s_s", L"s", L"s_s", L"s"},
                   std::deque<int>{1, 0, 1, 0, 1});
  assertAnalyzesTo(
      a, L"of the of",
      std::deque<wstring>{L"of", L"of_the", L"the", L"the_of", L"of"},
      std::deque<int>{1, 0, 1, 0, 1});
  delete a;
}

CommonGramsFilterTest::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<CommonGramsFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
CommonGramsFilterTest::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<CommonGramsFilter>(tokenizer, commonWords));
}

void CommonGramsFilterTest::testCaseSensitive() 
{
  const wstring input = L"How The s a brown s cow d like A B thing?";
  shared_ptr<MockTokenizer> wt =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  wt->setReader(make_shared<StringReader>(input));
  shared_ptr<TokenFilter> cgf = make_shared<CommonGramsFilter>(wt, commonWords);
  assertTokenStreamContents(
      cgf, std::deque<wstring>{L"How", L"The", L"The_s", L"s", L"s_a", L"a",
                                L"a_brown", L"brown", L"brown_s", L"s",
                                L"s_cow", L"cow", L"cow_d", L"d", L"d_like",
                                L"like", L"A", L"B", L"thing?"});
}

void CommonGramsFilterTest::testLastWordisStopWord() 
{
  const wstring input = L"dog the";
  shared_ptr<MockTokenizer> wt =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  wt->setReader(make_shared<StringReader>(input));
  shared_ptr<CommonGramsFilter> cgf =
      make_shared<CommonGramsFilter>(wt, commonWords);
  shared_ptr<TokenFilter> nsf = make_shared<CommonGramsQueryFilter>(cgf);
  assertTokenStreamContents(nsf, std::deque<wstring>{L"dog_the"});
}

void CommonGramsFilterTest::testFirstWordisStopWord() 
{
  const wstring input = L"the dog";
  shared_ptr<MockTokenizer> wt =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  wt->setReader(make_shared<StringReader>(input));
  shared_ptr<CommonGramsFilter> cgf =
      make_shared<CommonGramsFilter>(wt, commonWords);
  shared_ptr<TokenFilter> nsf = make_shared<CommonGramsQueryFilter>(cgf);
  assertTokenStreamContents(nsf, std::deque<wstring>{L"the_dog"});
}

void CommonGramsFilterTest::testOneWordQueryStopWord() 
{
  const wstring input = L"the";
  shared_ptr<MockTokenizer> wt =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  wt->setReader(make_shared<StringReader>(input));
  shared_ptr<CommonGramsFilter> cgf =
      make_shared<CommonGramsFilter>(wt, commonWords);
  shared_ptr<TokenFilter> nsf = make_shared<CommonGramsQueryFilter>(cgf);
  assertTokenStreamContents(nsf, std::deque<wstring>{L"the"});
}

void CommonGramsFilterTest::testOneWordQuery() 
{
  const wstring input = L"monster";
  shared_ptr<MockTokenizer> wt =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  wt->setReader(make_shared<StringReader>(input));
  shared_ptr<CommonGramsFilter> cgf =
      make_shared<CommonGramsFilter>(wt, commonWords);
  shared_ptr<TokenFilter> nsf = make_shared<CommonGramsQueryFilter>(cgf);
  assertTokenStreamContents(nsf, std::deque<wstring>{L"monster"});
}

void CommonGramsFilterTest::TestFirstAndLastStopWord() 
{
  const wstring input = L"the of";
  shared_ptr<MockTokenizer> wt =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  wt->setReader(make_shared<StringReader>(input));
  shared_ptr<CommonGramsFilter> cgf =
      make_shared<CommonGramsFilter>(wt, commonWords);
  shared_ptr<TokenFilter> nsf = make_shared<CommonGramsQueryFilter>(cgf);
  assertTokenStreamContents(nsf, std::deque<wstring>{L"the_of"});
}

void CommonGramsFilterTest::testRandomStrings() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());

  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;

  shared_ptr<Analyzer> b =
      make_shared<AnalyzerAnonymousInnerClass4>(shared_from_this());

  checkRandomData(random(), b, 1000 * RANDOM_MULTIPLIER);
  delete b;
}

CommonGramsFilterTest::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<CommonGramsFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
CommonGramsFilterTest::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> t =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<CommonGramsFilter> cgf =
      make_shared<CommonGramsFilter>(t, commonWords);
  return make_shared<TokenStreamComponents>(t, cgf);
}

CommonGramsFilterTest::AnalyzerAnonymousInnerClass4::
    AnalyzerAnonymousInnerClass4(
        shared_ptr<CommonGramsFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
CommonGramsFilterTest::AnalyzerAnonymousInnerClass4::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> t =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<CommonGramsFilter> cgf =
      make_shared<CommonGramsFilter>(t, commonWords);
  return make_shared<TokenStreamComponents>(
      t, make_shared<CommonGramsQueryFilter>(cgf));
}
} // namespace org::apache::lucene::analysis::commongrams