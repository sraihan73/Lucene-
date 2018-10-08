using namespace std;

#include "DistinctValuesCollector.h"

namespace org::apache::lucene::search::grouping
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
} // namespace org::apache::lucene::search::grouping