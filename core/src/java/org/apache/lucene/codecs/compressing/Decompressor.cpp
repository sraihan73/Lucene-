using namespace std;

#include "Decompressor.h"
#include "../../store/DataInput.h"
#include "../../util/BytesRef.h"

namespace org::apache::lucene::codecs::compressing
{
using DataInput = org::apache::lucene::store::DataInput;
using BytesRef = org::apache::lucene::util::BytesRef;

Decompressor::Decompressor() {}
} // namespace org::apache::lucene::codecs::compressing