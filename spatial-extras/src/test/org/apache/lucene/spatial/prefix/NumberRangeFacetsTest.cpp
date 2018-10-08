using namespace std;

#include "NumberRangeFacetsTest.h"

namespace org::apache::lucene::spatial::prefix
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Query = org::apache::lucene::search::Query;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using TermInSetQuery = org::apache::lucene::search::TermInSetQuery;
using StrategyTestCase = org::apache::lucene::spatial::StrategyTestCase;
using Facets =
    org::apache::lucene::spatial::prefix::NumberRangePrefixTreeStrategy::Facets;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using CellIterator = org::apache::lucene::spatial::prefix::tree::CellIterator;
using DateRangePrefixTree =
    org::apache::lucene::spatial::prefix::tree::DateRangePrefixTree;
using NumberRangePrefixTree =
    org::apache::lucene::spatial::prefix::tree::NumberRangePrefixTree;
using UnitNRShape = org::apache::lucene::spatial::prefix::tree::
    NumberRangePrefixTree::UnitNRShape;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using org::junit::Before;
using org::junit::Test;
using org::locationtech::spatial4j::shape::Shape;
//    import static com.carrotsearch.randomizedtesting.RandomizedTest.randomInt;
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomIntBetween;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void setUp() throws Exception
void NumberRangeFacetsTest::setUp() 
{
  StrategyTestCase::setUp();
  tree =
      org::apache::lucene::spatial::prefix::tree->DateRangePrefixTree::INSTANCE;
  strategy = make_shared<NumberRangePrefixTreeStrategy>(tree, L"dateRange");
  shared_ptr<Calendar> tmpCal = tree->newCal();
  randomCalWindowField = randomIntBetween(
      1, Calendar::ZONE_OFFSET - 1); // we're not allowed to add zone offset
  tmpCal->add(randomCalWindowField, 2'000);
  randomCalWindowMs = max(2000LL, tmpCal->getTimeInMillis());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = 20) @Test public void test() throws
// java.io.IOException
void NumberRangeFacetsTest::test() 
{
  // generate test data
  deque<std::shared_ptr<Shape>> indexedShapes =
      deque<std::shared_ptr<Shape>>();
  constexpr int numIndexedShapes = random()->nextInt(15);
  for (int i = 0; i < numIndexedShapes; i++) {
    indexedShapes.push_back(randomShape());
  }

  // Main index loop:
  for (int i = 0; i < indexedShapes.size(); i++) {
    shared_ptr<Shape> shape = indexedShapes[i];
    adoc(L"" + to_wstring(i), shape);

    if (random()->nextInt(10) == 0) {
      commit(); // intermediate commit, produces extra segments
    }
  }

  // delete some documents randomly
  for (int id = 0; id < indexedShapes.size(); id++) {
    if (random()->nextInt(10) == 0) {
      deleteDoc(L"" + to_wstring(id));
      indexedShapes[id].reset();
    }
  }

  commit();

  // Main query loop:
  for (int queryIdx = 0; queryIdx < 10; queryIdx++) {
    preQueryHavoc();

    // We need to have a facet range window to do the facets between (a start
    // time & end time). We randomly pick a date, decide the level we want to
    // facet on, and then pick a right end time that is up to 2 thousand values
    // later.
    int calFieldFacet = randomCalWindowField - 1;
    if (calFieldFacet > 1 && rarely()) {
      calFieldFacet--;
    }
    shared_ptr<Calendar> *const leftCal = randomCalendar();
    leftCal->add(calFieldFacet, -1 * randomInt(1000));
    shared_ptr<Calendar> rightCal =
        std::static_pointer_cast<Calendar>(leftCal->clone());
    rightCal->add(calFieldFacet, randomInt(2000));
    // Pick facet detail level based on cal field.
    int detailLevel = tree->getTreeLevelForCalendarField(calFieldFacet);
    if (detailLevel < 0) { // no exact match
      detailLevel = -1 * detailLevel;
    }

    // Randomly pick a filter/acceptDocs
    shared_ptr<Bits> topAcceptDocs = nullptr;
    deque<int> acceptFieldIds = deque<int>();
    if (usually()) {
      // get all possible IDs into a deque, random shuffle it, then randomly
      // choose how many of the first we use to
      // replace the deque.
      for (int i = 0; i < indexedShapes.size(); i++) {
        if (indexedShapes[i] == nullptr) { // we deleted this one
          continue;
        }
        acceptFieldIds.push_back(i);
      }
      Collections::shuffle(acceptFieldIds, random());
      acceptFieldIds =
          acceptFieldIds.subList(0, randomInt(acceptFieldIds.size()));
      if (!acceptFieldIds.empty()) {
        deque<std::shared_ptr<BytesRef>> terms =
            deque<std::shared_ptr<BytesRef>>();
        for (shared_ptr<> : : optional<int> acceptDocId : acceptFieldIds) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          terms.push_back(make_shared<BytesRef>(acceptDocId->toString()));
        }

        topAcceptDocs =
            searchForDocBits(make_shared<TermInSetQuery>(L"id", terms));
      }
    }

    // Lets do it!
    shared_ptr<NumberRangePrefixTree::NRShape> facetRange =
        tree->toRangeShape(tree->toShape(leftCal), tree->toShape(rightCal));
    shared_ptr<Facets> facets =
        (std::static_pointer_cast<NumberRangePrefixTreeStrategy>(strategy))
            ->calcFacets(indexSearcher->getTopReaderContext(), topAcceptDocs,
                         facetRange, detailLevel);

    // System.out.println("Q: " + queryIdx + " " + facets);

    // Verify results. We do it by looping over indexed shapes and reducing the
    // facet counts.
    shared_ptr<Shape> facetShapeRounded = facetRange->roundToLevel(detailLevel);
    for (int indexedShapeId = 0; indexedShapeId < indexedShapes.size();
         indexedShapeId++) {
      if (topAcceptDocs != nullptr &&
          !find(acceptFieldIds.begin(), acceptFieldIds.end(), indexedShapeId) !=
              acceptFieldIds.end()) {
        continue; // this doc was filtered out via acceptDocs
      }
      shared_ptr<Shape> indexedShape = indexedShapes[indexedShapeId];
      if (indexedShape == nullptr) { // was deleted
        continue;
      }
      shared_ptr<Shape> indexedShapeRounded =
          (std::static_pointer_cast<NumberRangePrefixTree::NRShape>(
               indexedShape))
              ->roundToLevel(detailLevel);
      if (!indexedShapeRounded->relate(facetShapeRounded)
               .intersects()) { // no intersection at all
        continue;
      }
      // walk the cells
      shared_ptr<CellIterator> *const cellIterator =
          tree->getTreeCellIterator(indexedShape, detailLevel);
      while (cellIterator->hasNext()) {
        shared_ptr<Cell> cell = cellIterator->next();
        if (!cell->getShape()->relate(facetShapeRounded).intersects()) {
          cellIterator->remove(); // no intersection; prune
          continue;
        }
        assert(cell->getLevel() <= detailLevel);

        if (cell->getLevel() == detailLevel) {
          // count it
          shared_ptr<NumberRangePrefixTree::UnitNRShape> shape =
              std::static_pointer_cast<NumberRangePrefixTree::UnitNRShape>(
                  cell->getShape());
          shared_ptr<NumberRangePrefixTree::UnitNRShape> *const parentShape =
              shape->getShapeAtLevel(detailLevel - 1); // get parent
          shared_ptr<Facets::FacetParentVal> *const facetParentVal =
              facets->parents->get(parentShape);
          assertNotNull(facetParentVal);
          int index = shape->getValAtLevel(shape->getLevel());
          assertNotNull(facetParentVal->childCounts);
          assert(facetParentVal->childCounts[index] > 0);
          facetParentVal->childCounts[index]--;

        } else if (cell->isLeaf()) {
          // count it, and remove/prune.
          if (cell->getLevel() < detailLevel - 1) {
            assert(facets->topLeaves > 0);
            facets->topLeaves--;
          } else {
            shared_ptr<NumberRangePrefixTree::UnitNRShape> shape =
                std::static_pointer_cast<NumberRangePrefixTree::UnitNRShape>(
                    cell->getShape());
            shared_ptr<NumberRangePrefixTree::UnitNRShape> *const parentShape =
                shape->getShapeAtLevel(detailLevel - 1); // get parent
            shared_ptr<Facets::FacetParentVal> *const facetParentVal =
                facets->parents->get(parentShape);
            assertNotNull(facetParentVal);
            assert(facetParentVal->parentLeaves > 0);
            facetParentVal->parentLeaves--;
          }

          cellIterator->remove();
        }
        cellIterator++;
      }
    }
    // At this point; all counts should be down to zero.
    assertTrue(facets->topLeaves == 0);
    for (auto facetParentVal : facets->parents) {
      assertTrue(facetParentVal->second->parentLeaves == 0);
      if (facetParentVal->second.childCounts != nullptr) {
        for (int childCount : facetParentVal->second.childCounts) {
          assertTrue(childCount == 0);
        }
      }
    }
  }
}

shared_ptr<Bits> NumberRangeFacetsTest::searchForDocBits(
    shared_ptr<Query> query) 
{
  shared_ptr<FixedBitSet> bitSet =
      make_shared<FixedBitSet>(indexSearcher->getIndexReader()->maxDoc());
  indexSearcher->search(query, make_shared<SimpleCollectorAnonymousInnerClass>(
                                   shared_from_this(), bitSet));
  return bitSet;
}

NumberRangeFacetsTest::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(
        shared_ptr<NumberRangeFacetsTest> outerInstance,
        shared_ptr<FixedBitSet> bitSet)
{
  this->outerInstance = outerInstance;
  this->bitSet = bitSet;
}

void NumberRangeFacetsTest::SimpleCollectorAnonymousInnerClass::collect(
    int doc) 
{
  bitSet->set(leafDocBase + doc);
}

void NumberRangeFacetsTest::SimpleCollectorAnonymousInnerClass::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  leafDocBase = context->docBase;
}

bool NumberRangeFacetsTest::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return false;
}

void NumberRangeFacetsTest::preQueryHavoc()
{
  if (std::dynamic_pointer_cast<RecursivePrefixTreeStrategy>(strategy) !=
      nullptr) {
    shared_ptr<RecursivePrefixTreeStrategy> rpts =
        std::static_pointer_cast<RecursivePrefixTreeStrategy>(strategy);
    int scanLevel = randomInt(rpts->getGrid()->getMaxLevels());
    rpts->setPrefixGridScanLevel(scanLevel);
  }
}

shared_ptr<Shape> NumberRangeFacetsTest::randomShape()
{
  shared_ptr<Calendar> cal1 = randomCalendar();
  shared_ptr<NumberRangePrefixTree::UnitNRShape> s1 = tree->toShape(cal1);
  if (rarely()) {
    return s1;
  }
  try {
    shared_ptr<Calendar> cal2 = randomCalendar();
    shared_ptr<NumberRangePrefixTree::UnitNRShape> s2 = tree->toShape(cal2);
    if (cal1->compareTo(cal2) < 0) {
      return tree->toRangeShape(s1, s2);
    } else {
      return tree->toRangeShape(s2, s1);
    }
  } catch (const invalid_argument &e) {
    assert(e.what()->startsWith(L"Differing precision"));
    return s1;
  }
}

shared_ptr<Calendar> NumberRangeFacetsTest::randomCalendar()
{
  shared_ptr<Calendar> cal = tree->newCal();
  cal->setTimeInMillis(random()->nextLong() % randomCalWindowMs);
  try {
    tree->clearFieldsAfter(cal,
                           random()->nextInt(Calendar::FIELD_COUNT + 1) - 1);
  } catch (const AssertionError &e) {
    if (!e->getMessage().equals(L"Calendar underflow")) {
      throw e;
    }
  }
  return cal;
}
} // namespace org::apache::lucene::spatial::prefix