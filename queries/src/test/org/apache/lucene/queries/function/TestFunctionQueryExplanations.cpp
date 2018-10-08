using namespace std;

#include "TestFunctionQueryExplanations.h"

namespace org::apache::lucene::queries::function
{
using ConstValueSource =
    org::apache::lucene::queries::function::valuesource::ConstValueSource;
using RangeMapFloatFunction =
    org::apache::lucene::queries::function::valuesource::RangeMapFloatFunction;
using BaseExplanationTestCase =
    org::apache::lucene::search::BaseExplanationTestCase;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using Query = org::apache::lucene::search::Query;

void TestFunctionQueryExplanations::testSimple() 
{
  shared_ptr<Query> q =
      make_shared<FunctionQuery>(make_shared<ConstValueSource>(5));
  qtest(q, std::deque<int>{0, 1, 2, 3});
}

void TestFunctionQueryExplanations::testBoost() 
{
  shared_ptr<Query> q = make_shared<BoostQuery>(
      make_shared<FunctionQuery>(make_shared<ConstValueSource>(5)), 2);
  qtest(q, std::deque<int>{0, 1, 2, 3});
}

void TestFunctionQueryExplanations::testMapFunction() 
{
  shared_ptr<ValueSource> rff = make_shared<RangeMapFloatFunction>(
      make_shared<ConstValueSource>(3), 0, 1, 2, optional<float>(4));
  shared_ptr<Query> q = make_shared<FunctionQuery>(rff);
  qtest(q, std::deque<int>{0, 1, 2, 3});
  assertEquals(L"map_obj(const(3.0),0.0,1.0,const(2.0),const(4.0))",
               rff->description());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"map_obj(const(3.0),min=0.0,max=1.0,target=const(2.0),defaultVal="
               L"const(4.0))",
               rff->getValues(nullptr, nullptr)->toString(123));

  // DefaultValue is null -> defaults to source value
  rff = make_shared<RangeMapFloatFunction>(make_shared<ConstValueSource>(3), 0,
                                           1, 2, nullptr);
  assertEquals(L"map_obj(const(3.0),0.0,1.0,const(2.0),null)", rff->description());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"map_obj(const(3.0),min=0.0,max=1.0,target=const(2.0),defaultVal=null)",
      rff->getValues(nullptr, nullptr)->toString(123));
}
} // namespace org::apache::lucene::queries::function