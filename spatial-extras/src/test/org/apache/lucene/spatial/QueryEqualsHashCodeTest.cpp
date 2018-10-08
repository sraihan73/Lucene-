using namespace std;

#include "QueryEqualsHashCodeTest.h"

namespace org::apache::lucene::spatial
{
using BBoxStrategy = org::apache::lucene::spatial::bbox::BBoxStrategy;
using CompositeSpatialStrategy =
    org::apache::lucene::spatial::composite::CompositeSpatialStrategy;
using RecursivePrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::RecursivePrefixTreeStrategy;
using TermQueryPrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::TermQueryPrefixTreeStrategy;
using GeohashPrefixTree =
    org::apache::lucene::spatial::prefix::tree::GeohashPrefixTree;
using QuadPrefixTree =
    org::apache::lucene::spatial::prefix::tree::QuadPrefixTree;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using SerializedDVStrategy =
    org::apache::lucene::spatial::serialized::SerializedDVStrategy;
using PointVectorStrategy =
    org::apache::lucene::spatial::deque::PointVectorStrategy;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Shape;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEqualsHashCode()
void QueryEqualsHashCodeTest::testEqualsHashCode()
{

  switch (random()->nextInt(4)) { // 0-3
  case 0:
    predicate = SpatialOperation::Contains;
    break;
  case 1:
    predicate = SpatialOperation::IsWithin;
    break;

  default:
    predicate = SpatialOperation::Intersects;
    break;
  }
  shared_ptr<SpatialPrefixTree> *const gridQuad =
      make_shared<QuadPrefixTree>(ctx, 10);
  shared_ptr<SpatialPrefixTree> *const gridGeohash =
      make_shared<GeohashPrefixTree>(ctx, 10);

  shared_ptr<deque<std::shared_ptr<SpatialStrategy>>> strategies =
      deque<std::shared_ptr<SpatialStrategy>>();
  shared_ptr<RecursivePrefixTreeStrategy> recursive_geohash =
      make_shared<RecursivePrefixTreeStrategy>(gridGeohash,
                                               L"recursive_geohash");
  strategies->add(recursive_geohash);
  strategies->add(
      make_shared<TermQueryPrefixTreeStrategy>(gridQuad, L"termquery_quad"));
  strategies->add(PointVectorStrategy::newInstance(ctx, L"pointvector"));
  strategies->add(BBoxStrategy::newInstance(ctx, L"bbox"));
  shared_ptr<SerializedDVStrategy> *const serialized =
      make_shared<SerializedDVStrategy>(ctx, L"serialized");
  strategies->add(serialized);
  strategies->add(make_shared<CompositeSpatialStrategy>(
      L"composite", recursive_geohash, serialized));
  for (auto strategy : strategies) {
    testEqualsHashcode(strategy);
  }
}

void QueryEqualsHashCodeTest::testEqualsHashcode(
    shared_ptr<SpatialStrategy> strategy)
{
  shared_ptr<SpatialArgs> *const args1 = makeArgs1();
  shared_ptr<SpatialArgs> *const args2 = makeArgs2();
  testEqualsHashcode(args1, args2,
                     make_shared<ObjGeneratorAnonymousInnerClass>(
                         shared_from_this(), strategy));
  testEqualsHashcode(args1, args2,
                     make_shared<ObjGeneratorAnonymousInnerClass2>(
                         shared_from_this(), strategy));
}

QueryEqualsHashCodeTest::ObjGeneratorAnonymousInnerClass::
    ObjGeneratorAnonymousInnerClass(
        shared_ptr<QueryEqualsHashCodeTest> outerInstance,
        shared_ptr<org::apache::lucene::spatial::SpatialStrategy> strategy)
{
  this->outerInstance = outerInstance;
  this->strategy = strategy;
}

any QueryEqualsHashCodeTest::ObjGeneratorAnonymousInnerClass::gen(
    shared_ptr<SpatialArgs> args)
{
  return strategy->makeQuery(args);
}

QueryEqualsHashCodeTest::ObjGeneratorAnonymousInnerClass2::
    ObjGeneratorAnonymousInnerClass2(
        shared_ptr<QueryEqualsHashCodeTest> outerInstance,
        shared_ptr<org::apache::lucene::spatial::SpatialStrategy> strategy)
{
  this->outerInstance = outerInstance;
  this->strategy = strategy;
}

any QueryEqualsHashCodeTest::ObjGeneratorAnonymousInnerClass2::gen(
    shared_ptr<SpatialArgs> args)
{
  return strategy->makeDistanceValueSource(args->getShape()->getCenter());
}

void QueryEqualsHashCodeTest::testEqualsHashcode(
    shared_ptr<SpatialArgs> args1, shared_ptr<SpatialArgs> args2,
    shared_ptr<ObjGenerator> generator)
{
  any first;
  try {
    first = generator->gen(args1);
  } catch (const UnsupportedOperationException &e) {
    return;
  }
  if (first == nullptr) {
    return; // unsupported op?
  }
  any second = generator->gen(args1); // should be the same
  assertEquals(first, second);
  assertEquals(first.hashCode(), second.hashCode());
  assertTrue(args1->equals(args2) == false);
  second = generator->gen(args2); // now should be different
  assertTrue(first.equals(second) == false);
  assertTrue(first.hashCode() != second.hashCode());
}

shared_ptr<SpatialArgs> QueryEqualsHashCodeTest::makeArgs1()
{
  shared_ptr<Shape> *const shape1 = ctx->makeRectangle(0, 0, 10, 10);
  return make_shared<SpatialArgs>(predicate, shape1);
}

shared_ptr<SpatialArgs> QueryEqualsHashCodeTest::makeArgs2()
{
  shared_ptr<Shape> *const shape2 = ctx->makeRectangle(0, 0, 20, 20);
  return make_shared<SpatialArgs>(predicate, shape2);
}
} // namespace org::apache::lucene::spatial