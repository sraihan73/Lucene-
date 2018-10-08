using namespace std;

#include "DocValuesWriter.h"

namespace org::apache::lucene::index
{
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using SortField = org::apache::lucene::search::SortField;
} // namespace org::apache::lucene::index