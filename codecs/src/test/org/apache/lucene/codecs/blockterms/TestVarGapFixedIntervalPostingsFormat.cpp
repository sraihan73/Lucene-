using namespace std;

#include "TestVarGapFixedIntervalPostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"

namespace org::apache::lucene::codecs::blockterms
{
using Codec = org::apache::lucene::codecs::Codec;
using LuceneVarGapDocFreqInterval =
    org::apache::lucene::codecs::blockterms::LuceneVarGapDocFreqInterval;
using BasePostingsFormatTestCase =
    org::apache::lucene::index::BasePostingsFormatTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Codec> TestVarGapFixedIntervalPostingsFormat::getCodec()
{
  return codec;
}
} // namespace org::apache::lucene::codecs::blockterms