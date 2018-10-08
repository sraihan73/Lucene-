using namespace std;

#include "AllGroupHeadsCollector.h"

namespace org::apache::lucene::search::grouping
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FieldComparator = org::apache::lucene::search::FieldComparator;
using LeafFieldComparator = org::apache::lucene::search::LeafFieldComparator;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
} // namespace org::apache::lucene::search::grouping