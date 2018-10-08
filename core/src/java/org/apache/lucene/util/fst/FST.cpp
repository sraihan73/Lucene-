using namespace std;

#include "FST.h"

namespace org::apache::lucene::util::fst
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using InputStreamDataInput = org::apache::lucene::store::InputStreamDataInput;
using OutputStreamDataOutput =
    org::apache::lucene::store::OutputStreamDataOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using Accountable = org::apache::lucene::util::Accountable;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using Constants = org::apache::lucene::util::Constants;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
} // namespace org::apache::lucene::util::fst