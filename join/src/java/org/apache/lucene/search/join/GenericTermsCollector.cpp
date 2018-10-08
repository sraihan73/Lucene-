using namespace std;

#include "GenericTermsCollector.h"

namespace org::apache::lucene::search::join
{
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Collector = org::apache::lucene::search::Collector;
using LeafCollector = org::apache::lucene::search::LeafCollector;
using org::apache::lucene::search::join::DocValuesTermsCollector::Function;
using MV = org::apache::lucene::search::join::TermsWithScoreCollector::MV;
using SV = org::apache::lucene::search::join::TermsWithScoreCollector::SV;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
} // namespace org::apache::lucene::search::join