using namespace std;

#include "SingleFragListBuilderTest.h"

namespace org::apache::lucene::search::vectorhighlight
{
using Term = org::apache::lucene::index::Term;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;

void SingleFragListBuilderTest::testNullFieldFragList() 
{
  shared_ptr<SingleFragListBuilder> sflb = make_shared<SingleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(
      fpl(make_shared<TermQuery>(make_shared<Term>(F, L"a")), {L"b c d"}), 100);
  assertEquals(0, ffl->getFragInfos().size());
}

void SingleFragListBuilderTest::testShortFieldFragList() 
{
  shared_ptr<SingleFragListBuilder> sflb = make_shared<SingleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(
      fpl(make_shared<TermQuery>(make_shared<Term>(F, L"a")), {L"a b c d"}),
      100);
  assertEquals(1, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(a((0,1)))/1.0(0,2147483647)",
               ffl->getFragInfos()[0]->toString());
}

void SingleFragListBuilderTest::testLongFieldFragList() 
{
  shared_ptr<SingleFragListBuilder> sflb = make_shared<SingleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(
      fpl(make_shared<TermQuery>(make_shared<Term>(F, L"a")),
          {L"a b c d", L"a b c d e f g h i",
           L"j k l m n o p q r s t u v w x y z a b c", L"d e f g"}),
      100);
  assertEquals(1, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"subInfos=(a((0,1))a((8,9))a((60,61)))/3.0(0,2147483647)",
               ffl->getFragInfos()[0]->toString());
}

shared_ptr<FieldPhraseList> SingleFragListBuilderTest::fpl(
    shared_ptr<Query> query, deque<wstring> &indexValues) 
{
  make1dmfIndex(indexValues);
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(query, true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  return make_shared<FieldPhraseList>(stack, fq);
}
} // namespace org::apache::lucene::search::vectorhighlight