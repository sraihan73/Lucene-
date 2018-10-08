using namespace std;

#include "UnsupportedSpatialOperation.h"

namespace org::apache::lucene::spatial::query
{

UnsupportedSpatialOperation::UnsupportedSpatialOperation(
    shared_ptr<SpatialOperation> op)
    : UnsupportedOperationException(op->getName())
{
}
} // namespace org::apache::lucene::spatial::query