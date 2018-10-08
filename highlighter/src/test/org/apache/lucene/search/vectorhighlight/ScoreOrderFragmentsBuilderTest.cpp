using namespace std;

#include "ScoreOrderFragmentsBuilderTest.h"

namespace org::apache::lucene::search::vectorhighlight
{
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;

void ScoreOrderFragmentsBuilderTest::test3Frags() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(F, L"a")),
             BooleanClause::Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(F, L"c")),
             BooleanClause::Occur::SHOULD);

  shared_ptr<FieldFragList> ffl = this->ffl(
      query->build(), L"a b b b b b b b b b b b a b a b b b b b c a a b b");
  shared_ptr<ScoreOrderFragmentsBuilder> sofb =
      make_shared<ScoreOrderFragmentsBuilder>();
  std::deque<wstring> f = sofb->createFragments(reader, 0, F, ffl, 3);
  assertEquals(3, f.size());
  // check score order
  assertEquals(L"<b>c</b> <b>a</b> <b>a</b> b b", f[0]);
  assertEquals(L"b b <b>a</b> b <b>a</b> b b b b b c", f[1]);
  assertEquals(L"<b>a</b> b b b b b b b b b b", f[2]);
}

shared_ptr<FieldFragList> ScoreOrderFragmentsBuilderTest::ffl(
    shared_ptr<Query> query, const wstring &indexValue) 
{
  make1d1fIndex(indexValue);
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(query, true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  return (make_shared<SimpleFragListBuilder>())->createFieldFragList(fpl, 20);
}
} // namespace org::apache::lucene::search::vectorhighlight