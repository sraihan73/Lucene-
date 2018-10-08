using namespace std;

#include "TestBytesRefAttImpl.h"

namespace org::apache::lucene::analysis::tokenattributes
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestBytesRefAttImpl::testCopyTo() 
{
  shared_ptr<BytesTermAttributeImpl> t = make_shared<BytesTermAttributeImpl>();
  shared_ptr<BytesTermAttributeImpl> copy = assertCopyIsEqual(t);

  // first do empty
  assertEquals(t->getBytesRef(), copy->getBytesRef());
  assertNull(copy->getBytesRef());
  // now after setting it
  t->setBytesRef(make_shared<BytesRef>(L"hello"));
  copy = assertCopyIsEqual(t);
  assertEquals(t->getBytesRef(), copy->getBytesRef());
  assertNotSame(t->getBytesRef(), copy->getBytesRef());
}

template <typename T>
T TestBytesRefAttImpl::assertCopyIsEqual(T att) 
{
  static_assert(is_base_of<org.apache.lucene.util.AttributeImpl, T>::value,
                L"T must inherit from org.apache.lucene.util.AttributeImpl");

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") T copy = (T)
  // att.getClass().newInstance();
  T copy = static_cast<T>(att->getClass().newInstance());
  att->copyTo(copy);
  assertEquals(L"Copied instance must be equal", att, copy);
  assertEquals(L"Copied instance's hashcode must be equal", att->hashCode(),
               copy->hashCode());
  return copy;
}
} // namespace org::apache::lucene::analysis::tokenattributes