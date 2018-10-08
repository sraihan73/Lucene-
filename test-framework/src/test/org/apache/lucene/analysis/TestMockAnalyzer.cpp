using namespace std;

#include "TestMockAnalyzer.h"

namespace org::apache::lucene::analysis
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Fields = org::apache::lucene::index::Fields;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using LeafReader = org::apache::lucene::index::LeafReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;
using Automata = org::apache::lucene::util::automaton::Automata;
using AutomatonTestUtil =
    org::apache::lucene::util::automaton::AutomatonTestUtil;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using Operations = org::apache::lucene::util::automaton::Operations;
using RegExp = org::apache::lucene::util::automaton::RegExp;
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

void TestMockAnalyzer::testWhitespace() 
{
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  assertAnalyzesTo(a, L"A bc defg hiJklmn opqrstuv wxy z ",
                   std::deque<wstring>{L"a", L"bc", L"defg", L"hijklmn",
                                        L"opqrstuv", L"wxy", L"z"});
  assertAnalyzesTo(a, L"aba cadaba shazam",
                   std::deque<wstring>{L"aba", L"cadaba", L"shazam"});
  assertAnalyzesTo(a, L"break on whitespace",
                   std::deque<wstring>{L"break", L"on", L"whitespace"});
}

void TestMockAnalyzer::testSimple() 
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  assertAnalyzesTo(a, L"a-bc123 defg+hijklmn567opqrstuv78wxy_z ",
                   std::deque<wstring>{L"a", L"bc", L"defg", L"hijklmn",
                                        L"opqrstuv", L"wxy", L"z"});
  assertAnalyzesTo(a, L"aba4cadaba-Shazam",
                   std::deque<wstring>{L"aba", L"cadaba", L"shazam"});
  assertAnalyzesTo(a, L"break+on/Letters",
                   std::deque<wstring>{L"break", L"on", L"letters"});
}

void TestMockAnalyzer::testKeyword() 
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false);
  assertAnalyzesTo(
      a, L"a-bc123 defg+hijklmn567opqrstuv78wxy_z ",
      std::deque<wstring>{L"a-bc123 defg+hijklmn567opqrstuv78wxy_z "});
  assertAnalyzesTo(a, L"aba4cadaba-Shazam",
                   std::deque<wstring>{L"aba4cadaba-Shazam"});
  assertAnalyzesTo(a, L"break+on/Nothing",
                   std::deque<wstring>{L"break+on/Nothing"});
  // currently though emits no tokens for empty string: maybe we can do it,
  // but we don't want to emit tokens infinitely...
  assertAnalyzesTo(a, L"", std::deque<wstring>(0));
}

void TestMockAnalyzer::testSingleChar() 
{
  shared_ptr<CharacterRunAutomaton> single = make_shared<CharacterRunAutomaton>(
      (make_shared<RegExp>(L"."))->toAutomaton());
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random(), single, false);
  assertAnalyzesTo(
      a, L"foobar", std::deque<wstring>{L"f", L"o", L"o", L"b", L"a", L"r"},
      std::deque<int>{0, 1, 2, 3, 4, 5}, std::deque<int>{1, 2, 3, 4, 5, 6});
  checkRandomData(random(), a, 100);
}

void TestMockAnalyzer::testTwoChars() 
{
  shared_ptr<CharacterRunAutomaton> single = make_shared<CharacterRunAutomaton>(
      (make_shared<RegExp>(L".."))->toAutomaton());
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random(), single, false);
  assertAnalyzesTo(a, L"foobar", std::deque<wstring>{L"fo", L"ob", L"ar"},
                   std::deque<int>{0, 2, 4}, std::deque<int>{2, 4, 6});
  // make sure when last term is a "partial" match that end() is correct
  assertTokenStreamContents(a->tokenStream(L"bogus", L"fooba"),
                            std::deque<wstring>{L"fo", L"ob"},
                            std::deque<int>{0, 2}, std::deque<int>{2, 4},
                            std::deque<int>{1, 1}, optional<int>(5));
  checkRandomData(random(), a, 100);
}

void TestMockAnalyzer::testThreeChars() 
{
  shared_ptr<CharacterRunAutomaton> single = make_shared<CharacterRunAutomaton>(
      (make_shared<RegExp>(L"..."))->toAutomaton());
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random(), single, false);
  assertAnalyzesTo(a, L"foobar", std::deque<wstring>{L"foo", L"bar"},
                   std::deque<int>{0, 3}, std::deque<int>{3, 6});
  // make sure when last term is a "partial" match that end() is correct
  assertTokenStreamContents(a->tokenStream(L"bogus", L"fooba"),
                            std::deque<wstring>{L"foo"}, std::deque<int>{0},
                            std::deque<int>{3}, std::deque<int>{1},
                            optional<int>(5));
  checkRandomData(random(), a, 100);
}

void TestMockAnalyzer::testUppercase() 
{
  shared_ptr<CharacterRunAutomaton> single = make_shared<CharacterRunAutomaton>(
      (make_shared<RegExp>(L"[A-Z][a-z]*"))->toAutomaton());
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random(), single, false);
  assertAnalyzesTo(
      a, L"FooBarBAZ", std::deque<wstring>{L"Foo", L"Bar", L"B", L"A", L"Z"},
      std::deque<int>{0, 3, 6, 7, 8}, std::deque<int>{3, 6, 7, 8, 9});
  assertAnalyzesTo(a, L"aFooBar", std::deque<wstring>{L"Foo", L"Bar"},
                   std::deque<int>{1, 4}, std::deque<int>{4, 7});
  checkRandomData(random(), a, 100);
}

void TestMockAnalyzer::testStop() 
{
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(
      random(), MockTokenizer::SIMPLE, true, MockTokenFilter::ENGLISH_STOPSET);
  assertAnalyzesTo(a, L"the quick brown a fox",
                   std::deque<wstring>{L"quick", L"brown", L"fox"},
                   std::deque<int>{2, 1, 2});
}

void TestMockAnalyzer::testKeep() 
{
  shared_ptr<CharacterRunAutomaton> keepWords =
      make_shared<CharacterRunAutomaton>(Operations::complement(
          Operations::union_(Arrays::asList(Automata::makeString(L"foo"),
                                            Automata::makeString(L"bar"))),
          AutomatonTestUtil::DEFAULT_MAX_DETERMINIZED_STATES));
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(
      random(), MockTokenizer::SIMPLE, true, keepWords);
  assertAnalyzesTo(a, L"quick foo brown bar bar fox foo",
                   std::deque<wstring>{L"foo", L"bar", L"bar", L"foo"},
                   std::deque<int>{2, 2, 1, 2});
}

void TestMockAnalyzer::testLength() 
{
  shared_ptr<CharacterRunAutomaton> length5 =
      make_shared<CharacterRunAutomaton>(
          (make_shared<RegExp>(L".{5,}"))->toAutomaton());
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(
      random(), MockTokenizer::WHITESPACE, true, length5);
  assertAnalyzesTo(a, L"ok toolong fine notfine",
                   std::deque<wstring>{L"ok", L"fine"},
                   std::deque<int>{1, 2});
}

void TestMockAnalyzer::testTooLongToken() 
{
  shared_ptr<Analyzer> whitespace =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  assertTokenStreamContents(
      whitespace->tokenStream(L"bogus", L"test 123 toolong ok "),
      std::deque<wstring>{L"test", L"123", L"toolo", L"ng", L"ok"},
      std::deque<int>{0, 5, 9, 14, 17}, std::deque<int>{4, 8, 14, 16, 19},
      optional<int>(20));

  assertTokenStreamContents(
      whitespace->tokenStream(L"bogus", L"test 123 toolo"),
      std::deque<wstring>{L"test", L"123", L"toolo"},
      std::deque<int>{0, 5, 9}, std::deque<int>{4, 8, 14}, optional<int>(14));
}

TestMockAnalyzer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestMockAnalyzer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestMockAnalyzer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> t =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false, 5);
  return make_shared<TokenStreamComponents>(t, t);
}

void TestMockAnalyzer::testLUCENE_3042() 
{
  wstring testString = L"t";

  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (TokenStream stream =
  // analyzer.tokenStream("dummy", testString))
  {
    TokenStream stream = analyzer->tokenStream(L"dummy", testString);
    stream->reset();
    while (stream->incrementToken()) {
      // consume
    }
    stream->end();
  }

  assertAnalyzesTo(analyzer, testString, std::deque<wstring>{L"t"});
}

void TestMockAnalyzer::testRandomStrings() 
{
  checkRandomData(random(), make_shared<MockAnalyzer>(random()), atLeast(1000));
}

void TestMockAnalyzer::testRandomRegexps() 
{
  int iters = TEST_NIGHTLY ? atLeast(30) : atLeast(1);
  for (int i = 0; i < iters; i++) {
    shared_ptr<CharacterRunAutomaton> *const dfa =
        make_shared<CharacterRunAutomaton>(
            AutomatonTestUtil::randomAutomaton(random()),
            numeric_limits<int>::max());
    constexpr bool lowercase = random()->nextBoolean();
    constexpr int limit = TestUtil::nextInt(random(), 0, 500);
    shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass2>(
        shared_from_this(), dfa, lowercase, limit);
    checkRandomData(random(), a, 100);
    delete a;
  }
}

TestMockAnalyzer::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<TestMockAnalyzer> outerInstance,
    shared_ptr<CharacterRunAutomaton> dfa, bool lowercase, int limit)
{
  this->outerInstance = outerInstance;
  this->dfa = dfa;
  this->lowercase = lowercase;
  this->limit = limit;
}

shared_ptr<TokenStreamComponents>
TestMockAnalyzer::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> t = make_shared<MockTokenizer>(dfa, lowercase, limit);
  return make_shared<TokenStreamComponents>(t, t);
}

void TestMockAnalyzer::testForwardOffsets() 
{
  int num = atLeast(1000);
  for (int i = 0; i < num; i++) {
    wstring s = TestUtil::randomHtmlishString(random(), 20);
    shared_ptr<StringReader> reader = make_shared<StringReader>(s);
    shared_ptr<MockCharFilter> charfilter =
        make_shared<MockCharFilter>(reader, 2);
    shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (TokenStream ts =
    // analyzer.tokenStream("bogus", charfilter))
    {
      TokenStream ts = analyzer->tokenStream(L"bogus", charfilter);
      ts->reset();
      while (ts->incrementToken()) {
        ;
      }
      ts->end();
    }
  }
}

void TestMockAnalyzer::testWrapReader() 
{
  // LUCENE-5153: test that wrapping an analyzer's reader is allowed
  shared_ptr<Random> *const random = TestMockAnalyzer::random();

  shared_ptr<Analyzer> *const delegate_ = make_shared<MockAnalyzer>(random);
  shared_ptr<Analyzer> a = make_shared<AnalyzerWrapperAnonymousInnerClass>(
      shared_from_this(), delegate_->getReuseStrategy(), delegate_);

  checkOneTerm(a, L"abc", L"aabc");
}

TestMockAnalyzer::AnalyzerWrapperAnonymousInnerClass::
    AnalyzerWrapperAnonymousInnerClass(
        shared_ptr<TestMockAnalyzer> outerInstance,
        shared_ptr<org::apache::lucene::analysis::Analyzer::ReuseStrategy>
            getReuseStrategy,
        shared_ptr<org::apache::lucene::analysis::Analyzer> delegate_)
    : AnalyzerWrapper(getReuseStrategy)
{
  this->outerInstance = outerInstance;
  this->delegate_ = delegate_;
}

shared_ptr<Reader>
TestMockAnalyzer::AnalyzerWrapperAnonymousInnerClass::wrapReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<MockCharFilter>(reader, 7);
}

shared_ptr<Analyzer>
TestMockAnalyzer::AnalyzerWrapperAnonymousInnerClass::getWrappedAnalyzer(
    const wstring &fieldName)
{
  return delegate_;
}

void TestMockAnalyzer::testChangeGaps() 
{
  // LUCENE-5324: check that it is possible to change the wrapper's gaps
  constexpr int positionGap = random()->nextInt(1000);
  constexpr int offsetGap = random()->nextInt(1000);
  shared_ptr<Analyzer> *const delegate_ = make_shared<MockAnalyzer>(random());
  shared_ptr<Analyzer> *const a =
      make_shared<DelegatingAnalyzerWrapperAnonymousInnerClass>(
          shared_from_this(), delegate_->getReuseStrategy(), positionGap,
          offsetGap, delegate_);

  shared_ptr<RandomIndexWriter> *const writer =
      make_shared<RandomIndexWriter>(random(), newDirectory(), a);
  shared_ptr<Document> *const doc = make_shared<Document>();
  shared_ptr<FieldType> *const ft = make_shared<FieldType>();
  ft->setIndexOptions(IndexOptions::DOCS);
  ft->setTokenized(true);
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorPositions(true);
  ft->setStoreTermVectorOffsets(true);
  doc->push_back(make_shared<Field>(L"f", L"a", ft));
  doc->push_back(make_shared<Field>(L"f", L"a", ft));
  writer->addDocument(doc);
  shared_ptr<LeafReader> *const reader = getOnlyLeafReader(writer->getReader());
  shared_ptr<Fields> *const fields = reader->getTermVectors(0);
  shared_ptr<Terms> *const terms = fields->terms(L"f");
  shared_ptr<TermsEnum> *const te = terms->begin();
  TestUtil::assertEquals(make_shared<BytesRef>(L"a"), te->next());
  shared_ptr<PostingsEnum> *const dpe =
      te->postings(nullptr, PostingsEnum::ALL);
  TestUtil::assertEquals(0, dpe->nextDoc());
  TestUtil::assertEquals(2, dpe->freq());
  TestUtil::assertEquals(0, dpe->nextPosition());
  TestUtil::assertEquals(0, dpe->startOffset());
  constexpr int endOffset = dpe->endOffset();
  TestUtil::assertEquals(1 + positionGap, dpe->nextPosition());
  TestUtil::assertEquals(1 + endOffset + offsetGap, dpe->endOffset());
  TestUtil::assertEquals(nullptr, te->next());
  delete reader;
  delete writer;
  delete writer->w->getDirectory();
}

TestMockAnalyzer::DelegatingAnalyzerWrapperAnonymousInnerClass::
    DelegatingAnalyzerWrapperAnonymousInnerClass(
        shared_ptr<TestMockAnalyzer> outerInstance,
        shared_ptr<org::apache::lucene::analysis::Analyzer::ReuseStrategy>
            getReuseStrategy,
        int positionGap, int offsetGap,
        shared_ptr<org::apache::lucene::analysis::Analyzer> delegate_)
    : DelegatingAnalyzerWrapper(getReuseStrategy)
{
  this->outerInstance = outerInstance;
  this->positionGap = positionGap;
  this->offsetGap = offsetGap;
  this->delegate_ = delegate_;
}

shared_ptr<Analyzer>
TestMockAnalyzer::DelegatingAnalyzerWrapperAnonymousInnerClass::
    getWrappedAnalyzer(const wstring &fieldName)
{
  return delegate_;
}

int TestMockAnalyzer::DelegatingAnalyzerWrapperAnonymousInnerClass::
    getPositionIncrementGap(const wstring &fieldName)
{
  return positionGap;
}

int TestMockAnalyzer::DelegatingAnalyzerWrapperAnonymousInnerClass::
    getOffsetGap(const wstring &fieldName)
{
  return offsetGap;
}
} // namespace org::apache::lucene::analysis