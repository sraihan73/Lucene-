using namespace std;

#include "Lucene53RWNormsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/NormsConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "Lucene53NormsConsumer.h"

namespace org::apache::lucene::codecs::lucene53
{
using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

shared_ptr<NormsConsumer> Lucene53RWNormsFormat::normsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<Lucene53NormsConsumer>(state, DATA_CODEC, DATA_EXTENSION,
                                            METADATA_CODEC, METADATA_EXTENSION);
}
} // namespace org::apache::lucene::codecs::lucene53