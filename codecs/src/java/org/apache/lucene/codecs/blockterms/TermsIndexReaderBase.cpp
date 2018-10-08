using namespace std;

#include "TermsIndexReaderBase.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"

namespace org::apache::lucene::codecs::blockterms
{
using FieldInfo = org::apache::lucene::index::FieldInfo;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
} // namespace org::apache::lucene::codecs::blockterms