using namespace std;

#include "WeightedFragListBuilderTest.h"

namespace org::apache::lucene::search::vectorhighlight
{
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;
using WeightedFragInfo = org::apache::lucene::search::vectorhighlight::
    FieldFragList::WeightedFragInfo;
using SubInfo = org::apache::lucene::search::vectorhighlight::FieldFragList::
    WeightedFragInfo::SubInfo;

void WeightedFragListBuilderTest::test2WeightedFragList() 
{
  testCase(pqF({L"the", L"both"}), 100,
           L"subInfos=(theboth((195,203)))/0.8679108(149,249)", 0.8679108);
}

void WeightedFragListBuilderTest::test2SubInfos() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(pqF({L"the", L"both"}), Occur::MUST);
  query->add(tq(L"examples"), Occur::MUST);

  testCase(query->build(), 1000,
           L"subInfos=(examples((19,27))examples((66,74))theboth((195,203)))/"
           L"1.8411169(0,1000)",
           1.8411169);
}

void WeightedFragListBuilderTest::testCase(
    shared_ptr<Query> query, int fragCharSize, const wstring &expectedFragInfo,
    double expectedTotalSubInfoBoost) 
{
  makeIndexLongMV();

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(query, true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  shared_ptr<WeightedFragListBuilder> wflb =
      make_shared<WeightedFragListBuilder>();
  shared_ptr<FieldFragList> ffl = wflb->createFieldFragList(fpl, fragCharSize);
  assertEquals(1, ffl->getFragInfos().size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(expectedFragInfo, ffl->getFragInfos()[0]->toString());

  float totalSubInfoBoost = 0;
  for (auto info : ffl->getFragInfos()) {
    for (auto subInfo : info->getSubInfos()) {
      totalSubInfoBoost += subInfo->getBoost();
    }
  }
  assertEquals(expectedTotalSubInfoBoost, totalSubInfoBoost, .0000001);
}
} // namespace org::apache::lucene::search::vectorhighlight