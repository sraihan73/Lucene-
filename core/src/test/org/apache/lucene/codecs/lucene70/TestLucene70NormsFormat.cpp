using namespace std;

#include "TestLucene70NormsFormat.h"

namespace org::apache::lucene::codecs::lucene70
{
using Codec = org::apache::lucene::codecs::Codec;
using Lucene70Codec = org::apache::lucene::codecs::lucene70::Lucene70Codec;
using BaseNormsFormatTestCase =
    org::apache::lucene::index::BaseNormsFormatTestCase;

shared_ptr<Codec> TestLucene70NormsFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::lucene70