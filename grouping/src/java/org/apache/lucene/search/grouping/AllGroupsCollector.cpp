using namespace std;

#include "AllGroupsCollector.h"

namespace org::apache::lucene::search::grouping
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
} // namespace org::apache::lucene::search::grouping