using namespace std;

#include "SpatialMatchConcern.h"

namespace org::apache::lucene::spatial
{

SpatialMatchConcern::SpatialMatchConcern(bool order, bool superset)
    : orderIsImportant(order), resultsAreSuperset(superset)
{
}

const shared_ptr<SpatialMatchConcern> SpatialMatchConcern::EXACT =
    make_shared<SpatialMatchConcern>(true, false);
const shared_ptr<SpatialMatchConcern> SpatialMatchConcern::FILTER =
    make_shared<SpatialMatchConcern>(false, false);
const shared_ptr<SpatialMatchConcern> SpatialMatchConcern::SUPERSET =
    make_shared<SpatialMatchConcern>(false, true);
} // namespace org::apache::lucene::spatial