using namespace std;

#include "TestSimplePatternTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/MappingCharFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/NormalizeCharMap.h"
#include "../../../../../../java/org/apache/lucene/analysis/pattern/SimplePatternTokenizer.h"

namespace org::apache::lucene::analysis::pattern
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharFilter = org::apache::lucene::analysis::CharFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using MappingCharFilter =
    org::apache::lucene::analysis::charfilter::MappingCharFilter;
using NormalizeCharMap =
    org::apache::lucene::analysis::charfilter::NormalizeCharMap;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using TestUtil = org::apache::lucene::util::TestUtil;
using Automaton = org::apache::lucene::util::automaton::Automaton;

void TestSimplePatternTokenizer::testGreedy() 
{
  shared_ptr<Tokenizer> t = make_shared<SimplePatternTokenizer>(L"(foo)+");
  t->setReader(make_shared<StringReader>(L"bar foofoo baz"));
  assertTokenStreamContents(t, std::deque<wstring>{L"foofoo"},
                            std::deque<int>{4}, std::deque<int>{10});
}

void TestSimplePatternTokenizer::testBigLookahead() 
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  for (int i = 0; i < 100; i++) {
    b->append(L'a');
  }
  b->append(L'b');
  shared_ptr<Tokenizer> t = make_shared<SimplePatternTokenizer>(b->toString());

  b = make_shared<StringBuilder>();
  for (int i = 0; i < 200; i++) {
    b->append(L'a');
  }
  t->setReader(make_shared<StringReader>(b->toString()));
  t->reset();
  assertFalse(t->incrementToken());
}

void TestSimplePatternTokenizer::testOneToken() 
{
  shared_ptr<Tokenizer> t = make_shared<SimplePatternTokenizer>(L".*");
  shared_ptr<CharTermAttribute> termAtt =
      t->getAttribute(CharTermAttribute::typeid);
  wstring s;
  while (true) {
    s = TestUtil::randomUnicodeString(random());
    if (s.length() > 0) {
      break;
    }
  }
  t->setReader(make_shared<StringReader>(s));
  t->reset();
  assertTrue(t->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(s, termAtt->toString());
}

void TestSimplePatternTokenizer::testEmptyStringPatternNoMatch() throw(
    runtime_error)
{
  shared_ptr<Tokenizer> t = make_shared<SimplePatternTokenizer>(L"a*");
  t->setReader(make_shared<StringReader>(L"bbb"));
  t->reset();
  assertFalse(t->incrementToken());
}

void TestSimplePatternTokenizer::testEmptyStringPatternOneMatch() throw(
    runtime_error)
{
  shared_ptr<Tokenizer> t = make_shared<SimplePatternTokenizer>(L"a*");
  shared_ptr<CharTermAttribute> termAtt =
      t->getAttribute(CharTermAttribute::typeid);
  t->setReader(make_shared<StringReader>(L"bbab"));
  t->reset();
  assertTrue(t->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"a", termAtt->toString());
  assertFalse(t->incrementToken());
}

void TestSimplePatternTokenizer::testEndOffset() 
{
  shared_ptr<Tokenizer> t = make_shared<SimplePatternTokenizer>(L"a+");
  shared_ptr<CharTermAttribute> termAtt =
      t->getAttribute(CharTermAttribute::typeid);
  shared_ptr<OffsetAttribute> offsetAtt =
      t->getAttribute(OffsetAttribute::typeid);
  t->setReader(make_shared<StringReader>(L"aaabbb"));
  t->reset();
  assertTrue(t->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"aaa", termAtt->toString());
  assertFalse(t->incrementToken());
  t->end();
  TestUtil::assertEquals(6, offsetAtt->endOffset());
}

void TestSimplePatternTokenizer::testFixedToken() 
{
  shared_ptr<Tokenizer> t = make_shared<SimplePatternTokenizer>(L"aaaa");

  t->setReader(make_shared<StringReader>(L"aaaaaaaaaaaaaaa"));
  assertTokenStreamContents(t, std::deque<wstring>{L"aaaa", L"aaaa", L"aaaa"},
                            std::deque<int>{0, 4, 8},
                            std::deque<int>{4, 8, 12});
}

void TestSimplePatternTokenizer::testBasic() 
{
  wstring qpattern = L"\\'([^\\']+)\\'"; // get stuff between "'"
  std::deque<std::deque<wstring>> tests = {
      std::deque<wstring>{L":", L"boo:and:foo", L": :"},
      std::deque<wstring>{qpattern, L"aaa 'bbb' 'ccc'", L"'bbb' 'ccc'"}};

  for (auto test : tests) {
    shared_ptr<TokenStream> stream =
        make_shared<SimplePatternTokenizer>(test[0]);
    (std::static_pointer_cast<Tokenizer>(stream))
        ->setReader(make_shared<StringReader>(test[1]));
    wstring out = tsToString(stream);

    assertEquals(L"pattern: " + test[0] + L" with input: " + test[1], test[2],
                 out);
  }
}

void TestSimplePatternTokenizer::testNotDeterminized() 
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int start = a->createState();
  int mid1 = a->createState();
  int mid2 = a->createState();
  int end = a->createState();
  a->setAccept(end, true);
  a->addTransition(start, mid1, L'a', L'z');
  a->addTransition(start, mid2, L'a', L'z');
  a->addTransition(mid1, end, L'b');
  a->addTransition(mid2, end, L'b');
  expectThrows(invalid_argument::typeid,
               [&]() { make_shared<SimplePatternTokenizer>(a); });
}

void TestSimplePatternTokenizer::testOffsetCorrection() 
{
  const wstring INPUT = L"G&uuml;nther G&uuml;nther is here";

  // create MappingCharFilter
  deque<wstring> mappingRules = deque<wstring>();
  mappingRules.push_back(L"\"&uuml;\" => \"ü\"");
  shared_ptr<NormalizeCharMap::Builder> builder =
      make_shared<NormalizeCharMap::Builder>();
  builder->add(L"&uuml;", L"ü");
  shared_ptr<NormalizeCharMap> normMap = builder->build();
  shared_ptr<CharFilter> charStream =
      make_shared<MappingCharFilter>(normMap, make_shared<StringReader>(INPUT));

  // create SimplePatternTokenizer
  shared_ptr<Tokenizer> stream =
      make_shared<SimplePatternTokenizer>(L"Günther");
  stream->setReader(charStream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"Günther", L"Günther"},
      std::deque<int>{0, 13}, std::deque<int>{12, 25}, INPUT.length());
}

wstring TestSimplePatternTokenizer::tsToString(
    shared_ptr<TokenStream> in_) 
{
  shared_ptr<StringBuilder> out = make_shared<StringBuilder>();
  shared_ptr<CharTermAttribute> termAtt =
      in_->addAttribute(CharTermAttribute::typeid);
  // extra safety to enforce, that the state is not preserved and also
  // assign bogus values
  in_->clearAttributes();
  termAtt->setEmpty()->append(L"bogusTerm");
  in_->reset();
  while (in_->incrementToken()) {
    if (out->length() > 0) {
      out->append(L' ');
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    out->append(termAtt->toString());
    in_->clearAttributes();
    termAtt->setEmpty()->append(L"bogusTerm");
  }

  delete in_;
  return out->toString();
}

void TestSimplePatternTokenizer::testRandomStrings() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;

  shared_ptr<Analyzer> b =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkRandomData(random(), b, 1000 * RANDOM_MULTIPLIER);
  delete b;
}

TestSimplePatternTokenizer::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestSimplePatternTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSimplePatternTokenizer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<SimplePatternTokenizer>(L"a");
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer);
}

TestSimplePatternTokenizer::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestSimplePatternTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSimplePatternTokenizer::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<SimplePatternTokenizer>(L"a");
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer);
}

void TestSimplePatternTokenizer::testEndLookahead() 
{
  shared_ptr<Tokenizer> t = make_shared<SimplePatternTokenizer>(L"(ab)+");
  t->setReader(make_shared<StringReader>(L"aba"));
  assertTokenStreamContents(t, std::deque<wstring>{L"ab"}, std::deque<int>{0},
                            std::deque<int>{2}, 3);
}
} // namespace org::apache::lucene::analysis::pattern