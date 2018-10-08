using namespace std;

#include "TestSimpleTextTermVectorsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"
#include "../../../../../../java/org/apache/lucene/codecs/simpletext/SimpleTextCodec.h"

namespace org::apache::lucene::codecs::simpletext
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseTermVectorsFormatTestCase =
    org::apache::lucene::index::BaseTermVectorsFormatTestCase;

shared_ptr<Codec> TestSimpleTextTermVectorsFormat::getCodec()
{
  return make_shared<SimpleTextCodec>();
}
} // namespace org::apache::lucene::codecs::simpletext