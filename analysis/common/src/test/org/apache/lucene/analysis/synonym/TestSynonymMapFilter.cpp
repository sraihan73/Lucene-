using namespace std;

#include "TestSynonymMapFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockGraphTokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SolrSynonymParser.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymMap.h"

namespace org::apache::lucene::analysis::synonym
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using MockGraphTokenFilter =
    org::apache::lucene::analysis::MockGraphTokenFilter;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using namespace org::apache::lucene::analysis::tokenattributes;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestSynonymMapFilter::add(const wstring &input, const wstring &output,
                               bool keepOrig)
{
  if (VERBOSE) {
    wcout << L"  add input=" << input << L" output=" << output << L" keepOrig="
          << keepOrig << endl;
  }
  shared_ptr<CharsRefBuilder> inputCharsRef = make_shared<CharsRefBuilder>();
  SynonymMap::Builder::join(input.split(L" +"), inputCharsRef);

  shared_ptr<CharsRefBuilder> outputCharsRef = make_shared<CharsRefBuilder>();
  SynonymMap::Builder::join(output.split(L" +"), outputCharsRef);

  b->add(inputCharsRef->get(), outputCharsRef->get(), keepOrig);
}

void TestSynonymMapFilter::assertEquals(shared_ptr<CharTermAttribute> term,
                                        const wstring &expected)
{
  assertEquals(expected.length(), term->length());
  const std::deque<wchar_t> buffer = term->buffer();
  for (int chIDX = 0; chIDX < expected.length(); chIDX++) {
    assertEquals(expected[chIDX], buffer[chIDX]);
  }
}

void TestSynonymMapFilter::verify(const wstring &input,
                                  const wstring &output) 
{
  if (VERBOSE) {
    wcout << L"TEST: verify input=" << input << L" expectedOutput=" << output
          << endl;
  }

  tokensIn->setReader(make_shared<StringReader>(input));
  tokensOut->reset();
  const std::deque<wstring> expected = output.split(L" ");
  int expectedUpto = 0;
  while (tokensOut->incrementToken()) {

    if (VERBOSE) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wcout << L"  incr token=" << termAtt->toString() << L" posIncr="
            << posIncrAtt->getPositionIncrement() << L" startOff="
            << offsetAtt->startOffset() << L" endOff=" << offsetAtt->endOffset()
            << endl;
    }

    assertTrue(expectedUpto < expected.size());
    constexpr int startOffset = offsetAtt->startOffset();
    constexpr int endOffset = offsetAtt->endOffset();

    const std::deque<wstring> expectedAtPos =
        expected[expectedUpto++].split(L"/");
    for (int atPos = 0; atPos < expectedAtPos.size(); atPos++) {
      if (atPos > 0) {
        assertTrue(tokensOut->incrementToken());
        if (VERBOSE) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          wcout << L"  incr token=" << termAtt->toString() << L" posIncr="
                << posIncrAtt->getPositionIncrement() << L" startOff="
                << offsetAtt->startOffset() << L" endOff="
                << offsetAtt->endOffset() << endl;
        }
      }
      constexpr int colonIndex = (int)expectedAtPos[atPos].find(L':');
      constexpr int underbarIndex = (int)expectedAtPos[atPos].find(L'_');
      const wstring expectedToken;
      constexpr int expectedEndOffset;
      constexpr int expectedPosLen;
      if (colonIndex != -1) {
        expectedToken = expectedAtPos[atPos].substr(0, colonIndex);
        if (underbarIndex != -1) {
          expectedEndOffset = stoi(expectedAtPos[atPos].substr(
              1 + colonIndex, underbarIndex - (1 + colonIndex)));
          expectedPosLen = stoi(expectedAtPos[atPos].substr(1 + underbarIndex));
        } else {
          expectedEndOffset = stoi(expectedAtPos[atPos].substr(1 + colonIndex));
          expectedPosLen = 1;
        }
      } else {
        expectedToken = expectedAtPos[atPos];
        expectedEndOffset = endOffset;
        expectedPosLen = 1;
      }
      // C++ TODO: There is no native C++ equivalent to 'toString':
      assertEquals(expectedToken, termAtt->toString());
      assertEquals(atPos == 0 ? 1 : 0, posIncrAtt->getPositionIncrement());
      // start/end offset of all tokens at same pos should
      // be the same:
      assertEquals(startOffset, offsetAtt->startOffset());
      assertEquals(expectedEndOffset, offsetAtt->endOffset());
      assertEquals(expectedPosLen, posLenAtt->getPositionLength());
    }
  }
  tokensOut->end();
  delete tokensOut;
  if (VERBOSE) {
    wcout << L"  incr: END" << endl;
  }
  assertEquals(expectedUpto, expected.size());
}

void TestSynonymMapFilter::testDontKeepOrig() 
{
  b = make_shared<SynonymMap::Builder>(true);
  add(L"a b", L"foo", false);

  shared_ptr<SynonymMap> *const map_obj = b->build();

  shared_ptr<Analyzer> *const analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), map_obj);

  assertAnalyzesTo(analyzer, L"a b c", std::deque<wstring>{L"foo", L"c"},
                   std::deque<int>{0, 4}, std::deque<int>{3, 5}, nullptr,
                   std::deque<int>{1, 1}, std::deque<int>{1, 1}, true);
  checkAnalysisConsistency(random(), analyzer, false, L"a b c");
  delete analyzer;
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestSynonymMapFilter> outerInstance,
    shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, false));
}

void TestSynonymMapFilter::testDoKeepOrig() 
{
  b = make_shared<SynonymMap::Builder>(true);
  add(L"a b", L"foo", true);

  shared_ptr<SynonymMap> *const map_obj = b->build();

  shared_ptr<Analyzer> *const analyzer =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this(), map_obj);

  assertAnalyzesTo(
      analyzer, L"a b c", std::deque<wstring>{L"a", L"foo", L"b", L"c"},
      std::deque<int>{0, 0, 2, 4}, std::deque<int>{1, 3, 3, 5}, nullptr,
      std::deque<int>{1, 0, 1, 1}, std::deque<int>{1, 2, 1, 1}, true);
  checkAnalysisConsistency(random(), analyzer, false, L"a b c");
  delete analyzer;
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestSynonymMapFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, false));
}

void TestSynonymMapFilter::testBasic() 
{
  b = make_shared<SynonymMap::Builder>(true);
  add(L"a", L"foo", true);
  add(L"a b", L"bar fee", true);
  add(L"b c", L"dog collar", true);
  add(L"c d", L"dog harness holder extras", true);
  add(L"m c e", L"dog barks loudly", false);
  add(L"i j k", L"feep", true);

  add(L"e f", L"foo bar", false);
  add(L"e f", L"baz bee", false);

  add(L"z", L"boo", false);
  add(L"y", L"bee", true);

  tokensIn = make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  tokensIn->setReader(make_shared<StringReader>(L"a"));
  tokensIn->reset();
  assertTrue(tokensIn->incrementToken());
  assertFalse(tokensIn->incrementToken());
  tokensIn->end();
  delete tokensIn;

  tokensOut = make_shared<SynonymFilter>(tokensIn, b->build(), true);
  termAtt = tokensOut->addAttribute(CharTermAttribute::typeid);
  posIncrAtt = tokensOut->addAttribute(PositionIncrementAttribute::typeid);
  posLenAtt = tokensOut->addAttribute(PositionLengthAttribute::typeid);
  offsetAtt = tokensOut->addAttribute(OffsetAttribute::typeid);

  verify(L"a b c", L"a/bar b/fee c");

  // syn output extends beyond input tokens
  verify(L"x a b c d", L"x a/bar b/fee c/dog d/harness holder extras");

  verify(L"a b a", L"a/bar b/fee a/foo");

  // outputs that add to one another:
  verify(L"c d c d",
         L"c/dog d/harness c/holder/dog d/extras/harness holder extras");

  // two outputs for same input
  verify(L"e f", L"foo/baz bar/bee");

  // verify multi-word / single-output offsets:
  verify(L"g i j k g", L"g i/feep:7_3 j k g");

  // mixed keepOrig true/false:
  verify(L"a m c e x", L"a/foo dog barks loudly x");
  verify(L"c d m c e x", L"c/dog d/harness holder/dog extras/barks loudly x");
  assertTrue(tokensOut->getCaptureCount() > 0);

  // no captureStates when no syns matched
  verify(L"p q r s t", L"p q r s t");
  assertEquals(0, tokensOut->getCaptureCount());

  // no captureStates when only single-input syns, w/ no
  // lookahead needed, matched
  verify(L"p q z y t", L"p q boo y/bee t");
  assertEquals(0, tokensOut->getCaptureCount());
}

wstring TestSynonymMapFilter::getRandomString(wchar_t start, int alphabetSize,
                                              int length)
{
  assert(alphabetSize <= 26);
  std::deque<wchar_t> s(2 * length);
  for (int charIDX = 0; charIDX < length; charIDX++) {
    s[2 * charIDX] =
        static_cast<wchar_t>(start + random()->nextInt(alphabetSize));
    s[2 * charIDX + 1] = L' ';
  }
  return wstring(s);
}

wstring
TestSynonymMapFilter::slowSynMatcher(const wstring &doc,
                                     deque<std::shared_ptr<OneSyn>> &syns,
                                     int maxOutputLength)
{
  assertTrue(doc.length() % 2 == 0);
  constexpr int numInputs = doc.length() / 2;
  std::deque<bool> keepOrigs(numInputs);
  std::deque<bool> hasMatch(numInputs);
  Arrays::fill(keepOrigs, false);
  std::deque<wstring> outputs(numInputs + maxOutputLength);
  std::deque<std::shared_ptr<OneSyn>> matches(numInputs);
  for (auto syn : syns) {
    int idx = -1;
    while (true) {
      idx = (int)doc.find(syn->in_, 1 + idx);
      if (idx == -1) {
        break;
      }
      assertTrue(idx % 2 == 0);
      constexpr int matchIDX = idx / 2;
      assertTrue(syn->in_.length() % 2 == 1);
      if (matches[matchIDX] == nullptr) {
        matches[matchIDX] = syn;
      } else if (syn->in_.length() > matches[matchIDX]->in_->length()) {
        // Greedy conflict resolution: longer match wins:
        matches[matchIDX] = syn;
      } else {
        assertTrue(syn->in_.length() < matches[matchIDX]->in_->length());
      }
    }
  }

  // Greedy conflict resolution: if syn matches a range of inputs,
  // it prevents other syns from matching that range
  for (int inputIDX = 0; inputIDX < numInputs; inputIDX++) {
    shared_ptr<OneSyn> *const match = matches[inputIDX];
    if (match != nullptr) {
      constexpr int synInLength = (1 + match->in_.length()) / 2;
      for (int nextInputIDX = inputIDX + 1;
           nextInputIDX < numInputs && nextInputIDX < (inputIDX + synInLength);
           nextInputIDX++) {
        matches[nextInputIDX].reset();
      }
    }
  }

  // Fill overlapping outputs:
  for (int inputIDX = 0; inputIDX < numInputs; inputIDX++) {
    shared_ptr<OneSyn> *const syn = matches[inputIDX];
    if (syn == nullptr) {
      continue;
    }
    for (int idx = 0; idx < (1 + syn->in_.length()) / 2; idx++) {
      hasMatch[inputIDX + idx] = true;
      keepOrigs[inputIDX + idx] |= syn->keepOrig;
    }
    for (auto synOut : syn->out) {
      const std::deque<wstring> synOutputs = synOut.split(L" ");
      assertEquals(synOutputs.size(), (1 + synOut.length()) / 2);
      constexpr int matchEnd = inputIDX + synOutputs.size();
      int synUpto = 0;
      for (int matchIDX = inputIDX; matchIDX < matchEnd; matchIDX++) {
        if (outputs[matchIDX] == L"") {
          outputs[matchIDX] = synOutputs[synUpto++];
        } else {
          outputs[matchIDX] = outputs[matchIDX] + L"/" + synOutputs[synUpto++];
        }
        constexpr int endOffset;
        if (matchIDX < numInputs) {
          constexpr int posLen;
          if (synOutputs.size() == 1) {
            // Add full endOffset
            endOffset = (inputIDX * 2) + syn->in_.length();
            posLen = syn->keepOrig ? (1 + syn->in_.length()) / 2 : 1;
          } else {
            // Add endOffset matching input token's
            endOffset = (matchIDX * 2) + 1;
            posLen = 1;
          }
          outputs[matchIDX] = outputs[matchIDX] + L":" + to_wstring(endOffset) +
                              L"_" + to_wstring(posLen);
        }
      }
    }
  }

  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  std::deque<wstring> inputTokens = doc.split(L" ");
  constexpr int limit = inputTokens.size() + maxOutputLength;
  for (int inputIDX = 0; inputIDX < limit; inputIDX++) {
    bool posHasOutput = false;
    if (inputIDX >= numInputs && outputs[inputIDX] == L"") {
      break;
    }
    if (inputIDX < numInputs && (!hasMatch[inputIDX] || keepOrigs[inputIDX])) {
      assertTrue(inputTokens[inputIDX].length() != 0);
      sb->append(inputTokens[inputIDX]);
      posHasOutput = true;
    }

    if (outputs[inputIDX] != L"") {
      if (posHasOutput) {
        sb->append(L'/');
      }
      sb->append(outputs[inputIDX]);
    } else if (!posHasOutput) {
      continue;
    }
    if (inputIDX < limit - 1) {
      sb->append(L' ');
    }
  }

  return sb->toString();
}

void TestSynonymMapFilter::testRandom() 
{

  constexpr int alphabetSize = TestUtil::nextInt(random(), 2, 7);

  constexpr int docLen = atLeast(3000);
  // final int docLen = 50;

  const wstring document = getRandomString(L'a', alphabetSize, docLen);

  if (VERBOSE) {
    wcout << L"TEST: doc=" << document << endl;
  }

  constexpr int numSyn = atLeast(5);
  // final int numSyn = 2;

  const unordered_map<wstring, std::shared_ptr<OneSyn>> synMap =
      unordered_map<wstring, std::shared_ptr<OneSyn>>();
  const deque<std::shared_ptr<OneSyn>> syns =
      deque<std::shared_ptr<OneSyn>>();
  constexpr bool dedup = random()->nextBoolean();
  if (VERBOSE) {
    wcout << L"  dedup=" << dedup << endl;
  }
  b = make_shared<SynonymMap::Builder>(dedup);
  for (int synIDX = 0; synIDX < numSyn; synIDX++) {
    const wstring synIn = StringHelper::trim(
        getRandomString(L'a', alphabetSize, TestUtil::nextInt(random(), 1, 5)));
    shared_ptr<OneSyn> s = synMap[synIn];
    if (s == nullptr) {
      s = make_shared<OneSyn>();
      s->in_ = synIn;
      syns.push_back(s);
      s->out = deque<>();
      synMap.emplace(synIn, s);
      s->keepOrig = random()->nextBoolean();
    }
    const wstring synOut = StringHelper::trim(
        getRandomString(L'0', 10, TestUtil::nextInt(random(), 1, 5)));
    s->out.push_back(synOut);
    add(synIn, synOut, s->keepOrig);
    if (VERBOSE) {
      wcout << L"  syns[" << synIDX << L"] = " << s->in_ << L" -> " << s->out
            << L" keepOrig=" << s->keepOrig << endl;
    }
  }

  tokensIn = make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  tokensIn->setReader(make_shared<StringReader>(L"a"));
  tokensIn->reset();
  assertTrue(tokensIn->incrementToken());
  assertFalse(tokensIn->incrementToken());
  tokensIn->end();
  delete tokensIn;

  tokensOut = make_shared<SynonymFilter>(tokensIn, b->build(), true);
  termAtt = tokensOut->addAttribute(CharTermAttribute::typeid);
  posIncrAtt = tokensOut->addAttribute(PositionIncrementAttribute::typeid);
  posLenAtt = tokensOut->addAttribute(PositionLengthAttribute::typeid);
  offsetAtt = tokensOut->addAttribute(OffsetAttribute::typeid);

  if (dedup) {
    pruneDups(syns);
  }

  const wstring expected = slowSynMatcher(document, syns, 5);

  if (VERBOSE) {
    wcout << L"TEST: expected=" << expected << endl;
  }

  verify(document, expected);
}

void TestSynonymMapFilter::pruneDups(deque<std::shared_ptr<OneSyn>> &syns)
{
  shared_ptr<Set<wstring>> seen = unordered_set<wstring>();
  for (auto syn : syns) {
    int idx = 0;
    while (idx < syn->out.size()) {
      wstring out = syn->out[idx];
      if (!seen->contains(out)) {
        seen->add(out);
        idx++;
      } else {
        syn->out.erase(syn->out.begin() + idx);
      }
    }
    seen->clear();
  }
}

wstring TestSynonymMapFilter::randomNonEmptyString()
{
  while (true) {
    const wstring s =
        StringHelper::trim(TestUtil::randomUnicodeString(random()));
    if (s.length() != 0 && s.find(L'\u0000') == wstring::npos) {
      return s;
    }
  }
}

void TestSynonymMapFilter::testRandom2() 
{
  constexpr int numIters = atLeast(3);
  for (int i = 0; i < numIters; i++) {
    b = make_shared<SynonymMap::Builder>(random()->nextBoolean());
    constexpr int numEntries = atLeast(10);
    for (int j = 0; j < numEntries; j++) {
      add(randomNonEmptyString(), randomNonEmptyString(),
          random()->nextBoolean());
    }
    shared_ptr<SynonymMap> *const map_obj = b->build();
    constexpr bool ignoreCase = random()->nextBoolean();

    shared_ptr<Analyzer> *const analyzer =
        make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), map_obj,
                                                 ignoreCase);

    checkRandomData(random(), analyzer, 100);
    delete analyzer;
  }
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestSynonymMapFilter> outerInstance,
    shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj,
    bool ignoreCase)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
  this->ignoreCase = ignoreCase;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, ignoreCase));
}

void TestSynonymMapFilter::testRandom2GraphAfter() 
{
  constexpr int numIters = atLeast(3);
  shared_ptr<Random> random = TestSynonymMapFilter::random();
  for (int i = 0; i < numIters; i++) {
    b = make_shared<SynonymMap::Builder>(random->nextBoolean());
    constexpr int numEntries = atLeast(10);
    for (int j = 0; j < numEntries; j++) {
      add(randomNonEmptyString(), randomNonEmptyString(),
          random->nextBoolean());
    }
    shared_ptr<SynonymMap> *const map_obj = b->build();
    constexpr bool ignoreCase = random->nextBoolean();

    shared_ptr<Analyzer> *const analyzer =
        make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this(), map_obj,
                                                  ignoreCase);

    checkRandomData(random, analyzer, 100);
    delete analyzer;
  }
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestSynonymMapFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj,
        bool ignoreCase)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
  this->ignoreCase = ignoreCase;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  shared_ptr<TokenStream> syns =
      make_shared<SynonymFilter>(tokenizer, map_obj, ignoreCase);
  shared_ptr<TokenStream> graph =
      make_shared<MockGraphTokenFilter>(random(), syns);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, graph);
}

void TestSynonymMapFilter::testEmptyTerm() 
{
  shared_ptr<Random> random = TestSynonymMapFilter::random();
  constexpr int numIters = atLeast(10);
  for (int i = 0; i < numIters; i++) {
    b = make_shared<SynonymMap::Builder>(random->nextBoolean());
    constexpr int numEntries = atLeast(10);
    for (int j = 0; j < numEntries; j++) {
      add(randomNonEmptyString(), randomNonEmptyString(),
          random->nextBoolean());
    }
    shared_ptr<SynonymMap> *const map_obj = b->build();
    constexpr bool ignoreCase = random->nextBoolean();

    shared_ptr<Analyzer> *const analyzer =
        make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this(), map_obj,
                                                  ignoreCase);

    checkAnalysisConsistency(random, analyzer, random->nextBoolean(), L"");
    delete analyzer;
  }
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestSynonymMapFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj,
        bool ignoreCase)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
  this->ignoreCase = ignoreCase;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, ignoreCase));
}

void TestSynonymMapFilter::testRandomHuge() 
{
  shared_ptr<Random> random = TestSynonymMapFilter::random();
  constexpr int numIters = atLeast(3);
  for (int i = 0; i < numIters; i++) {
    b = make_shared<SynonymMap::Builder>(random->nextBoolean());
    constexpr int numEntries = atLeast(10);
    if (VERBOSE) {
      wcout << L"TEST: iter=" << i << L" numEntries=" << numEntries << endl;
    }
    for (int j = 0; j < numEntries; j++) {
      add(randomNonEmptyString(), randomNonEmptyString(),
          random->nextBoolean());
    }
    shared_ptr<SynonymMap> *const map_obj = b->build();
    constexpr bool ignoreCase = random->nextBoolean();

    shared_ptr<Analyzer> *const analyzer =
        make_shared<AnalyzerAnonymousInnerClass4>(shared_from_this(), map_obj,
                                                  ignoreCase);

    checkRandomData(random, analyzer, 100, 1024);
    delete analyzer;
  }
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass4::
    AnalyzerAnonymousInnerClass4(
        shared_ptr<TestSynonymMapFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj,
        bool ignoreCase)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
  this->ignoreCase = ignoreCase;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass4::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, ignoreCase));
}

void TestSynonymMapFilter::testVanishingTerms() 
{
  wstring testFile =
      wstring(L"aaa => aaaa1 aaaa2 aaaa3\n") + L"bbb => bbbb1 bbbb2\n";
  shared_ptr<Analyzer> synAnalyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<SolrSynonymParser> parser =
      make_shared<SolrSynonymParser>(true, true, synAnalyzer);
  parser->parse(make_shared<StringReader>(testFile));
  shared_ptr<SynonymMap> *const map_obj = parser->build();
  delete synAnalyzer;

  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass5>(shared_from_this(), map_obj);

  // where did my pot go?!
  assertAnalyzesTo(analyzer, L"xyzzy bbb pot of gold",
                   std::deque<wstring>{L"xyzzy", L"bbbb1", L"pot", L"bbbb2",
                                        L"of", L"gold"});

  // this one nukes 'pot' and 'of'
  // xyzzy aaa pot of gold -> xyzzy aaaa1 aaaa2 aaaa3 gold
  assertAnalyzesTo(analyzer, L"xyzzy aaa pot of gold",
                   std::deque<wstring>{L"xyzzy", L"aaaa1", L"pot", L"aaaa2",
                                        L"of", L"aaaa3", L"gold"});
  delete analyzer;
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass5::
    AnalyzerAnonymousInnerClass5(
        shared_ptr<TestSynonymMapFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass5::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, true));
}

void TestSynonymMapFilter::testBasic2() 
{
  b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = false;
  add(L"aaa", L"aaaa1 aaaa2 aaaa3", keepOrig);
  add(L"bbb", L"bbbb1 bbbb2", keepOrig);
  tokensIn = make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  tokensIn->setReader(make_shared<StringReader>(L"a"));
  tokensIn->reset();
  assertTrue(tokensIn->incrementToken());
  assertFalse(tokensIn->incrementToken());
  tokensIn->end();
  delete tokensIn;

  tokensOut = make_shared<SynonymFilter>(tokensIn, b->build(), true);
  termAtt = tokensOut->addAttribute(CharTermAttribute::typeid);
  posIncrAtt = tokensOut->addAttribute(PositionIncrementAttribute::typeid);
  posLenAtt = tokensOut->addAttribute(PositionLengthAttribute::typeid);
  offsetAtt = tokensOut->addAttribute(OffsetAttribute::typeid);

  if (keepOrig) {
    verify(L"xyzzy bbb pot of gold", L"xyzzy bbb/bbbb1 pot/bbbb2 of gold");
    verify(L"xyzzy aaa pot of gold",
           L"xyzzy aaa/aaaa1 pot/aaaa2 of/aaaa3 gold");
  } else {
    verify(L"xyzzy bbb pot of gold", L"xyzzy bbbb1 pot/bbbb2 of gold");
    verify(L"xyzzy aaa pot of gold", L"xyzzy aaaa1 pot/aaaa2 of/aaaa3 gold");
  }
}

void TestSynonymMapFilter::testMatching() 
{
  b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = false;
  add(L"a b", L"ab", keepOrig);
  add(L"a c", L"ac", keepOrig);
  add(L"a", L"aa", keepOrig);
  add(L"b", L"bb", keepOrig);
  add(L"z x c v", L"zxcv", keepOrig);
  add(L"x c", L"xc", keepOrig);
  shared_ptr<SynonymMap> *const map_obj = b->build();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass6>(shared_from_this(), map_obj);

  checkOneTerm(a, L"$", L"$");
  checkOneTerm(a, L"a", L"aa");
  checkOneTerm(a, L"b", L"bb");

  assertAnalyzesTo(a, L"a $", std::deque<wstring>{L"aa", L"$"},
                   std::deque<int>{1, 1});

  assertAnalyzesTo(a, L"$ a", std::deque<wstring>{L"$", L"aa"},
                   std::deque<int>{1, 1});

  assertAnalyzesTo(a, L"a a", std::deque<wstring>{L"aa", L"aa"},
                   std::deque<int>{1, 1});

  assertAnalyzesTo(a, L"z x c v", std::deque<wstring>{L"zxcv"},
                   std::deque<int>{1});

  assertAnalyzesTo(a, L"z x c $", std::deque<wstring>{L"z", L"xc", L"$"},
                   std::deque<int>{1, 1, 1});
  delete a;
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass6::
    AnalyzerAnonymousInnerClass6(
        shared_ptr<TestSynonymMapFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass6::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, true));
}

void TestSynonymMapFilter::testRepeatsOff() 
{
  b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = false;
  add(L"a b", L"ab", keepOrig);
  add(L"a b", L"ab", keepOrig);
  add(L"a b", L"ab", keepOrig);
  shared_ptr<SynonymMap> *const map_obj = b->build();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass7>(shared_from_this(), map_obj);

  assertAnalyzesTo(a, L"a b", std::deque<wstring>{L"ab"}, std::deque<int>{1});
  delete a;
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass7::
    AnalyzerAnonymousInnerClass7(
        shared_ptr<TestSynonymMapFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass7::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, true));
}

void TestSynonymMapFilter::testRepeatsOn() 
{
  b = make_shared<SynonymMap::Builder>(false);
  constexpr bool keepOrig = false;
  add(L"a b", L"ab", keepOrig);
  add(L"a b", L"ab", keepOrig);
  add(L"a b", L"ab", keepOrig);
  shared_ptr<SynonymMap> *const map_obj = b->build();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass8>(shared_from_this(), map_obj);

  assertAnalyzesTo(a, L"a b", std::deque<wstring>{L"ab", L"ab", L"ab"},
                   std::deque<int>{1, 0, 0});
  delete a;
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass8::
    AnalyzerAnonymousInnerClass8(
        shared_ptr<TestSynonymMapFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass8::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, true));
}

void TestSynonymMapFilter::testRecursion() 
{
  b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = false;
  add(L"zoo", L"zoo", keepOrig);
  shared_ptr<SynonymMap> *const map_obj = b->build();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass9>(shared_from_this(), map_obj);

  assertAnalyzesTo(a, L"zoo zoo $ zoo",
                   std::deque<wstring>{L"zoo", L"zoo", L"$", L"zoo"},
                   std::deque<int>{1, 1, 1, 1});
  delete a;
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass9::
    AnalyzerAnonymousInnerClass9(
        shared_ptr<TestSynonymMapFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass9::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, true));
}

void TestSynonymMapFilter::testRecursion2() 
{
  b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = false;
  add(L"zoo", L"zoo", keepOrig);
  add(L"zoo", L"zoo zoo", keepOrig);
  shared_ptr<SynonymMap> *const map_obj = b->build();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass10>(shared_from_this(), map_obj);

  // verify("zoo zoo $ zoo", "zoo/zoo zoo/zoo/zoo $/zoo zoo/zoo zoo");
  assertAnalyzesTo(a, L"zoo zoo $ zoo",
                   std::deque<wstring>{L"zoo", L"zoo", L"zoo", L"zoo", L"zoo",
                                        L"$", L"zoo", L"zoo", L"zoo", L"zoo"},
                   std::deque<int>{1, 0, 1, 0, 0, 1, 0, 1, 0, 1});
  delete a;
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass10::
    AnalyzerAnonymousInnerClass10(
        shared_ptr<TestSynonymMapFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass10::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, true));
}

void TestSynonymMapFilter::testOutputHangsOffEnd() 
{
  b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = false;
  // b hangs off the end (no input token under it):
  add(L"a", L"a b", keepOrig);
  tokensIn = make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  tokensIn->setReader(make_shared<StringReader>(L"a"));
  tokensIn->reset();
  assertTrue(tokensIn->incrementToken());
  assertFalse(tokensIn->incrementToken());
  tokensIn->end();
  delete tokensIn;

  tokensOut = make_shared<SynonymFilter>(tokensIn, b->build(), true);
  termAtt = tokensOut->addAttribute(CharTermAttribute::typeid);
  posIncrAtt = tokensOut->addAttribute(PositionIncrementAttribute::typeid);
  offsetAtt = tokensOut->addAttribute(OffsetAttribute::typeid);
  posLenAtt = tokensOut->addAttribute(PositionLengthAttribute::typeid);

  // Make sure endOffset inherits from previous input token:
  verify(L"a", L"a b:1");
}

void TestSynonymMapFilter::testIncludeOrig() 
{
  b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = true;
  add(L"a b", L"ab", keepOrig);
  add(L"a c", L"ac", keepOrig);
  add(L"a", L"aa", keepOrig);
  add(L"b", L"bb", keepOrig);
  add(L"z x c v", L"zxcv", keepOrig);
  add(L"x c", L"xc", keepOrig);
  shared_ptr<SynonymMap> *const map_obj = b->build();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass11>(shared_from_this(), map_obj);

  assertAnalyzesTo(a, L"$", std::deque<wstring>{L"$"}, std::deque<int>{1});
  assertAnalyzesTo(a, L"a", std::deque<wstring>{L"a", L"aa"},
                   std::deque<int>{1, 0});
  assertAnalyzesTo(a, L"a", std::deque<wstring>{L"a", L"aa"},
                   std::deque<int>{1, 0});
  assertAnalyzesTo(a, L"$ a", std::deque<wstring>{L"$", L"a", L"aa"},
                   std::deque<int>{1, 1, 0});
  assertAnalyzesTo(a, L"a $", std::deque<wstring>{L"a", L"aa", L"$"},
                   std::deque<int>{1, 0, 1});
  assertAnalyzesTo(a, L"$ a !", std::deque<wstring>{L"$", L"a", L"aa", L"!"},
                   std::deque<int>{1, 1, 0, 1});
  assertAnalyzesTo(a, L"a a", std::deque<wstring>{L"a", L"aa", L"a", L"aa"},
                   std::deque<int>{1, 0, 1, 0});
  assertAnalyzesTo(a, L"b", std::deque<wstring>{L"b", L"bb"},
                   std::deque<int>{1, 0});
  assertAnalyzesTo(a, L"z x c v",
                   std::deque<wstring>{L"z", L"zxcv", L"x", L"c", L"v"},
                   std::deque<int>{1, 0, 1, 1, 1});
  assertAnalyzesTo(a, L"z x c $",
                   std::deque<wstring>{L"z", L"x", L"xc", L"c", L"$"},
                   std::deque<int>{1, 1, 0, 1, 1});
  delete a;
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass11::
    AnalyzerAnonymousInnerClass11(
        shared_ptr<TestSynonymMapFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass11::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, true));
}

void TestSynonymMapFilter::testRecursion3() 
{
  b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = true;
  add(L"zoo zoo", L"zoo", keepOrig);
  shared_ptr<SynonymMap> *const map_obj = b->build();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass12>(shared_from_this(), map_obj);

  assertAnalyzesTo(a, L"zoo zoo $ zoo",
                   std::deque<wstring>{L"zoo", L"zoo", L"zoo", L"$", L"zoo"},
                   std::deque<int>{1, 0, 1, 1, 1});
  delete a;
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass12::
    AnalyzerAnonymousInnerClass12(
        shared_ptr<TestSynonymMapFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass12::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, true));
}

void TestSynonymMapFilter::testRecursion4() 
{
  b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = true;
  add(L"zoo zoo", L"zoo", keepOrig);
  add(L"zoo", L"zoo zoo", keepOrig);
  shared_ptr<SynonymMap> *const map_obj = b->build();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass13>(shared_from_this(), map_obj);

  assertAnalyzesTo(a, L"zoo zoo $ zoo",
                   std::deque<wstring>{L"zoo", L"zoo", L"zoo", L"$", L"zoo",
                                        L"zoo", L"zoo"},
                   std::deque<int>{1, 0, 1, 1, 1, 0, 1});
  delete a;
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass13::
    AnalyzerAnonymousInnerClass13(
        shared_ptr<TestSynonymMapFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass13::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, true));
}

void TestSynonymMapFilter::testMultiwordOffsets() 
{
  b = make_shared<SynonymMap::Builder>(true);
  constexpr bool keepOrig = true;
  add(L"national hockey league", L"nhl", keepOrig);
  shared_ptr<SynonymMap> *const map_obj = b->build();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass14>(shared_from_this(), map_obj);

  assertAnalyzesTo(
      a, L"national hockey league",
      std::deque<wstring>{L"national", L"nhl", L"hockey", L"league"},
      std::deque<int>{0, 0, 9, 16}, std::deque<int>{8, 22, 15, 22},
      std::deque<int>{1, 0, 1, 1});
  delete a;
}

TestSynonymMapFilter::AnalyzerAnonymousInnerClass14::
    AnalyzerAnonymousInnerClass14(
        shared_ptr<TestSynonymMapFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSynonymMapFilter::AnalyzerAnonymousInnerClass14::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SynonymFilter>(tokenizer, map_obj, true));
}

void TestSynonymMapFilter::testEmpty() 
{
  shared_ptr<Tokenizer> tokenizer = make_shared<MockTokenizer>();
  tokenizer->setReader(make_shared<StringReader>(L"aa bb"));
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<SynonymFilter>(
        tokenizer, (make_shared<SynonymMap::Builder>(true))->build(), true);
  });
  assertEquals(L"fst must be non-null", expected.what());
}
} // namespace org::apache::lucene::analysis::synonym