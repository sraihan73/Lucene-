using namespace std;

#include "RandomSpatialOpFuzzyPrefixTree50Test.h"

namespace org::apache::lucene::spatial::prefix
{

shared_ptr<RecursivePrefixTreeStrategy>
RandomSpatialOpFuzzyPrefixTree50Test::newRPT()
{
  return make_shared<RecursivePrefixTreeStrategyAnonymousInnerClass>(
      shared_from_this(), this->grid, getClass().getSimpleName());
}

RandomSpatialOpFuzzyPrefixTree50Test::
    RecursivePrefixTreeStrategyAnonymousInnerClass::
        RecursivePrefixTreeStrategyAnonymousInnerClass(
            shared_ptr<RandomSpatialOpFuzzyPrefixTree50Test> outerInstance,
            shared_ptr<
                org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree>
                grid,
            shared_ptr<UnknownType> getSimpleName)
    : RecursivePrefixTreeStrategy(grid, getSimpleName)
{
  this->outerInstance = outerInstance;
}

shared_ptr<CellToBytesRefIterator> RandomSpatialOpFuzzyPrefixTree50Test::
    RecursivePrefixTreeStrategyAnonymousInnerClass::newCellToBytesRefIterator()
{
  return make_shared<CellToBytesRefIterator50>();
}
} // namespace org::apache::lucene::spatial::prefix