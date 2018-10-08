using namespace std;

#include "SimpleFragListBuilderTest.h"

namespace org::apache::lucene::search::vectorhighlight
{
using Term = org::apache::lucene::index::Term;
using namespace org::apache::lucene::search;

void SimpleFragListBuilderTest::testNullFieldFragList() 
{
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(
      fpl(make_shared<TermQuery>(make_shared<Term>(F, L"a")), L"b c d"), 100);
  assertEquals(0, ffl->getFragInfos().size());
}

void SimpleFragListBuilderTest::testTooSmallFragSize() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    shared_ptr<SimpleFragListBuilder> sflb =
        make_shared<SimpleFragListBuilder>();
    sflb->createFieldFragList(
        fpl(make_shared<TermQuery>(make_shared<Term>(F, L"a")), L"b c d"),
        sflb->minFragCharSize - 1);
  });
}

void SimpleFragListBuilderTest::testSmallerFragSizeThanTermQuery() throw(
    runtime_error)
{
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(
      fpl(make_shared<TermQuery>(make_shared<Term>(F, L"abcdefghijklmnopqrs")),
          L"abcdefghijklmnopqrs"),
      sflb->minFragCharSize);
  assertEquals(1, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(abcdefghijklmnopqrs((0,19)))/1.0(0,19)",
               ffl->getFragInfos()[0]->toString());
}

void SimpleFragListBuilderTest::testSmallerFragSizeThanPhraseQuery() throw(
    runtime_error)
{
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();

  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(F, L"abcdefgh", L"jklmnopqrs");

  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(
      fpl(phraseQuery, L"abcdefgh   jklmnopqrs"), sflb->minFragCharSize);
  assertEquals(1, ffl->getFragInfos().size());
  if (VERBOSE) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << ffl->getFragInfos()[0]->toString() << endl;
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(abcdefghjklmnopqrs((0,21)))/1.0(0,21)",
               ffl->getFragInfos()[0]->toString());
}

void SimpleFragListBuilderTest::test1TermIndex() 
{
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(
      fpl(make_shared<TermQuery>(make_shared<Term>(F, L"a")), L"a"), 100);
  assertEquals(1, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(a((0,1)))/1.0(0,100)",
               ffl->getFragInfos()[0]->toString());
}

void SimpleFragListBuilderTest::test2TermsIndex1Frag() 
{
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(
      fpl(make_shared<TermQuery>(make_shared<Term>(F, L"a")), L"a a"), 100);
  assertEquals(1, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(a((0,1))a((2,3)))/2.0(0,100)",
               ffl->getFragInfos()[0]->toString());

  ffl = sflb->createFieldFragList(
      fpl(make_shared<TermQuery>(make_shared<Term>(F, L"a")),
          L"a b b b b b b b b a"),
      20);
  assertEquals(1, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(a((0,1))a((18,19)))/2.0(0,20)",
               ffl->getFragInfos()[0]->toString());

  ffl = sflb->createFieldFragList(
      fpl(make_shared<TermQuery>(make_shared<Term>(F, L"a")),
          L"b b b b a b b b b a"),
      20);
  assertEquals(1, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(a((8,9))a((18,19)))/2.0(4,24)",
               ffl->getFragInfos()[0]->toString());
}

void SimpleFragListBuilderTest::test2TermsIndex2Frags() 
{
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(
      fpl(make_shared<TermQuery>(make_shared<Term>(F, L"a")),
          L"a b b b b b b b b b b b b b a"),
      20);
  assertEquals(2, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(a((0,1)))/1.0(0,20)",
               ffl->getFragInfos()[0]->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(a((28,29)))/1.0(20,40)",
               ffl->getFragInfos()[1]->toString());

  ffl = sflb->createFieldFragList(
      fpl(make_shared<TermQuery>(make_shared<Term>(F, L"a")),
          L"a b b b b b b b b b b b b a"),
      20);
  assertEquals(2, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(a((0,1)))/1.0(0,20)",
               ffl->getFragInfos()[0]->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(a((26,27)))/1.0(20,40)",
               ffl->getFragInfos()[1]->toString());

  ffl = sflb->createFieldFragList(
      fpl(make_shared<TermQuery>(make_shared<Term>(F, L"a")),
          L"a b b b b b b b b b a"),
      20);
  assertEquals(2, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(a((0,1)))/1.0(0,20)",
               ffl->getFragInfos()[0]->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(a((20,21)))/1.0(20,40)",
               ffl->getFragInfos()[1]->toString());
}

void SimpleFragListBuilderTest::test2TermsQuery() 
{
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();

  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(F, L"a")),
                    BooleanClause::Occur::SHOULD);
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(F, L"b")),
                    BooleanClause::Occur::SHOULD);

  shared_ptr<FieldFragList> ffl =
      sflb->createFieldFragList(fpl(booleanQuery->build(), L"c d e"), 20);
  assertEquals(0, ffl->getFragInfos().size());

  ffl = sflb->createFieldFragList(fpl(booleanQuery->build(), L"d b c"), 20);
  assertEquals(1, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(b((2,3)))/1.0(0,20)",
               ffl->getFragInfos()[0]->toString());

  ffl = sflb->createFieldFragList(fpl(booleanQuery->build(), L"a b c"), 20);
  assertEquals(1, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(a((0,1))b((2,3)))/2.0(0,20)",
               ffl->getFragInfos()[0]->toString());
}

void SimpleFragListBuilderTest::testPhraseQuery() 
{
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();

  shared_ptr<PhraseQuery> phraseQuery = make_shared<PhraseQuery>(F, L"a", L"b");

  shared_ptr<FieldFragList> ffl =
      sflb->createFieldFragList(fpl(phraseQuery, L"c d e"), 20);
  assertEquals(0, ffl->getFragInfos().size());

  ffl = sflb->createFieldFragList(fpl(phraseQuery, L"a c b"), 20);
  assertEquals(0, ffl->getFragInfos().size());

  ffl = sflb->createFieldFragList(fpl(phraseQuery, L"a b c"), 20);
  assertEquals(1, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(ab((0,3)))/1.0(0,20)",
               ffl->getFragInfos()[0]->toString());
}

void SimpleFragListBuilderTest::testPhraseQuerySlop() 
{
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();

  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(1, F, L"a", L"b");

  shared_ptr<FieldFragList> ffl =
      sflb->createFieldFragList(fpl(phraseQuery, L"a c b"), 20);
  assertEquals(1, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(ab((0,1)(4,5)))/1.0(0,20)",
               ffl->getFragInfos()[0]->toString());
}

shared_ptr<FieldPhraseList>
SimpleFragListBuilderTest::fpl(shared_ptr<Query> query,
                               const wstring &indexValue) 
{
  make1d1fIndex(indexValue);
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(query, true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  return make_shared<FieldPhraseList>(stack, fq);
}

void SimpleFragListBuilderTest::test1PhraseShortMV() 
{
  makeIndexShortMV();

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"d"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(fpl, 100);
  assertEquals(1, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(d((9,10)))/1.0(0,100)",
               ffl->getFragInfos()[0]->toString());
}

void SimpleFragListBuilderTest::test1PhraseLongMV() 
{
  makeIndexLongMV();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"search", L"engines"}), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(fpl, 100);
  assertEquals(1, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(searchengines((102,116))searchengines((157,171)))/"
               L"2.0(87,187)",
               ffl->getFragInfos()[0]->toString());
}

void SimpleFragListBuilderTest::test1PhraseLongMVB() 
{
  makeIndexLongMVB();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"sp", L"pe", L"ee", L"ed"}), true,
                              true); // "speed" -(2gram)-> "sp","pe","ee","ed"
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(fpl, 100);
  assertEquals(1, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(sppeeeed((88,93)))/1.0(41,141)",
               ffl->getFragInfos()[0]->toString());
}
} // namespace org::apache::lucene::search::vectorhighlight