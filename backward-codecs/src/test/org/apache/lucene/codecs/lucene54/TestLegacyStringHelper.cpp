using namespace std;

#include "TestLegacyStringHelper.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../java/org/apache/lucene/codecs/lucene54/LegacyStringHelper.h"

namespace org::apache::lucene::codecs::lucene54
{
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestLegacyStringHelper::testBytesDifference()
{
  shared_ptr<BytesRef> left = make_shared<BytesRef>(L"foobar");
  shared_ptr<BytesRef> right = make_shared<BytesRef>(L"foozo");
  assertEquals(3, LegacyStringHelper::bytesDifference(left, right));
}

void TestLegacyStringHelper::testSortKeyLength() 
{
  assertEquals(
      3, LegacyStringHelper::sortKeyLength(make_shared<BytesRef>(L"foo"),
                                           make_shared<BytesRef>(L"for")));
  assertEquals(
      3, LegacyStringHelper::sortKeyLength(make_shared<BytesRef>(L"foo1234"),
                                           make_shared<BytesRef>(L"for1234")));
  assertEquals(2,
               LegacyStringHelper::sortKeyLength(make_shared<BytesRef>(L"foo"),
                                                 make_shared<BytesRef>(L"fz")));
  assertEquals(1,
               LegacyStringHelper::sortKeyLength(make_shared<BytesRef>(L"foo"),
                                                 make_shared<BytesRef>(L"g")));
  assertEquals(
      4, LegacyStringHelper::sortKeyLength(make_shared<BytesRef>(L"foo"),
                                           make_shared<BytesRef>(L"food")));
}
} // namespace org::apache::lucene::codecs::lucene54