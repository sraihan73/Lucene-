using namespace std;

#include "TestPatternReplaceCharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/pattern/PatternReplaceCharFilter.h"

namespace org::apache::lucene::analysis::pattern
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharFilter = org::apache::lucene::analysis::CharFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Ignore;

void TestPatternReplaceCharFilter::testFailingDot() 
{
  checkOutput(L"A. .B.", L"\\.[\\s]*", L".", L"A..B.", L"A..B.");
}

void TestPatternReplaceCharFilter::testLongerReplacement() 
{
  checkOutput(L"XXabcZZabcYY", L"abc", L"abcde", L"XXabcdeZZabcdeYY",
              L"XXabcccZZabcccYY");
  checkOutput(L"XXabcabcYY", L"abc", L"abcde", L"XXabcdeabcdeYY",
              L"XXabcccabcccYY");
  checkOutput(L"abcabcYY", L"abc", L"abcde", L"abcdeabcdeYY", L"abcccabcccYY");
  checkOutput(L"YY", L"^", L"abcde", L"abcdeYY", L"YYYYYYY");
  checkOutput(L"YY", L"$", L"abcde", L"YYabcde", L"YYYYYYY");
  checkOutput(L"XYZ", L".", L"abc", L"abcabcabc", L"XXXYYYZZZ");
  checkOutput(L"XYZ", L".", L"$0abc", L"XabcYabcZabc", L"XXXXYYYYZZZZ");
}

void TestPatternReplaceCharFilter::testShorterReplacement() 
{
  checkOutput(L"XXabcZZabcYY", L"abc", L"xy", L"XXxyZZxyYY", L"XXabZZabYY");
  checkOutput(L"XXabcabcYY", L"abc", L"xy", L"XXxyxyYY", L"XXababYY");
  checkOutput(L"abcabcYY", L"abc", L"xy", L"xyxyYY", L"ababYY");
  checkOutput(L"abcabcYY", L"abc", L"", L"YY", L"YY");
  checkOutput(L"YYabcabc", L"abc", L"", L"YY", L"YY");
}

void TestPatternReplaceCharFilter::checkOutput(
    const wstring &input, const wstring &pattern, const wstring &replacement,
    const wstring &expectedOutput,
    const wstring &expectedIndexMatchedOutput) 
{
  shared_ptr<CharFilter> cs = make_shared<PatternReplaceCharFilter>(
      this->pattern(pattern), replacement, make_shared<StringReader>(input));

  shared_ptr<StringBuilder> output = make_shared<StringBuilder>();
  for (int chr = cs->read(); chr > 0; chr = cs->read()) {
    output->append(static_cast<wchar_t>(chr));
  }

  shared_ptr<StringBuilder> indexMatched = make_shared<StringBuilder>();
  for (int i = 0; i < output->length(); i++) {
    indexMatched->append(
        (cs->correctOffset(i) < 0 ? L"-" : input[cs->correctOffset(i)]));
  }

  bool outputGood = expectedOutput == output->toString();
  bool indexMatchedGood =
      expectedIndexMatchedOutput == indexMatched->toString();

  if (!outputGood || !indexMatchedGood || false) {
    wcout << L"Pattern : " << pattern << endl;
    wcout << L"Replac. : " << replacement << endl;
    wcout << L"Input   : " << input << endl;
    wcout << L"Output  : " << output << endl;
    wcout << L"Expected: " << expectedOutput << endl;
    wcout << L"Output/i: " << indexMatched << endl;
    wcout << L"Expected: " << expectedIndexMatchedOutput << endl;
    wcout << endl;
  }

  assertTrue(L"Output doesn't match.", outputGood);
  assertTrue(L"Index-matched output doesn't match.", indexMatchedGood);
}

void TestPatternReplaceCharFilter::testNothingChange() 
{
  const wstring BLOCK = L"this is test.";
  shared_ptr<CharFilter> cs = make_shared<PatternReplaceCharFilter>(
      pattern(L"(aa)\\s+(bb)\\s+(cc)"), L"$1$2$3",
      make_shared<StringReader>(BLOCK));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"this", L"is", L"test."},
                            std::deque<int>{0, 5, 8},
                            std::deque<int>{4, 7, 13}, BLOCK.length());
}

void TestPatternReplaceCharFilter::testReplaceByEmpty() 
{
  const wstring BLOCK = L"aa bb cc";
  shared_ptr<CharFilter> cs = make_shared<PatternReplaceCharFilter>(
      pattern(L"(aa)\\s+(bb)\\s+(cc)"), L"", make_shared<StringReader>(BLOCK));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>());
}

void TestPatternReplaceCharFilter::test1block1matchSameLength() throw(
    IOException)
{
  const wstring BLOCK = L"aa bb cc";
  shared_ptr<CharFilter> cs = make_shared<PatternReplaceCharFilter>(
      pattern(L"(aa)\\s+(bb)\\s+(cc)"), L"$1#$2#$3",
      make_shared<StringReader>(BLOCK));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"aa#bb#cc"},
                            std::deque<int>{0}, std::deque<int>{8},
                            BLOCK.length());
}

void TestPatternReplaceCharFilter::test1block1matchLonger() 
{
  const wstring BLOCK = L"aa bb cc dd";
  shared_ptr<CharFilter> cs = make_shared<PatternReplaceCharFilter>(
      pattern(L"(aa)\\s+(bb)\\s+(cc)"), L"$1##$2###$3",
      make_shared<StringReader>(BLOCK));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"aa##bb###cc", L"dd"},
                            std::deque<int>{0, 9}, std::deque<int>{8, 11},
                            BLOCK.length());
}

void TestPatternReplaceCharFilter::test1block2matchLonger() 
{
  const wstring BLOCK = L" a  a";
  shared_ptr<CharFilter> cs = make_shared<PatternReplaceCharFilter>(
      pattern(L"a"), L"aa", make_shared<StringReader>(BLOCK));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"aa", L"aa"},
                            std::deque<int>{1, 4}, std::deque<int>{2, 5},
                            BLOCK.length());
}

void TestPatternReplaceCharFilter::test1block1matchShorter() 
{
  const wstring BLOCK = L"aa  bb   cc dd";
  shared_ptr<CharFilter> cs = make_shared<PatternReplaceCharFilter>(
      pattern(L"(aa)\\s+(bb)\\s+(cc)"), L"$1#$2",
      make_shared<StringReader>(BLOCK));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts, std::deque<wstring>{L"aa#bb", L"dd"},
                            std::deque<int>{0, 12}, std::deque<int>{11, 14},
                            BLOCK.length());
}

void TestPatternReplaceCharFilter::test1blockMultiMatches() 
{
  const wstring BLOCK = L"  aa bb cc --- aa bb aa   bb   cc";
  shared_ptr<CharFilter> cs = make_shared<PatternReplaceCharFilter>(
      pattern(L"(aa)\\s+(bb)\\s+(cc)"), L"$1  $2  $3",
      make_shared<StringReader>(BLOCK));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(
      ts,
      std::deque<wstring>{L"aa", L"bb", L"cc", L"---", L"aa", L"bb", L"aa",
                           L"bb", L"cc"},
      std::deque<int>{2, 6, 9, 11, 15, 18, 21, 25, 29},
      std::deque<int>{4, 8, 10, 14, 17, 20, 23, 27, 33}, BLOCK.length());
}

void TestPatternReplaceCharFilter::test2blocksMultiMatches() 
{
  const wstring BLOCK = L"  aa bb cc --- aa bb aa. bb aa   bb cc";

  shared_ptr<CharFilter> cs = make_shared<PatternReplaceCharFilter>(
      pattern(L"(aa)\\s+(bb)"), L"$1##$2", make_shared<StringReader>(BLOCK));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(
      ts,
      std::deque<wstring>{L"aa##bb", L"cc", L"---", L"aa##bb", L"aa.", L"bb",
                           L"aa##bb", L"cc"},
      std::deque<int>{2, 8, 11, 15, 21, 25, 28, 36},
      std::deque<int>{7, 10, 14, 20, 24, 27, 35, 38}, BLOCK.length());
}

void TestPatternReplaceCharFilter::testChain() 
{
  const wstring BLOCK = L" a bb - ccc . --- bb a . ccc ccc bb";
  shared_ptr<CharFilter> cs = make_shared<PatternReplaceCharFilter>(
      pattern(L"a"), L"aa", make_shared<StringReader>(BLOCK));
  cs = make_shared<PatternReplaceCharFilter>(pattern(L"bb"), L"b", cs);
  cs = make_shared<PatternReplaceCharFilter>(pattern(L"ccc"), L"c", cs);
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(
      ts,
      std::deque<wstring>{L"aa", L"b", L"-", L"c", L".", L"---", L"b", L"aa",
                           L".", L"c", L"c", L"b"},
      std::deque<int>{1, 3, 6, 8, 12, 14, 18, 21, 23, 25, 29, 33},
      std::deque<int>{2, 5, 7, 11, 13, 17, 20, 22, 24, 28, 32, 35},
      BLOCK.length());
}

shared_ptr<Pattern> TestPatternReplaceCharFilter::pattern(const wstring &p)
{
  return Pattern::compile(p);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore public void testNastyPattern() throws Exception
void TestPatternReplaceCharFilter::testNastyPattern() 
{
  shared_ptr<Pattern> p = Pattern::compile(L"(c.+)*xy");
  wstring input =
      L"[;<!--aecbbaa--><    febcfdc fbb = \"fbeeebff\" fc = dd   "
      L">\\';<eefceceaa e= babae\" eacbaff =\"fcfaccacd\" = bcced>>><  bccaafe "
      L"edb = ecfccdff\"   <?</script><    edbd ebbcd=\"faacfcc\" aeca= bedbc "
      L"ceeaac =adeafde aadccdaf = \"afcc ffda=aafbe &#x16921ed5\"1843785582']";
  for (int i = 0; i < input.length(); i++) {
    shared_ptr<Matcher> matcher = p->matcher(input.substr(0, i));
    int64_t t = System::currentTimeMillis();
    if (matcher->find()) {
      wcout << matcher->group() << endl;
    }
    wcout << i << L" > " << (System::currentTimeMillis() - t) / 1000.0 << endl;
  }
}

void TestPatternReplaceCharFilter::testRandomStrings() 
{
  int numPatterns = 10 + random()->nextInt(20);
  shared_ptr<Random> random =
      make_shared<Random>(TestPatternReplaceCharFilter::random()->nextLong());
  for (int i = 0; i < numPatterns; i++) {
    shared_ptr<Pattern> *const p =
        TestUtil::randomPattern(TestPatternReplaceCharFilter::random());

    const wstring replacement = TestUtil::randomSimpleString(random);
    shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass>(
        shared_from_this(), p, replacement);

    /* max input length. don't make it longer -- exponential processing
     * time for certain patterns. */
    constexpr int maxInputLength = 30;
    /* ASCII only input?: */
    constexpr bool asciiOnly = true;
    checkRandomData(random, a, 250 * RANDOM_MULTIPLIER, maxInputLength,
                    asciiOnly);
    delete a;
  }
}

TestPatternReplaceCharFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestPatternReplaceCharFilter> outerInstance,
        shared_ptr<Pattern> p, const wstring &replacement)
{
  this->outerInstance = outerInstance;
  this->p = p;
  this->replacement = replacement;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPatternReplaceCharFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

shared_ptr<Reader>
TestPatternReplaceCharFilter::AnalyzerAnonymousInnerClass::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<PatternReplaceCharFilter>(p, replacement, reader);
}
} // namespace org::apache::lucene::analysis::pattern