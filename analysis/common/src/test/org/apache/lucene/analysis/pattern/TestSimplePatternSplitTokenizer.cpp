using namespace std;

#include "TestSimplePatternSplitTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/MappingCharFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/NormalizeCharMap.h"
#include "../../../../../../java/org/apache/lucene/analysis/pattern/SimplePatternSplitTokenizer.h"

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

void TestSimplePatternSplitTokenizer::testGreedy() 
{
  shared_ptr<Tokenizer> t = make_shared<SimplePatternSplitTokenizer>(L"(foo)+");
  t->setReader(make_shared<StringReader>(L"bar foofoo baz"));
  assertTokenStreamContents(t, std::deque<wstring>{L"bar ", L" baz"},
                            std::deque<int>{0, 10}, std::deque<int>{4, 14});
}

void TestSimplePatternSplitTokenizer::testBackToBack() 
{
  shared_ptr<Tokenizer> t = make_shared<SimplePatternSplitTokenizer>(L"foo");
  t->setReader(make_shared<StringReader>(L"bar foofoo baz"));
  assertTokenStreamContents(t, std::deque<wstring>{L"bar ", L" baz"},
                            std::deque<int>{0, 10}, std::deque<int>{4, 14});
}

void TestSimplePatternSplitTokenizer::testBigLookahead() 
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  for (int i = 0; i < 100; i++) {
    b->append(L'a');
  }
  b->append(L'b');
  shared_ptr<Tokenizer> t =
      make_shared<SimplePatternSplitTokenizer>(b->toString());
  shared_ptr<CharTermAttribute> termAtt =
      t->getAttribute(CharTermAttribute::typeid);

  b = make_shared<StringBuilder>();
  for (int i = 0; i < 200; i++) {
    b->append(L'a');
  }
  t->setReader(make_shared<StringReader>(b->toString()));
  t->reset();
  assertTrue(t->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(b->toString(), termAtt->toString());
  assertFalse(t->incrementToken());
}

void TestSimplePatternSplitTokenizer::testNoTokens() 
{
  shared_ptr<Tokenizer> t = make_shared<SimplePatternSplitTokenizer>(L".*");
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
  assertFalse(t->incrementToken());
}

void TestSimplePatternSplitTokenizer::testEmptyStringPatternNoMatch() throw(
    runtime_error)
{
  shared_ptr<Tokenizer> t = make_shared<SimplePatternSplitTokenizer>(L"a*");
  shared_ptr<CharTermAttribute> termAtt =
      t->getAttribute(CharTermAttribute::typeid);
  t->setReader(make_shared<StringReader>(L"bbb"));
  t->reset();
  assertTrue(t->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"bbb", termAtt->toString());
  assertFalse(t->incrementToken());
}

void TestSimplePatternSplitTokenizer::testSplitSingleCharWhitespace() throw(
    runtime_error)
{
  shared_ptr<Tokenizer> t =
      make_shared<SimplePatternSplitTokenizer>(L"[ \t\r\n]");
  shared_ptr<CharTermAttribute> termAtt =
      t->getAttribute(CharTermAttribute::typeid);
  t->setReader(make_shared<StringReader>(L"a \tb   c"));
  assertTokenStreamContents(t, std::deque<wstring>{L"a", L"b", L"c"},
                            std::deque<int>{0, 3, 7},
                            std::deque<int>{1, 4, 8});
}

void TestSimplePatternSplitTokenizer::testSplitMultiCharWhitespace() throw(
    runtime_error)
{
  shared_ptr<Tokenizer> t =
      make_shared<SimplePatternSplitTokenizer>(L"[ \t\r\n]*");
  shared_ptr<CharTermAttribute> termAtt =
      t->getAttribute(CharTermAttribute::typeid);
  t->setReader(make_shared<StringReader>(L"a \tb   c"));
  assertTokenStreamContents(t, std::deque<wstring>{L"a", L"b", L"c"},
                            std::deque<int>{0, 3, 7},
                            std::deque<int>{1, 4, 8});
}

void TestSimplePatternSplitTokenizer::testLeadingNonToken() 
{
  shared_ptr<Tokenizer> t =
      make_shared<SimplePatternSplitTokenizer>(L"[ \t\r\n]*");
  shared_ptr<CharTermAttribute> termAtt =
      t->getAttribute(CharTermAttribute::typeid);
  t->setReader(make_shared<StringReader>(L"    a c"));
  assertTokenStreamContents(t, std::deque<wstring>{L"a", L"c"},
                            std::deque<int>{4, 6}, std::deque<int>{5, 7});
}

void TestSimplePatternSplitTokenizer::testTrailingNonToken() throw(
    runtime_error)
{
  shared_ptr<Tokenizer> t =
      make_shared<SimplePatternSplitTokenizer>(L"[ \t\r\n]*");
  shared_ptr<CharTermAttribute> termAtt =
      t->getAttribute(CharTermAttribute::typeid);
  t->setReader(make_shared<StringReader>(L"a c   "));
  assertTokenStreamContents(t, std::deque<wstring>{L"a", L"c"},
                            std::deque<int>{0, 2}, std::deque<int>{1, 3});
}

void TestSimplePatternSplitTokenizer::testEmptyStringPatternOneMatch() throw(
    runtime_error)
{
  shared_ptr<Tokenizer> t = make_shared<SimplePatternSplitTokenizer>(L"a*");
  shared_ptr<CharTermAttribute> termAtt =
      t->getAttribute(CharTermAttribute::typeid);
  t->setReader(make_shared<StringReader>(L"bbab"));
  assertTokenStreamContents(t, std::deque<wstring>{L"bb", L"b"},
                            std::deque<int>{0, 3}, std::deque<int>{2, 4});
}

void TestSimplePatternSplitTokenizer::testEndOffset() 
{
  shared_ptr<Tokenizer> t = make_shared<SimplePatternSplitTokenizer>(L"a+");
  shared_ptr<CharTermAttribute> termAtt =
      t->getAttribute(CharTermAttribute::typeid);
  shared_ptr<OffsetAttribute> offsetAtt =
      t->getAttribute(OffsetAttribute::typeid);
  t->setReader(make_shared<StringReader>(L"aaabbb"));
  t->reset();
  assertTrue(t->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"bbb", termAtt->toString());
  assertFalse(t->incrementToken());
  t->end();
  TestUtil::assertEquals(6, offsetAtt->endOffset());
}

void TestSimplePatternSplitTokenizer::testFixedToken() 
{
  shared_ptr<Tokenizer> t = make_shared<SimplePatternSplitTokenizer>(L"aaaa");

  t->setReader(make_shared<StringReader>(L"aaaaaaaaaaaaaaa"));
  assertTokenStreamContents(t, std::deque<wstring>{L"aaa"},
                            std::deque<int>{12}, std::deque<int>{15});
}

void TestSimplePatternSplitTokenizer::testBasic() 
{
  std::deque<std::deque<wstring>> tests = {
      std::deque<wstring>{L"--", L"aaa--bbb--ccc", L"aaa bbb ccc"},
      std::deque<wstring>{L":", L"aaa:bbb:ccc", L"aaa bbb ccc"},
      std::deque<wstring>{L":", L"boo:and:foo", L"boo and foo"},
      std::deque<wstring>{L"o", L"boo:and:foo", L"b :and:f"}};

  for (auto test : tests) {
    shared_ptr<TokenStream> stream =
        make_shared<SimplePatternSplitTokenizer>(test[0]);
    (std::static_pointer_cast<Tokenizer>(stream))
        ->setReader(make_shared<StringReader>(test[1]));
    wstring out = tsToString(stream);
    assertEquals(L"pattern: " + test[0] + L" with input: " + test[1], test[2],
                 out);
  }
}

void TestSimplePatternSplitTokenizer::testNotDeterminized() 
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
               [&]() { make_shared<SimplePatternSplitTokenizer>(a); });
}

void TestSimplePatternSplitTokenizer::testOffsetCorrection() throw(
    runtime_error)
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

  // create SimplePatternSplitTokenizer
  shared_ptr<Tokenizer> stream =
      make_shared<SimplePatternSplitTokenizer>(L"Günther");
  stream->setReader(charStream);
  assertTokenStreamContents(stream, std::deque<wstring>{L" ", L" is here"},
                            std::deque<int>{12, 25}, std::deque<int>{13, 33},
                            INPUT.length());
}

wstring TestSimplePatternSplitTokenizer::tsToString(
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

void TestSimplePatternSplitTokenizer::testRandomStrings() 
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

TestSimplePatternSplitTokenizer::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestSimplePatternSplitTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSimplePatternSplitTokenizer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<SimplePatternSplitTokenizer>(L"a");
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer);
}

TestSimplePatternSplitTokenizer::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestSimplePatternSplitTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSimplePatternSplitTokenizer::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<SimplePatternSplitTokenizer>(L"a");
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer);
}

void TestSimplePatternSplitTokenizer::testEndLookahead() 
{
  shared_ptr<Tokenizer> t = make_shared<SimplePatternSplitTokenizer>(L"(ab)+");
  t->setReader(make_shared<StringReader>(L"aba"));
  assertTokenStreamContents(t, std::deque<wstring>{L"a"}, std::deque<int>{2},
                            std::deque<int>{3}, 3);
}
} // namespace org::apache::lucene::analysis::pattern