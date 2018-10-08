using namespace std;

#include "TermsIndexWriterBase.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/TermStats.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"

namespace org::apache::lucene::codecs::blockterms
{
using TermStats = org::apache::lucene::codecs::TermStats;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using BytesRef = org::apache::lucene::util::BytesRef;

TermsIndexWriterBase::FieldWriter::FieldWriter(
    shared_ptr<TermsIndexWriterBase> outerInstance)
    : outerInstance(outerInstance)
{
}
} // namespace org::apache::lucene::codecs::blockterms