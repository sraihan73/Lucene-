using namespace std;

#include "TestSimpleAttributeImpl.h"

namespace org::apache::lucene::analysis::tokenattributes
{
using TestUtil = org::apache::lucene::util::TestUtil;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSimpleAttributeImpl::testAttributes()
{
  TestUtil::assertAttributeReflection(
      make_shared<PositionIncrementAttributeImpl>(),
      Collections::singletonMap(PositionIncrementAttribute::typeid->getName() +
                                    L"#positionIncrement",
                                1));
  TestUtil::assertAttributeReflection(
      make_shared<PositionLengthAttributeImpl>(),
      Collections::singletonMap(
          PositionLengthAttribute::typeid->getName() + L"#positionLength", 1));
  TestUtil::assertAttributeReflection(
      make_shared<FlagsAttributeImpl>(),
      Collections::singletonMap(FlagsAttribute::typeid->getName() + L"#flags",
                                0));
  TestUtil::assertAttributeReflection(
      make_shared<TypeAttributeImpl>(),
      Collections::singletonMap(TypeAttribute::typeid->getName() + L"#type",
                                TypeAttribute::DEFAULT_TYPE));
  TestUtil::assertAttributeReflection(
      make_shared<PayloadAttributeImpl>(),
      Collections::singletonMap(
          PayloadAttribute::typeid->getName() + L"#payload", nullptr));
  TestUtil::assertAttributeReflection(
      make_shared<KeywordAttributeImpl>(),
      Collections::singletonMap(
          KeywordAttribute::typeid->getName() + L"#keyword", false));
  TestUtil::assertAttributeReflection(
      make_shared<OffsetAttributeImpl>(),
      make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
}

TestSimpleAttributeImpl::HashMapAnonymousInnerClass::HashMapAnonymousInnerClass(
    shared_ptr<TestSimpleAttributeImpl> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(OffsetAttribute::typeid->getName() + L"#startOffset", 0);
  this->put(OffsetAttribute::typeid->getName() + L"#endOffset", 0);
}
} // namespace org::apache::lucene::analysis::tokenattributes