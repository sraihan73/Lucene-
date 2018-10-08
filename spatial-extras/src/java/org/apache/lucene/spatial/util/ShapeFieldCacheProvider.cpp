using namespace std;

#include "ShapeFieldCacheProvider.h"

namespace org::apache::lucene::spatial::util
{
using org::locationtech::spatial4j::shape::Shape;
using namespace org::apache::lucene::index;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using BytesRef = org::apache::lucene::util::BytesRef;
} // namespace org::apache::lucene::spatial::util