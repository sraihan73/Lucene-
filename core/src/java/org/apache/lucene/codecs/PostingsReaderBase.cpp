using namespace std;

#include "PostingsReaderBase.h"
#include "../index/FieldInfo.h"
#include "../index/PostingsEnum.h"
#include "../index/SegmentReadState.h"
#include "../store/DataInput.h"
#include "../store/IndexInput.h"
#include "BlockTermState.h"

namespace org::apache::lucene::codecs
{
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using DataInput = org::apache::lucene::store::DataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;

PostingsReaderBase::PostingsReaderBase() {}
} // namespace org::apache::lucene::codecs