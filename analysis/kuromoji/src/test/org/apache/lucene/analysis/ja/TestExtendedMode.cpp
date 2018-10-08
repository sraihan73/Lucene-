using namespace std;

#include "TestExtendedMode.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/UnicodeUtil.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseTokenizer.h"

namespace org::apache::lucene::analysis::ja
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using Mode = org::apache::lucene::analysis::ja::JapaneseTokenizer::Mode;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TestUtil = org::apache::lucene::util::TestUtil;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;

void TestExtendedMode::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestExtendedMode::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestExtendedMode> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestExtendedMode::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), nullptr, true,
      Mode::EXTENDED);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void TestExtendedMode::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestExtendedMode::testSurrogates() 
{
  assertAnalyzesTo(
      analyzer, L"𩬅艱鍟䇹愯瀛",
      std::deque<wstring>{L"𩬅", L"艱", L"鍟", L"䇹", L"愯", L"瀛"});
}

void TestExtendedMode::testSurrogates2() 
{
  int numIterations = atLeast(1000);
  for (int i = 0; i < numIterations; i++) {
    wstring s = TestUtil::randomUnicodeString(random(), 100);
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts
    // = analyzer.tokenStream("foo", s))
    {
      org::apache::lucene::analysis::TokenStream ts =
          analyzer->tokenStream(L"foo", s);
      shared_ptr<CharTermAttribute> termAtt =
          ts->addAttribute(CharTermAttribute::typeid);
      ts->reset();
      while (ts->incrementToken()) {
        assertTrue(UnicodeUtil::validUTF16String(termAtt));
      }
      ts->end();
    }
  }
}

void TestExtendedMode::testRandomStrings() 
{
  shared_ptr<Random> random = TestExtendedMode::random();
  checkRandomData(random, analyzer, 500 * RANDOM_MULTIPLIER);
}

void TestExtendedMode::testRandomHugeStrings() 
{
  shared_ptr<Random> random = TestExtendedMode::random();
  checkRandomData(random, analyzer, 30 * RANDOM_MULTIPLIER, 8192);
}
} // namespace org::apache::lucene::analysis::ja