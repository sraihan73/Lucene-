using namespace std;

#include "TestLucene50CompoundFormat.h"

namespace org::apache::lucene::codecs::lucene50
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseCompoundFormatTestCase =
    org::apache::lucene::index::BaseCompoundFormatTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Codec> TestLucene50CompoundFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::lucene50