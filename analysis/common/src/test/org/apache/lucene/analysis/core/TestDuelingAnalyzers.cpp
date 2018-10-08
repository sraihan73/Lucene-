using namespace std;

#include "TestDuelingAnalyzers.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/CharacterRunAutomaton.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/Operations.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockReaderWrapper.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/LetterTokenizer.h"

namespace org::apache::lucene::analysis::core
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockReaderWrapper = org::apache::lucene::analysis::MockReaderWrapper;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;
using Operations = org::apache::lucene::util::automaton::Operations;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using Automaton = org::apache::lucene::util::automaton::Automaton;

void TestDuelingAnalyzers::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  shared_ptr<Automaton> single = make_shared<Automaton>();
  int initial = single->createState();
  int accept = single->createState();
  single->setAccept(accept, true);

  // build an automaton matching this jvm's letter definition
  for (int i = 0; i <= 0x10FFFF; i++) {
    if (isalpha(i)) {
      single->addTransition(initial, accept, i);
    }
  }
  shared_ptr<Automaton> repeat = Operations::repeat(single);
  jvmLetter = make_shared<CharacterRunAutomaton>(repeat);
}

void TestDuelingAnalyzers::testLetterAscii() 
{
  shared_ptr<Random> random = TestDuelingAnalyzers::random();
  shared_ptr<Analyzer> left =
      make_shared<MockAnalyzer>(random, jvmLetter, false);
  shared_ptr<Analyzer> right =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  for (int i = 0; i < 1000; i++) {
    wstring s = TestUtil::randomSimpleString(random);
    assertEquals(s, left->tokenStream(L"foo", newStringReader(s)),
                 right->tokenStream(L"foo", newStringReader(s)));
  }
  IOUtils::close({left, right});
}

TestDuelingAnalyzers::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestDuelingAnalyzers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestDuelingAnalyzers::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<LetterTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory());
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void TestDuelingAnalyzers::testLetterAsciiHuge() 
{
  shared_ptr<Random> random = TestDuelingAnalyzers::random();
  int maxLength = 8192; // CharTokenizer.IO_BUFFER_SIZE*2
  shared_ptr<MockAnalyzer> left =
      make_shared<MockAnalyzer>(random, jvmLetter, false);
  left->setMaxTokenLength(255); // match CharTokenizer's max token length
  shared_ptr<Analyzer> right =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  int numIterations = atLeast(50);
  for (int i = 0; i < numIterations; i++) {
    wstring s = TestUtil::randomSimpleString(random, maxLength);
    assertEquals(s, left->tokenStream(L"foo", newStringReader(s)),
                 right->tokenStream(L"foo", newStringReader(s)));
  }
  IOUtils::close({left, right});
}

TestDuelingAnalyzers::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(shared_ptr<TestDuelingAnalyzers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestDuelingAnalyzers::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<LetterTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory());
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void TestDuelingAnalyzers::testLetterHtmlish() 
{
  shared_ptr<Random> random = TestDuelingAnalyzers::random();
  shared_ptr<Analyzer> left =
      make_shared<MockAnalyzer>(random, jvmLetter, false);
  shared_ptr<Analyzer> right =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  for (int i = 0; i < 1000; i++) {
    wstring s = TestUtil::randomHtmlishString(random, 20);
    assertEquals(s, left->tokenStream(L"foo", newStringReader(s)),
                 right->tokenStream(L"foo", newStringReader(s)));
  }
  IOUtils::close({left, right});
}

TestDuelingAnalyzers::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(shared_ptr<TestDuelingAnalyzers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestDuelingAnalyzers::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<LetterTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory());
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void TestDuelingAnalyzers::testLetterHtmlishHuge() 
{
  shared_ptr<Random> random = TestDuelingAnalyzers::random();
  int maxLength = 1024; // this is number of elements, not chars!
  shared_ptr<MockAnalyzer> left =
      make_shared<MockAnalyzer>(random, jvmLetter, false);
  left->setMaxTokenLength(255); // match CharTokenizer's max token length
  shared_ptr<Analyzer> right =
      make_shared<AnalyzerAnonymousInnerClass4>(shared_from_this());
  int numIterations = atLeast(50);
  for (int i = 0; i < numIterations; i++) {
    wstring s = TestUtil::randomHtmlishString(random, maxLength);
    assertEquals(s, left->tokenStream(L"foo", newStringReader(s)),
                 right->tokenStream(L"foo", newStringReader(s)));
  }
  IOUtils::close({left, right});
}

TestDuelingAnalyzers::AnalyzerAnonymousInnerClass4::
    AnalyzerAnonymousInnerClass4(shared_ptr<TestDuelingAnalyzers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestDuelingAnalyzers::AnalyzerAnonymousInnerClass4::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<LetterTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory());
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void TestDuelingAnalyzers::testLetterUnicode() 
{
  shared_ptr<Random> random = TestDuelingAnalyzers::random();
  shared_ptr<Analyzer> left = make_shared<MockAnalyzer>(
      TestDuelingAnalyzers::random(), jvmLetter, false);
  shared_ptr<Analyzer> right =
      make_shared<AnalyzerAnonymousInnerClass5>(shared_from_this());
  for (int i = 0; i < 1000; i++) {
    wstring s = TestUtil::randomUnicodeString(random);
    assertEquals(s, left->tokenStream(L"foo", newStringReader(s)),
                 right->tokenStream(L"foo", newStringReader(s)));
  }
  IOUtils::close({left, right});
}

TestDuelingAnalyzers::AnalyzerAnonymousInnerClass5::
    AnalyzerAnonymousInnerClass5(shared_ptr<TestDuelingAnalyzers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestDuelingAnalyzers::AnalyzerAnonymousInnerClass5::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<LetterTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory());
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void TestDuelingAnalyzers::testLetterUnicodeHuge() 
{
  shared_ptr<Random> random = TestDuelingAnalyzers::random();
  int maxLength = 4300; // CharTokenizer.IO_BUFFER_SIZE + fudge
  shared_ptr<MockAnalyzer> left =
      make_shared<MockAnalyzer>(random, jvmLetter, false);
  left->setMaxTokenLength(255); // match CharTokenizer's max token length
  shared_ptr<Analyzer> right =
      make_shared<AnalyzerAnonymousInnerClass6>(shared_from_this());
  int numIterations = atLeast(50);
  for (int i = 0; i < numIterations; i++) {
    wstring s = TestUtil::randomUnicodeString(random, maxLength);
    assertEquals(s, left->tokenStream(L"foo", newStringReader(s)),
                 right->tokenStream(L"foo", newStringReader(s)));
  }
  IOUtils::close({left, right});
}

TestDuelingAnalyzers::AnalyzerAnonymousInnerClass6::
    AnalyzerAnonymousInnerClass6(shared_ptr<TestDuelingAnalyzers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestDuelingAnalyzers::AnalyzerAnonymousInnerClass6::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<LetterTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory());
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void TestDuelingAnalyzers::assertEquals(
    const wstring &s, shared_ptr<TokenStream> left,
    shared_ptr<TokenStream> right) 
{
  left->reset();
  right->reset();
  shared_ptr<CharTermAttribute> leftTerm =
      left->addAttribute(CharTermAttribute::typeid);
  shared_ptr<CharTermAttribute> rightTerm =
      right->addAttribute(CharTermAttribute::typeid);
  shared_ptr<OffsetAttribute> leftOffset =
      left->addAttribute(OffsetAttribute::typeid);
  shared_ptr<OffsetAttribute> rightOffset =
      right->addAttribute(OffsetAttribute::typeid);
  shared_ptr<PositionIncrementAttribute> leftPos =
      left->addAttribute(PositionIncrementAttribute::typeid);
  shared_ptr<PositionIncrementAttribute> rightPos =
      right->addAttribute(PositionIncrementAttribute::typeid);

  while (left->incrementToken()) {
    assertTrue(L"wrong number of tokens for input: " + s,
               right->incrementToken());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"wrong term text for input: " + s, leftTerm->toString(),
                 rightTerm->toString());
    assertEquals(L"wrong position for input: " + s,
                 leftPos->getPositionIncrement(),
                 rightPos->getPositionIncrement());
    assertEquals(L"wrong start offset for input: " + s,
                 leftOffset->startOffset(), rightOffset->startOffset());
    assertEquals(L"wrong end offset for input: " + s, leftOffset->endOffset(),
                 rightOffset->endOffset());
  };
  assertFalse(L"wrong number of tokens for input: " + s,
              right->incrementToken());
  left->end();
  right->end();
  assertEquals(L"wrong final offset for input: " + s, leftOffset->endOffset(),
               rightOffset->endOffset());
  delete left;
  delete right;
}

shared_ptr<Reader> TestDuelingAnalyzers::newStringReader(const wstring &s)
{
  shared_ptr<Random> random = TestDuelingAnalyzers::random();
  shared_ptr<Reader> r = make_shared<StringReader>(s);
  if (random->nextBoolean()) {
    r = make_shared<MockReaderWrapper>(random, r);
  }
  return r;
}
} // namespace org::apache::lucene::analysis::core