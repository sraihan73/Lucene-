using namespace std;

#include "TestVarGapDocFreqIntervalPostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"

namespace org::apache::lucene::codecs::blockterms
{
using Codec = org::apache::lucene::codecs::Codec;
using LuceneVarGapFixedInterval =
    org::apache::lucene::codecs::blockterms::LuceneVarGapFixedInterval;
using BasePostingsFormatTestCase =
    org::apache::lucene::index::BasePostingsFormatTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Codec> TestVarGapDocFreqIntervalPostingsFormat::getCodec()
{
  return codec;
}
} // namespace org::apache::lucene::codecs::blockterms