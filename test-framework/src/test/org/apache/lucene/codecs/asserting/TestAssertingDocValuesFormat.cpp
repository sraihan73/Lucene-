using namespace std;

#include "TestAssertingDocValuesFormat.h"

namespace org::apache::lucene::codecs::asserting
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseDocValuesFormatTestCase =
    org::apache::lucene::index::BaseDocValuesFormatTestCase;

shared_ptr<Codec> TestAssertingDocValuesFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::asserting