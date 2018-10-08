using namespace std;

#include "TestGraphTokenizers.h"

namespace org::apache::lucene::analysis
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRef = org::apache::lucene::util::IntsRef;
using Automata = org::apache::lucene::util::automaton::Automata;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using AutomatonTestUtil =
    org::apache::lucene::util::automaton::AutomatonTestUtil;
using Operations = org::apache::lucene::util::automaton::Operations;
using Util = org::apache::lucene::util::fst::Util;
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

void TestGraphTokenizers::GraphTokenizer::reset() 
{
  Tokenizer::reset();
  tokens.clear();
  upto = 0;
}

bool TestGraphTokenizers::GraphTokenizer::incrementToken() 
{
  if (tokens.empty()) {
    fillTokens();
  }
  // System.out.println("graphTokenizer: incr upto=" + upto + " vs " +
  // tokens.size());
  if (upto == tokens.size()) {
    // System.out.println("  END @ " + tokens.size());
    return false;
  }
  shared_ptr<Token> *const t = tokens[upto++];
  // System.out.println("  return token=" + t);
  clearAttributes();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  termAtt->append(t->toString());
  offsetAtt->setOffset(t->startOffset(), t->endOffset());
  posIncrAtt->setPositionIncrement(t->getPositionIncrement());
  posLengthAtt->setPositionLength(t->getPositionLength());
  return true;
}

void TestGraphTokenizers::GraphTokenizer::end() 
{
  Tokenizer::end();
  // NOTE: somewhat... hackish, but we need this to
  // satisfy BTSTC:
  constexpr int lastOffset;
  if (tokens.size() > 0 && !tokens.empty()) {
    lastOffset = tokens[tokens.size() - 1]->endOffset();
  } else {
    lastOffset = 0;
  }
  offsetAtt->setOffset(correctOffset(lastOffset), correctOffset(inputLength));
}

void TestGraphTokenizers::GraphTokenizer::fillTokens() 
{
  shared_ptr<StringBuilder> *const sb = make_shared<StringBuilder>();
  const std::deque<wchar_t> buffer = std::deque<wchar_t>(256);
  while (true) {
    constexpr int count = input->read(buffer);
    if (count == -1) {
      break;
    }
    sb->append(buffer, 0, count);
    // System.out.println("got count=" + count);
  }
  // System.out.println("fillTokens: " + sb);

  inputLength = sb->length();

  const std::deque<wstring> parts = sb->toString()->split(L" ");

  tokens = deque<>();
  int pos = 0;
  int maxPos = -1;
  int offset = 0;
  // System.out.println("again");
  for (auto part : parts) {
    const std::deque<wstring> overlapped = part.split(L"/");
    bool firstAtPos = true;
    int minPosLength = numeric_limits<int>::max();
    for (auto part2 : overlapped) {
      constexpr int colonIndex = part2.find(L':');
      const wstring token;
      constexpr int posLength;
      if (colonIndex != -1) {
        token = part2.substr(0, colonIndex);
        posLength = stoi(part2.substr(1 + colonIndex));
      } else {
        token = part2;
        posLength = 1;
      }
      maxPos = max(maxPos, pos + posLength);
      minPosLength = min(minPosLength, posLength);
      shared_ptr<Token> *const t =
          make_shared<Token>(token, offset, offset + 2 * posLength - 1);
      t->setPositionLength(posLength);
      t->setPositionIncrement(firstAtPos ? 1 : 0);
      firstAtPos = false;
      // System.out.println("  add token=" + t + " startOff=" + t.startOffset()
      // + " endOff=" + t.endOffset());
      tokens.push_back(t);
    }
    pos += minPosLength;
    offset = 2 * pos;
  }
  assert((maxPos <= pos,
          L"input string mal-formed: posLength>1 tokens hang over the end"));
}

void TestGraphTokenizers::testMockGraphTokenFilterBasic() 
{

  for (int iter = 0; iter < 10 * RANDOM_MULTIPLIER; iter++) {

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
    }

    // Make new analyzer each time, because MGTF has fixed
    // seed:
    shared_ptr<Analyzer> *const a =
        make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

    checkAnalysisConsistency(random(), a, false, L"a b c d e f g h i j k");
  }
}

TestGraphTokenizers::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestGraphTokenizers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestGraphTokenizers::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const t =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> *const t2 =
      make_shared<MockGraphTokenFilter>(random(), t);
  return make_shared<TokenStreamComponents>(t, t2);
}

void TestGraphTokenizers::testMockGraphTokenFilterOnGraphInput() throw(
    runtime_error)
{
  for (int iter = 0; iter < 100 * RANDOM_MULTIPLIER; iter++) {

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
    }

    // Make new analyzer each time, because MGTF has fixed
    // seed:
    shared_ptr<Analyzer> *const a =
        make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());

    checkAnalysisConsistency(random(), a, false,
                             L"a/x:3 c/y:2 d e f/z:4 g h i j k");
  }
}

TestGraphTokenizers::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<TestGraphTokenizers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestGraphTokenizers::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const t = make_shared<GraphTokenizer>();
  shared_ptr<TokenStream> *const t2 =
      make_shared<MockGraphTokenFilter>(random(), t);
  return make_shared<TokenStreamComponents>(t, t2);
}

TestGraphTokenizers::RemoveATokens::RemoveATokens(shared_ptr<TokenStream> in_)
    : TokenFilter(in_)
{
}

void TestGraphTokenizers::RemoveATokens::reset() 
{
  TokenFilter::reset();
  pendingPosInc = 0;
}

void TestGraphTokenizers::RemoveATokens::end() 
{
  TokenFilter::end();
  posIncAtt->setPositionIncrement(pendingPosInc +
                                  posIncAtt->getPositionIncrement());
}

bool TestGraphTokenizers::RemoveATokens::incrementToken() 
{
  while (true) {
    constexpr bool gotOne = input->incrementToken();
    if (!gotOne) {
      return false;
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    else if (termAtt->toString()->equals(L"a")) {
      pendingPosInc += posIncAtt->getPositionIncrement();
    } else {
      posIncAtt->setPositionIncrement(pendingPosInc +
                                      posIncAtt->getPositionIncrement());
      pendingPosInc = 0;
      return true;
    }
  }
}

void TestGraphTokenizers::testMockGraphTokenFilterBeforeHoles() throw(
    runtime_error)
{
  for (int iter = 0; iter < 100 * RANDOM_MULTIPLIER; iter++) {

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
    }

    // Make new analyzer each time, because MGTF has fixed
    // seed:
    shared_ptr<Analyzer> *const a =
        make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

    shared_ptr<Random> random = TestGraphTokenizers::random();
    checkAnalysisConsistency(random, a, false, L"a b c d e f g h i j k");
    checkAnalysisConsistency(random, a, false, L"x y a b c d e f g h i j k");
    checkAnalysisConsistency(random, a, false, L"a b c d e f g h i j k a");
    checkAnalysisConsistency(random, a, false, L"a b c d e f g h i j k a x y");
  }
}

TestGraphTokenizers::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestGraphTokenizers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestGraphTokenizers::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const t =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> *const t2 =
      make_shared<MockGraphTokenFilter>(random(), t);
  shared_ptr<TokenStream> *const t3 = make_shared<RemoveATokens>(t2);
  return make_shared<TokenStreamComponents>(t, t3);
}

void TestGraphTokenizers::testMockGraphTokenFilterAfterHoles() throw(
    runtime_error)
{
  for (int iter = 0; iter < 100 * RANDOM_MULTIPLIER; iter++) {

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
    }

    // Make new analyzer each time, because MGTF has fixed
    // seed:
    shared_ptr<Analyzer> *const a =
        make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());

    shared_ptr<Random> random = TestGraphTokenizers::random();
    checkAnalysisConsistency(random, a, false, L"a b c d e f g h i j k");
    checkAnalysisConsistency(random, a, false, L"x y a b c d e f g h i j k");
    checkAnalysisConsistency(random, a, false, L"a b c d e f g h i j k a");
    checkAnalysisConsistency(random, a, false, L"a b c d e f g h i j k a x y");
  }
}

TestGraphTokenizers::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<TestGraphTokenizers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestGraphTokenizers::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const t =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> *const t2 = make_shared<RemoveATokens>(t);
  shared_ptr<TokenStream> *const t3 =
      make_shared<MockGraphTokenFilter>(random(), t2);
  return make_shared<TokenStreamComponents>(t, t3);
}

void TestGraphTokenizers::testMockGraphTokenFilterRandom() 
{
  for (int iter = 0; iter < 3 * RANDOM_MULTIPLIER; iter++) {

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
    }

    // Make new analyzer each time, because MGTF has fixed
    // seed:
    shared_ptr<Analyzer> *const a =
        make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());

    shared_ptr<Random> random = TestGraphTokenizers::random();
    checkRandomData(random, a, 5, atLeast(100));
  }
}

TestGraphTokenizers::AnalyzerAnonymousInnerClass3::AnalyzerAnonymousInnerClass3(
    shared_ptr<TestGraphTokenizers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestGraphTokenizers::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const t =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> *const t2 =
      make_shared<MockGraphTokenFilter>(random(), t);
  return make_shared<TokenStreamComponents>(t, t2);
}

void TestGraphTokenizers::testDoubleMockGraphTokenFilterRandom() throw(
    runtime_error)
{
  for (int iter = 0; iter < 3 * RANDOM_MULTIPLIER; iter++) {

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
    }

    // Make new analyzer each time, because MGTF has fixed
    // seed:
    shared_ptr<Analyzer> *const a =
        make_shared<AnalyzerAnonymousInnerClass4>(shared_from_this());

    shared_ptr<Random> random = TestGraphTokenizers::random();
    checkRandomData(random, a, 5, atLeast(100));
  }
}

TestGraphTokenizers::AnalyzerAnonymousInnerClass4::AnalyzerAnonymousInnerClass4(
    shared_ptr<TestGraphTokenizers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestGraphTokenizers::AnalyzerAnonymousInnerClass4::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const t =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> *const t1 =
      make_shared<MockGraphTokenFilter>(random(), t);
  shared_ptr<TokenStream> *const t2 =
      make_shared<MockGraphTokenFilter>(random(), t1);
  return make_shared<TokenStreamComponents>(t, t2);
}

void TestGraphTokenizers::testMockGraphTokenFilterBeforeHolesRandom() throw(
    runtime_error)
{
  for (int iter = 0; iter < 3 * RANDOM_MULTIPLIER; iter++) {

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
    }

    // Make new analyzer each time, because MGTF has fixed
    // seed:
    shared_ptr<Analyzer> *const a =
        make_shared<AnalyzerAnonymousInnerClass5>(shared_from_this());

    shared_ptr<Random> random = TestGraphTokenizers::random();
    checkRandomData(random, a, 5, atLeast(100));
  }
}

TestGraphTokenizers::AnalyzerAnonymousInnerClass5::AnalyzerAnonymousInnerClass5(
    shared_ptr<TestGraphTokenizers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestGraphTokenizers::AnalyzerAnonymousInnerClass5::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const t =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> *const t1 =
      make_shared<MockGraphTokenFilter>(random(), t);
  shared_ptr<TokenStream> *const t2 =
      make_shared<MockHoleInjectingTokenFilter>(random(), t1);
  return make_shared<TokenStreamComponents>(t, t2);
}

void TestGraphTokenizers::testMockGraphTokenFilterAfterHolesRandom() throw(
    runtime_error)
{
  for (int iter = 0; iter < 3 * RANDOM_MULTIPLIER; iter++) {

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
    }

    // Make new analyzer each time, because MGTF has fixed
    // seed:
    shared_ptr<Analyzer> *const a =
        make_shared<AnalyzerAnonymousInnerClass6>(shared_from_this());

    shared_ptr<Random> random = TestGraphTokenizers::random();
    checkRandomData(random, a, 5, atLeast(100));
  }
}

TestGraphTokenizers::AnalyzerAnonymousInnerClass6::AnalyzerAnonymousInnerClass6(
    shared_ptr<TestGraphTokenizers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestGraphTokenizers::AnalyzerAnonymousInnerClass6::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const t =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> *const t1 =
      make_shared<MockHoleInjectingTokenFilter>(random(), t);
  shared_ptr<TokenStream> *const t2 =
      make_shared<MockGraphTokenFilter>(random(), t1);
  return make_shared<TokenStreamComponents>(t, t2);
}

shared_ptr<Token> TestGraphTokenizers::token(const wstring &term, int posInc,
                                             int posLength)
{
  shared_ptr<Token> *const t = make_shared<Token>(term, 0, 0);
  t->setPositionIncrement(posInc);
  t->setPositionLength(posLength);
  return t;
}

shared_ptr<Token> TestGraphTokenizers::token(const wstring &term, int posInc,
                                             int posLength, int startOffset,
                                             int endOffset)
{
  shared_ptr<Token> *const t = make_shared<Token>(term, startOffset, endOffset);
  t->setPositionIncrement(posInc);
  t->setPositionLength(posLength);
  return t;
}

void TestGraphTokenizers::testSingleToken() 
{
  shared_ptr<TokenStream> *const ts = make_shared<CannedTokenStream>(
      std::deque<std::shared_ptr<Token>>{token(L"abc", 1, 1)});
  assertSameLanguage(s2a(L"abc"), ts);
}

void TestGraphTokenizers::testMultipleHoles() 
{
  shared_ptr<TokenStream> *const ts =
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"a", 1, 1), token(L"b", 3, 1)});
  assertSameLanguage(
      join({s2a(L"a"), SEP_A, HOLE_A, SEP_A, HOLE_A, SEP_A, s2a(L"b")}), ts);
}

void TestGraphTokenizers::testSynOverMultipleHoles() 
{
  shared_ptr<TokenStream> *const ts =
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"a", 1, 1), token(L"x", 0, 3), token(L"b", 3, 1)});
  shared_ptr<Automaton> *const a1 =
      join({s2a(L"a"), SEP_A, HOLE_A, SEP_A, HOLE_A, SEP_A, s2a(L"b")});
  shared_ptr<Automaton> *const a2 = join({s2a(L"x"), SEP_A, s2a(L"b")});
  assertSameLanguage(Operations::union_(a1, a2), ts);
}

const shared_ptr<org::apache::lucene::util::automaton::Automaton>
    TestGraphTokenizers::SEP_A =
        org::apache::lucene::util::automaton::Automata::makeChar(
            TokenStreamToAutomaton::POS_SEP);
const shared_ptr<org::apache::lucene::util::automaton::Automaton>
    TestGraphTokenizers::HOLE_A =
        org::apache::lucene::util::automaton::Automata::makeChar(
            TokenStreamToAutomaton::HOLE);

shared_ptr<Automaton> TestGraphTokenizers::join(deque<wstring> &strings)
{
  deque<std::shared_ptr<Automaton>> as = deque<std::shared_ptr<Automaton>>();
  for (wstring s : strings) {
    as.push_back(s2a(s));
    as.push_back(SEP_A);
  }
  as.pop_back();
  return Operations::concatenate(as);
}

shared_ptr<Automaton> TestGraphTokenizers::join(deque<Automaton> &as)
{
  return Operations::concatenate(Arrays::asList(as));
}

shared_ptr<Automaton> TestGraphTokenizers::s2a(const wstring &s)
{
  return Automata::makeString(s);
}

void TestGraphTokenizers::testTwoTokens() 
{
  shared_ptr<TokenStream> *const ts =
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"abc", 1, 1), token(L"def", 1, 1)});
  assertSameLanguage(join({L"abc", L"def"}), ts);
}

void TestGraphTokenizers::testHole() 
{

  shared_ptr<TokenStream> *const ts =
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"abc", 1, 1), token(L"def", 2, 1)});
  assertSameLanguage(join({s2a(L"abc"), SEP_A, HOLE_A, SEP_A, s2a(L"def")}),
                     ts);
}

void TestGraphTokenizers::testOverlappedTokensSausage() 
{

  // Two tokens on top of each other (sausage):
  shared_ptr<TokenStream> *const ts =
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"abc", 1, 1), token(L"xyz", 0, 1)});
  shared_ptr<Automaton> *const a1 = s2a(L"abc");
  shared_ptr<Automaton> *const a2 = s2a(L"xyz");
  assertSameLanguage(Operations::union_(a1, a2), ts);
}

void TestGraphTokenizers::testOverlappedTokensLattice() 
{

  shared_ptr<TokenStream> *const ts =
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"abc", 1, 1), token(L"xyz", 0, 2), token(L"def", 1, 1)});
  shared_ptr<Automaton> *const a1 = s2a(L"xyz");
  shared_ptr<Automaton> *const a2 = join({L"abc", L"def"});
  assertSameLanguage(Operations::union_(a1, a2), ts);
}

void TestGraphTokenizers::testSynOverHole() 
{

  shared_ptr<TokenStream> *const ts =
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"a", 1, 1), token(L"X", 0, 2), token(L"b", 2, 1)});
  shared_ptr<Automaton> *const a1 =
      Operations::union_(join({s2a(L"a"), SEP_A, HOLE_A}), s2a(L"X"));
  shared_ptr<Automaton> *const expected =
      Operations::concatenate(a1, join({SEP_A, s2a(L"b")}));
  assertSameLanguage(expected, ts);
}

void TestGraphTokenizers::testSynOverHole2() 
{

  shared_ptr<TokenStream> *const ts =
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"xyz", 1, 1), token(L"abc", 0, 3), token(L"def", 2, 1)});
  shared_ptr<Automaton> *const expected = Operations::union_(
      join({s2a(L"xyz"), SEP_A, HOLE_A, SEP_A, s2a(L"def")}), s2a(L"abc"));
  assertSameLanguage(expected, ts);
}

void TestGraphTokenizers::testOverlappedTokensLattice2() 
{

  shared_ptr<TokenStream> *const ts =
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"abc", 1, 1), token(L"xyz", 0, 3), token(L"def", 1, 1),
          token(L"ghi", 1, 1)});
  shared_ptr<Automaton> *const a1 = s2a(L"xyz");
  shared_ptr<Automaton> *const a2 = join({L"abc", L"def", L"ghi"});
  assertSameLanguage(Operations::union_(a1, a2), ts);
}

void TestGraphTokenizers::testToDot() 
{
  shared_ptr<TokenStream> *const ts = make_shared<CannedTokenStream>(
      std::deque<std::shared_ptr<Token>>{token(L"abc", 1, 1, 0, 4)});
  shared_ptr<StringWriter> w = make_shared<StringWriter>();
  (make_shared<TokenStreamToDot>(L"abcd", ts, make_shared<PrintWriter>(w)))
      ->toDot();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(w->toString()->find(L"abc / abcd") != -1);
}

void TestGraphTokenizers::testStartsWithHole() 
{
  shared_ptr<TokenStream> *const ts = make_shared<CannedTokenStream>(
      std::deque<std::shared_ptr<Token>>{token(L"abc", 2, 1)});
  assertSameLanguage(join({HOLE_A, SEP_A, s2a(L"abc")}), ts);
}

void TestGraphTokenizers::testEndsWithHole() 
{
  shared_ptr<TokenStream> *const ts = make_shared<CannedTokenStream>(
      1, 0, std::deque<std::shared_ptr<Token>>{token(L"abc", 2, 1)});
  assertSameLanguage(join({HOLE_A, SEP_A, s2a(L"abc"), SEP_A, HOLE_A}), ts);
}

void TestGraphTokenizers::testSynHangingOverEnd() 
{
  shared_ptr<TokenStream> *const ts =
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"a", 1, 1), token(L"X", 0, 10)});
  assertSameLanguage(Operations::union_(s2a(L"a"), s2a(L"X")), ts);
}

shared_ptr<Set<wstring>>
TestGraphTokenizers::toPathStrings(shared_ptr<Automaton> a)
{
  shared_ptr<BytesRefBuilder> scratchBytesRefBuilder =
      make_shared<BytesRefBuilder>();
  shared_ptr<Set<wstring>> paths = unordered_set<wstring>();
  for (auto ir : AutomatonTestUtil::getFiniteStringsRecursive(a, -1)) {
    paths->add(StringHelper::replace(
        Util::toBytesRef(ir, scratchBytesRefBuilder)->utf8ToString(),
        static_cast<wchar_t>(TokenStreamToAutomaton::POS_SEP), L' '));
  }
  return paths;
}

void TestGraphTokenizers::assertSameLanguage(
    shared_ptr<Automaton> expected,
    shared_ptr<TokenStream> ts) 
{
  assertSameLanguage(expected,
                     (make_shared<TokenStreamToAutomaton>())->toAutomaton(ts));
}

void TestGraphTokenizers::assertSameLanguage(shared_ptr<Automaton> expected,
                                             shared_ptr<Automaton> actual)
{
  shared_ptr<Automaton> expectedDet = Operations::determinize(
      Operations::removeDeadStates(expected),
      AutomatonTestUtil::DEFAULT_MAX_DETERMINIZED_STATES);
  shared_ptr<Automaton> actualDet = Operations::determinize(
      Operations::removeDeadStates(actual),
      AutomatonTestUtil::DEFAULT_MAX_DETERMINIZED_STATES);
  if (Operations::sameLanguage(expectedDet, actualDet) == false) {
    shared_ptr<Set<wstring>> expectedPaths = toPathStrings(expectedDet);
    shared_ptr<Set<wstring>> actualPaths = toPathStrings(actualDet);
    shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
    b->append(L"expected:\n");
    for (auto path : expectedPaths) {
      b->append(L"  ");
      b->append(path);
      if (actualPaths->contains(path) == false) {
        b->append(L" [missing!]");
      }
      b->append(L'\n');
    }
    b->append(L"actual:\n");
    for (auto path : actualPaths) {
      b->append(L"  ");
      b->append(path);
      if (expectedPaths->contains(path) == false) {
        b->append(L" [unexpected!]");
      }
      b->append(L'\n');
    }
    fail(L"accepted language is different:\n\n" + b->toString());
  }
}

void TestGraphTokenizers::testTokenStreamGraphWithHoles() 
{
  shared_ptr<TokenStream> *const ts =
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"abc", 1, 1), token(L"xyz", 1, 8), token(L"def", 1, 1),
          token(L"ghi", 1, 1)});
  assertSameLanguage(
      Operations::union_(join({s2a(L"abc"), SEP_A, s2a(L"xyz")}),
                         join({s2a(L"abc"), SEP_A, HOLE_A, SEP_A, s2a(L"def"),
                               SEP_A, s2a(L"ghi")})),
      ts);
}
} // namespace org::apache::lucene::analysis