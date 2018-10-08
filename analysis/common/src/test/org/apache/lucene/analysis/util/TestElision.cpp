using namespace std;

#include "TestElision.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/fr/FrenchAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/ElisionFilter.h"

namespace org::apache::lucene::analysis::util
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using FrenchAnalyzer = org::apache::lucene::analysis::fr::FrenchAnalyzer;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

void TestElision::testElision() 
{
  wstring test = L"Plop, juste pour voir l'embrouille avec O'brian. M'enfin.";
  shared_ptr<Tokenizer> tokenizer =
      make_shared<StandardTokenizer>(newAttributeFactory());
  tokenizer->setReader(make_shared<StringReader>(test));
  shared_ptr<CharArraySet> articles =
      make_shared<CharArraySet>(asSet({L"l", L"M"}), false);
  shared_ptr<TokenFilter> filter =
      make_shared<ElisionFilter>(tokenizer, articles);
  deque<wstring> tas = this->filter(filter);
  assertEquals(L"embrouille", tas[4]);
  assertEquals(L"O'brian", tas[6]);
  assertEquals(L"enfin", tas[7]);
}

deque<wstring>
TestElision::filter(shared_ptr<TokenFilter> filter) 
{
  deque<wstring> tas = deque<wstring>();
  shared_ptr<CharTermAttribute> termAtt =
      filter->getAttribute(CharTermAttribute::typeid);
  filter->reset();
  while (filter->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    tas.push_back(termAtt->toString());
  }
  filter->end();
  delete filter;
  return tas;
}

void TestElision::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestElision::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestElision> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestElision::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer,
      make_shared<ElisionFilter>(tokenizer, FrenchAnalyzer::DEFAULT_ARTICLES));
}
} // namespace org::apache::lucene::analysis::util