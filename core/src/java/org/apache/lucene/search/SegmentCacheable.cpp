using namespace std;

#include "SegmentCacheable.h"

namespace org::apache::lucene::search
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
} // namespace org::apache::lucene::search