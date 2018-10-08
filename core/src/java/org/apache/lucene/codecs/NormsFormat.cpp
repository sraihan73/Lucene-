using namespace std;

#include "NormsFormat.h"
#include "../index/SegmentReadState.h"
#include "../index/SegmentWriteState.h"
#include "NormsConsumer.h"
#include "NormsProducer.h"

namespace org::apache::lucene::codecs
{
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

NormsFormat::NormsFormat() {}
} // namespace org::apache::lucene::codecs