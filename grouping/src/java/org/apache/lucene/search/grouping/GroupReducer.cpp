using namespace std;

#include "GroupReducer.h"

namespace org::apache::lucene::search::grouping
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Collector = org::apache::lucene::search::Collector;
using LeafCollector = org::apache::lucene::search::LeafCollector;
using Scorer = org::apache::lucene::search::Scorer;
} // namespace org::apache::lucene::search::grouping