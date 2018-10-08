using namespace std;

#include "TestMorfologikAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/morfologik/MorfologikFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/morfologik/MorphosyntacticTagsAttribute.h"

namespace org::apache::lucene::analysis::morfologik
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

shared_ptr<Analyzer> TestMorfologikAnalyzer::getTestAnalyzer()
{
  return make_shared<MorfologikAnalyzer>();
}

void TestMorfologikAnalyzer::testSingleTokens() 
{
  shared_ptr<Analyzer> a = getTestAnalyzer();
  assertAnalyzesTo(a, L"a", std::deque<wstring>{L"a"});
  assertAnalyzesTo(a, L"liście",
                   std::deque<wstring>{L"liście", L"liść", L"deque", L"lista"});
  assertAnalyzesTo(a, L"danych",
                   std::deque<wstring>{L"dany", L"dana", L"dane", L"dać"});
  assertAnalyzesTo(a, L"ęóąśłżźćń", std::deque<wstring>{L"ęóąśłżźćń"});
  delete a;
}

void TestMorfologikAnalyzer::testMultipleTokens() 
{
  shared_ptr<Analyzer> a = getTestAnalyzer();
  assertAnalyzesTo(a, L"liście danych",
                   std::deque<wstring>{L"liście", L"liść", L"deque", L"lista",
                                        L"dany", L"dana", L"dane", L"dać"},
                   std::deque<int>{0, 0, 0, 0, 7, 7, 7, 7},
                   std::deque<int>{6, 6, 6, 6, 13, 13, 13, 13},
                   std::deque<int>{1, 0, 0, 0, 1, 0, 0, 0});

  assertAnalyzesTo(a, L"T. Gl\u00FCcksberg",
                   std::deque<wstring>{L"tom", L"tona", L"Gl\u00FCcksberg"},
                   std::deque<int>{0, 0, 3}, std::deque<int>{1, 1, 13},
                   std::deque<int>{1, 0, 1});
  delete a;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unused") private void dumpTokens(std::wstring
// input) throws java.io.IOException
void TestMorfologikAnalyzer::dumpTokens(const wstring &input) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.Analyzer a =
  // getTestAnalyzer(); org.apache.lucene.analysis.TokenStream ts =
  // a.tokenStream("dummy", input))
  {
    org::apache::lucene::analysis::Analyzer a = getTestAnalyzer();
    org::apache::lucene::analysis::TokenStream ts =
        a->tokenStream(L"dummy", input);
    ts->reset();

    shared_ptr<MorphosyntacticTagsAttribute> attribute =
        ts->getAttribute(MorphosyntacticTagsAttribute::typeid);
    shared_ptr<CharTermAttribute> charTerm =
        ts->getAttribute(CharTermAttribute::typeid);
    while (ts->incrementToken()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wcout << charTerm->toString() << L" => " << attribute->getTags() << endl;
    }
    ts->end();
  }
}

void TestMorfologikAnalyzer::testLeftoverStems() 
{
  shared_ptr<Analyzer> a = getTestAnalyzer();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts_1
  // = a.tokenStream("dummy", "liście"))
  {
    org::apache::lucene::analysis::TokenStream ts_1 =
        a->tokenStream(L"dummy", L"liście");
    shared_ptr<CharTermAttribute> termAtt_1 =
        ts_1->getAttribute(CharTermAttribute::typeid);
    ts_1->reset();
    ts_1->incrementToken();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"first stream", L"liście", termAtt_1->toString());
    ts_1->end();
  }

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts_2
  // = a.tokenStream("dummy", "danych"))
  {
    org::apache::lucene::analysis::TokenStream ts_2 =
        a->tokenStream(L"dummy", L"danych");
    shared_ptr<CharTermAttribute> termAtt_2 =
        ts_2->getAttribute(CharTermAttribute::typeid);
    ts_2->reset();
    ts_2->incrementToken();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"second stream", L"dany", termAtt_2->toString());
    ts_2->end();
  }
  delete a;
}

void TestMorfologikAnalyzer::testCase() 
{
  shared_ptr<Analyzer> a = getTestAnalyzer();

  assertAnalyzesTo(
      a, L"AGD",
      std::deque<wstring>{L"AGD", L"artykuły gospodarstwa domowego"});
  assertAnalyzesTo(a, L"agd",
                   std::deque<wstring>{L"artykuły gospodarstwa domowego"});

  assertAnalyzesTo(a, L"Poznania", std::deque<wstring>{L"Poznań"});
  assertAnalyzesTo(a, L"poznania",
                   std::deque<wstring>{L"poznanie", L"poznać"});

  assertAnalyzesTo(a, L"Aarona", std::deque<wstring>{L"Aaron"});
  assertAnalyzesTo(a, L"aarona", std::deque<wstring>{L"aarona"});

  assertAnalyzesTo(a, L"Liście",
                   std::deque<wstring>{L"liście", L"liść", L"deque", L"lista"});
  delete a;
}

void TestMorfologikAnalyzer::assertPOSToken(
    shared_ptr<TokenStream> ts, const wstring &term,
    deque<wstring> &tags) 
{
  ts->incrementToken();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(term, ts->getAttribute(CharTermAttribute::typeid)->toString());

  set<wstring> actual = set<wstring>();
  set<wstring> expected = set<wstring>();
  for (shared_ptr<StringBuilder> b :
       ts->getAttribute(MorphosyntacticTagsAttribute::typeid)->getTags()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    actual.insert(b->toString());
  }
  for (wstring s : tags) {
    expected.insert(s);
  }

  if (!expected.equals(actual)) {
    wcout << L"Expected:\n" << expected << endl;
    wcout << L"Actual:\n" << actual << endl;
    assertEquals(expected, actual);
  }
}

void TestMorfologikAnalyzer::testPOSAttribute() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.Analyzer a =
  // getTestAnalyzer(); org.apache.lucene.analysis.TokenStream ts =
  // a.tokenStream("dummy", "liście"))
  {
    org::apache::lucene::analysis::Analyzer a = getTestAnalyzer();
    org::apache::lucene::analysis::TokenStream ts =
        a->tokenStream(L"dummy", L"liście");
    ts->reset();
    assertPOSToken(
        ts, L"liście",
        {L"subst:sg:acc:n2", L"subst:sg:nom:n2", L"subst:sg:voc:n2"});

    assertPOSToken(
        ts, L"liść",
        {L"subst:pl:acc:m3", L"subst:pl:nom:m3", L"subst:pl:voc:m3"});

    assertPOSToken(ts, L"deque", {L"subst:sg:loc:m3", L"subst:sg:voc:m3"});

    assertPOSToken(ts, L"lista", {L"subst:sg:dat:f", L"subst:sg:loc:f"});
    ts->end();
  }
}

void TestMorfologikAnalyzer::testKeywordAttrTokens() 
{
  shared_ptr<Analyzer> a =
      make_shared<MorfologikAnalyzerAnonymousInnerClass>(shared_from_this());

  assertAnalyzesTo(
      a, L"liście danych",
      std::deque<wstring>{L"liście", L"dany", L"dana", L"dane", L"dać"},
      std::deque<int>{0, 7, 7, 7, 7}, std::deque<int>{6, 13, 13, 13, 13},
      std::deque<int>{1, 1, 0, 0, 0});
  delete a;
}

TestMorfologikAnalyzer::MorfologikAnalyzerAnonymousInnerClass::
    MorfologikAnalyzerAnonymousInnerClass(
        shared_ptr<TestMorfologikAnalyzer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestMorfologikAnalyzer::MorfologikAnalyzerAnonymousInnerClass::createComponents(
    const wstring &field)
{
  shared_ptr<CharArraySet> *const keywords =
      make_shared<CharArraySet>(1, false);
  keywords->add(L"liście");

  shared_ptr<Tokenizer> *const src = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(src);
  result = make_shared<SetKeywordMarkerFilter>(result, keywords);
  result = make_shared<MorfologikFilter>(result);

  return make_shared<Analyzer::TokenStreamComponents>(src, result);
}

void TestMorfologikAnalyzer::testRandom() 
{
  shared_ptr<Analyzer> a = getTestAnalyzer();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}
} // namespace org::apache::lucene::analysis::morfologik