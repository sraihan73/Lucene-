using namespace std;

#include "SortedSetDocValuesReaderState.h"

namespace org::apache::lucene::facet::sortedset
{
using IndexReader = org::apache::lucene::index::IndexReader;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Accountable = org::apache::lucene::util::Accountable;

SortedSetDocValuesReaderState::OrdRange::OrdRange(int start, int end)
    : start(start), end(end)
{
}

SortedSetDocValuesReaderState::SortedSetDocValuesReaderState() {}
} // namespace org::apache::lucene::facet::sortedset