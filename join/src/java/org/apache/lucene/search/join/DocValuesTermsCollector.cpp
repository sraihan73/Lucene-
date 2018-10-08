using namespace std;

#include "DocValuesTermsCollector.h"

namespace org::apache::lucene::search::join
{
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocValues = org::apache::lucene::index::DocValues;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
} // namespace org::apache::lucene::search::join