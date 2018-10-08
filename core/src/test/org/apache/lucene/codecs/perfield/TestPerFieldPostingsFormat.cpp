using namespace std;

#include "TestPerFieldPostingsFormat.h"

namespace org::apache::lucene::codecs::perfield
{
using Codec = org::apache::lucene::codecs::Codec;
using BasePostingsFormatTestCase =
    org::apache::lucene::index::BasePostingsFormatTestCase;
using RandomCodec = org::apache::lucene::index::RandomCodec;

shared_ptr<Codec> TestPerFieldPostingsFormat::getCodec()
{
  return make_shared<RandomCodec>(make_shared<Random>(random()->nextLong()),
                                  Collections::emptySet<wstring>());
}

void TestPerFieldPostingsFormat::testMergeStability() 
{
  assumeTrue(
      L"The MockRandom PF randomizes content on the fly, so we can't check it",
      false);
}

void TestPerFieldPostingsFormat::testPostingsEnumReuse() 
{
  assumeTrue(
      L"The MockRandom PF randomizes content on the fly, so we can't check it",
      false);
}
} // namespace org::apache::lucene::codecs::perfield