using namespace std;

#include "TestSimpleTextStoredFieldsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"
#include "../../../../../../java/org/apache/lucene/codecs/simpletext/SimpleTextCodec.h"

namespace org::apache::lucene::codecs::simpletext
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseStoredFieldsFormatTestCase =
    org::apache::lucene::index::BaseStoredFieldsFormatTestCase;

shared_ptr<Codec> TestSimpleTextStoredFieldsFormat::getCodec()
{
  return make_shared<SimpleTextCodec>();
}
} // namespace org::apache::lucene::codecs::simpletext