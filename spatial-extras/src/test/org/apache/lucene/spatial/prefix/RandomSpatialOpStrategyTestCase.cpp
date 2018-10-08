using namespace std;

#include "RandomSpatialOpStrategyTestCase.h"

namespace org::apache::lucene::spatial::prefix
{
using org::locationtech::spatial4j::shape::Shape;
using Query = org::apache::lucene::search::Query;
using StrategyTestCase = org::apache::lucene::spatial::StrategyTestCase;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
//    import static com.carrotsearch.randomizedtesting.RandomizedTest.randomInt;
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomIntBetween;

void RandomSpatialOpStrategyTestCase::testOperationRandomShapes(
    shared_ptr<SpatialOperation> operation) 
{

  constexpr int numIndexedShapes = randomIntBetween(1, 6);
  deque<std::shared_ptr<Shape>> indexedShapes =
      deque<std::shared_ptr<Shape>>(numIndexedShapes);
  for (int i = 0; i < numIndexedShapes; i++) {
    indexedShapes.push_back(randomIndexedShape());
  }

  constexpr int numQueryShapes = atLeast(20);
  deque<std::shared_ptr<Shape>> queryShapes =
      deque<std::shared_ptr<Shape>>(numQueryShapes);
  for (int i = 0; i < numQueryShapes; i++) {
    queryShapes.push_back(randomQueryShape());
  }

  testOperation(operation, indexedShapes, queryShapes, true);
}

void RandomSpatialOpStrategyTestCase::testOperation(
    shared_ptr<SpatialOperation> operation,
    deque<std::shared_ptr<Shape>> &indexedShapes,
    deque<std::shared_ptr<Shape>> &queryShapes, bool havoc) 
{
  {
    // first show that when there's no data, a query will result in no results
    shared_ptr<Query> query = strategy->makeQuery(
        make_shared<SpatialArgs>(operation, randomQueryShape()));
    shared_ptr<SearchResults> searchResults = executeQuery(query, 1);
    assertEquals(0, searchResults->numFound);
  }

  // Main index loop:
  for (int i = 0; i < indexedShapes.size(); i++) {
    shared_ptr<Shape> shape = indexedShapes[i];
    adoc(L"" + to_wstring(i), shape);

    if (havoc && random()->nextInt(10) == 0) {
      commit(); // intermediate commit, produces extra segments
    }
  }
  if (havoc) {
    // delete some documents randomly
    for (int id = 0; id < indexedShapes.size(); id++) {
      if (random()->nextInt(10) == 0) {
        deleteDoc(L"" + to_wstring(id));
        indexedShapes[id].reset();
      }
    }
  }

  commit();

  // Main query loop:
  for (int queryIdx = 0; queryIdx < queryShapes.size(); queryIdx++) {
    shared_ptr<Shape> *const queryShape = queryShapes[queryIdx];

    if (havoc) {
      preQueryHavoc();
    }

    // Generate truth via brute force:
    // We ensure true-positive matches (if the predicate on the raw shapes match
    //  then the search should find those same matches).
    shared_ptr<Set<wstring>> expectedIds =
        make_shared<LinkedHashSet<wstring>>(); // true-positives
    for (int id = 0; id < indexedShapes.size(); id++) {
      shared_ptr<Shape> indexedShape = indexedShapes[id];
      if (indexedShape == nullptr) {
        continue;
      }
      if (operation->evaluate(indexedShape, queryShape)) {
        expectedIds->add(L"" + to_wstring(id));
      }
    }

    // Search and verify results
    shared_ptr<SpatialArgs> args =
        make_shared<SpatialArgs>(operation, queryShape);
    shared_ptr<Query> query = strategy->makeQuery(args);
    shared_ptr<SearchResults> got = executeQuery(query, 100);
    shared_ptr<Set<wstring>> remainingExpectedIds =
        make_shared<LinkedHashSet<wstring>>(expectedIds);
    for (auto result : got->results) {
      wstring id = result->getId();
      if (!remainingExpectedIds->remove(id)) {
        fail(L"qIdx:" + to_wstring(queryIdx) + L" Shouldn't match", id,
             indexedShapes, queryShape, operation);
      }
    }
    if (!remainingExpectedIds->isEmpty()) {
      wstring id = remainingExpectedIds->begin()->next();
      fail(L"qIdx:" + to_wstring(queryIdx) + L" Should have matched", id,
           indexedShapes, queryShape, operation);
    }
  }
}

void RandomSpatialOpStrategyTestCase::fail(
    const wstring &label, const wstring &id,
    deque<std::shared_ptr<Shape>> &indexedShapes, shared_ptr<Shape> queryShape,
    shared_ptr<SpatialOperation> operation)
{
  fail(L"[" + operation + L"] " + label + L" I#" + id + L":" +
       indexedShapes[stoi(id)] + L" Q:" + queryShape);
}

void RandomSpatialOpStrategyTestCase::preQueryHavoc()
{
  if (std::dynamic_pointer_cast<RecursivePrefixTreeStrategy>(strategy) !=
      nullptr) {
    shared_ptr<RecursivePrefixTreeStrategy> rpts =
        std::static_pointer_cast<RecursivePrefixTreeStrategy>(strategy);
    int scanLevel = randomInt(rpts->getGrid()->getMaxLevels());
    rpts->setPrefixGridScanLevel(scanLevel);
  }
}
} // namespace org::apache::lucene::spatial::prefix