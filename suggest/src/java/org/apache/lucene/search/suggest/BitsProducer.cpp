using namespace std;

#include "BitsProducer.h"

namespace org::apache::lucene::search::suggest
{
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Bits = org::apache::lucene::util::Bits;

BitsProducer::BitsProducer() {}
} // namespace org::apache::lucene::search::suggest