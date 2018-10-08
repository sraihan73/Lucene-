using namespace std;

#include "TestWordnetSynonymParser.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymMap.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/WordnetSynonymParser.h"

namespace org::apache::lucene::analysis::synonym
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

void TestWordnetSynonymParser::testSynonyms() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<WordnetSynonymParser> parser =
      make_shared<WordnetSynonymParser>(true, true, analyzer);
  parser->parse(make_shared<StringReader>(synonymsFile));
  shared_ptr<SynonymMap> *const map_obj = parser->build();
  delete analyzer;

  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), map_obj);

  /* all expansions */
  assertAnalyzesTo(analyzer, L"Lost in the woods",
                   std::deque<wstring>{L"Lost", L"in", L"the", L"woods",
                                        L"wood", L"forest"},
                   std::deque<int>{0, 5, 8, 12, 12, 12},
                   std::deque<int>{4, 7, 11, 17, 17, 17},
                   std::deque<int>{1, 1, 1, 1, 0, 0});

  /* single quote */
  assertAnalyzesTo(analyzer, L"king", std::deque<wstring>{L"king", L"baron"});

  /* multi words */
  assertAnalyzesTo(
      analyzer, L"king's evil",
      std::deque<wstring>{L"king's", L"king's", L"evil", L"meany"});
  delete analyzer;
}

TestWordnetSynonymParser::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestWordnetSynonymParser> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestWordnetSynonymParser::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, false));
}
} // namespace org::apache::lucene::analysis::synonym