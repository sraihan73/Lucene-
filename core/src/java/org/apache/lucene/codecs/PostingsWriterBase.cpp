using namespace std;

#include "PostingsWriterBase.h"
#include "../index/FieldInfo.h"
#include "../index/SegmentWriteState.h"
#include "../index/TermsEnum.h"
#include "../store/DataOutput.h"
#include "../store/IndexOutput.h"
#include "../util/BytesRef.h"
#include "../util/FixedBitSet.h"
#include "BlockTermState.h"

namespace org::apache::lucene::codecs
{
using BlockTreeTermsWriter =
    org::apache::lucene::codecs::blocktree::BlockTreeTermsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

PostingsWriterBase::PostingsWriterBase() {}
} // namespace org::apache::lucene::codecs