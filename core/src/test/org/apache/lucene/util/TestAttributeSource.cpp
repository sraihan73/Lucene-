using namespace std;

#include "TestAttributeSource.h"

namespace org::apache::lucene::util
{
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using namespace org::apache::lucene::analysis::tokenattributes;

void TestAttributeSource::testCaptureState()
{
  // init a first instance
  shared_ptr<AttributeSource> src = make_shared<AttributeSource>();
  shared_ptr<CharTermAttribute> termAtt =
      src->addAttribute(CharTermAttribute::typeid);
  shared_ptr<TypeAttribute> typeAtt = src->addAttribute(TypeAttribute::typeid);
  termAtt->append(L"TestTerm");
  typeAtt->setType(L"TestType");
  constexpr int hashCode = src->hashCode();

  shared_ptr<AttributeSource::State> state = src->captureState();

  // modify the attributes
  termAtt->setEmpty()->append(L"AnotherTestTerm");
  typeAtt->setType(L"AnotherTestType");
  assertTrue(L"Hash code should be different", hashCode != src->hashCode());

  src->restoreState(state);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"TestTerm", termAtt->toString());
  assertEquals(L"TestType", typeAtt->type());
  assertEquals(L"Hash code should be equal after restore", hashCode,
               src->hashCode());

  // restore into an exact configured copy
  shared_ptr<AttributeSource> copy = make_shared<AttributeSource>();
  copy->addAttribute(CharTermAttribute::typeid);
  copy->addAttribute(TypeAttribute::typeid);
  copy->restoreState(state);
  assertEquals(L"Both AttributeSources should have same hashCode after restore",
               src->hashCode(), copy->hashCode());
  assertEquals(L"Both AttributeSources should be equal after restore", src,
               copy);

  // init a second instance (with attributes in different order and one
  // additional attribute)
  shared_ptr<AttributeSource> src2 = make_shared<AttributeSource>();
  typeAtt = src2->addAttribute(TypeAttribute::typeid);
  shared_ptr<FlagsAttribute> flagsAtt =
      src2->addAttribute(FlagsAttribute::typeid);
  termAtt = src2->addAttribute(CharTermAttribute::typeid);
  flagsAtt->setFlags(12345);

  src2->restoreState(state);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"TestTerm", termAtt->toString());
  assertEquals(L"TestType", typeAtt->type());
  assertEquals(L"FlagsAttribute should not be touched", 12345,
               flagsAtt->getFlags());

  // init a third instance missing one Attribute
  shared_ptr<AttributeSource> src3 = make_shared<AttributeSource>();
  termAtt = src3->addAttribute(CharTermAttribute::typeid);
  // The third instance is missing the TypeAttribute, so restoreState() should
  // throw IllegalArgumentException
  expectThrows(invalid_argument::typeid, [&]() { src3->restoreState(state); });
}

void TestAttributeSource::testCloneAttributes()
{
  shared_ptr<AttributeSource> *const src = make_shared<AttributeSource>();
  shared_ptr<FlagsAttribute> *const flagsAtt =
      src->addAttribute(FlagsAttribute::typeid);
  shared_ptr<TypeAttribute> *const typeAtt =
      src->addAttribute(TypeAttribute::typeid);
  flagsAtt->setFlags(1234);
  typeAtt->setType(L"TestType");

  shared_ptr<AttributeSource> *const clone = src->cloneAttributes();
  const Iterator<type_info> it = clone->getAttributeClassesIterator();
  assertEquals(L"FlagsAttribute must be the first attribute",
               FlagsAttribute::typeid, it->next());
  assertEquals(L"TypeAttribute must be the second attribute",
               TypeAttribute::typeid, it->next());
  assertFalse(L"No more attributes", it->hasNext());

  shared_ptr<FlagsAttribute> *const flagsAtt2 =
      clone->getAttribute(FlagsAttribute::typeid);
  assertNotNull(flagsAtt2);
  shared_ptr<TypeAttribute> *const typeAtt2 =
      clone->getAttribute(TypeAttribute::typeid);
  assertNotNull(typeAtt2);
  assertNotSame(
      L"FlagsAttribute of original and clone must be different instances",
      flagsAtt2, flagsAtt);
  assertNotSame(
      L"TypeAttribute of original and clone must be different instances",
      typeAtt2, typeAtt);
  assertEquals(L"FlagsAttribute of original and clone must be equal", flagsAtt2,
               flagsAtt);
  assertEquals(L"TypeAttribute of original and clone must be equal", typeAtt2,
               typeAtt);

  // test copy back
  flagsAtt2->setFlags(4711);
  typeAtt2->setType(L"OtherType");
  clone->copyTo(src);
  assertEquals(L"FlagsAttribute of original must now contain updated term",
               4711, flagsAtt->getFlags());
  assertEquals(L"TypeAttribute of original must now contain updated type",
               L"OtherType", typeAtt->type());
  // verify again:
  assertNotSame(
      L"FlagsAttribute of original and clone must be different instances",
      flagsAtt2, flagsAtt);
  assertNotSame(
      L"TypeAttribute of original and clone must be different instances",
      typeAtt2, typeAtt);
  assertEquals(L"FlagsAttribute of original and clone must be equal", flagsAtt2,
               flagsAtt);
  assertEquals(L"TypeAttribute of original and clone must be equal", typeAtt2,
               typeAtt);
}

void TestAttributeSource::testDefaultAttributeFactory() 
{
  shared_ptr<AttributeSource> src = make_shared<AttributeSource>();

  assertTrue(L"CharTermAttribute is not implemented by CharTermAttributeImpl",
             std::dynamic_pointer_cast<CharTermAttributeImpl>(
                 src->addAttribute(CharTermAttribute::typeid)) != nullptr);
  assertTrue(L"OffsetAttribute is not implemented by OffsetAttributeImpl",
             std::dynamic_pointer_cast<OffsetAttributeImpl>(
                 src->addAttribute(OffsetAttribute::typeid)) != nullptr);
  assertTrue(L"FlagsAttribute is not implemented by FlagsAttributeImpl",
             std::dynamic_pointer_cast<FlagsAttributeImpl>(
                 src->addAttribute(FlagsAttribute::typeid)) != nullptr);
  assertTrue(L"PayloadAttribute is not implemented by PayloadAttributeImpl",
             std::dynamic_pointer_cast<PayloadAttributeImpl>(
                 src->addAttribute(PayloadAttribute::typeid)) != nullptr);
  assertTrue(L"PositionIncrementAttribute is not implemented by "
             L"PositionIncrementAttributeImpl",
             std::dynamic_pointer_cast<PositionIncrementAttributeImpl>(
                 src->addAttribute(PositionIncrementAttribute::typeid)) !=
                 nullptr);
  assertTrue(L"TypeAttribute is not implemented by TypeAttributeImpl",
             std::dynamic_pointer_cast<TypeAttributeImpl>(
                 src->addAttribute(TypeAttribute::typeid)) != nullptr);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) public void
// testInvalidArguments() throws Exception
void TestAttributeSource::testInvalidArguments() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    shared_ptr<AttributeSource> src = make_shared<AttributeSource>();
    src->addAttribute(Token::typeid);
    fail(L"Should throw IllegalArgumentException");
  });

  expectThrows(invalid_argument::typeid, [&]() {
    shared_ptr<AttributeSource> src =
        make_shared<AttributeSource>(Token::TOKEN_ATTRIBUTE_FACTORY);
    src->addAttribute(Token::typeid);
  });

  expectThrows(invalid_argument::typeid, [&]() {
    shared_ptr<AttributeSource> src = make_shared<AttributeSource>();
    src->addAttribute(static_cast<type_info>(Iterator::typeid));
  });
}

void TestAttributeSource::testLUCENE_3042() 
{
  shared_ptr<AttributeSource> *const src1 = make_shared<AttributeSource>();
  src1->addAttribute(CharTermAttribute::typeid)->append(L"foo");
  int hash1 = src1->hashCode(); // this triggers a cached state
  shared_ptr<AttributeSource> *const src2 = make_shared<AttributeSource>(src1);
  src2->addAttribute(TypeAttribute::typeid)->setType(L"bar");
  assertTrue(L"The hashCode is identical, so the captured state was preserved.",
             hash1 != src1->hashCode());
  assertEquals(src2->hashCode(), src1->hashCode());
}

void TestAttributeSource::testClonePayloadAttribute() 
{
  // LUCENE-6055: verify that PayloadAttribute.clone() does deep cloning.
  shared_ptr<PayloadAttributeImpl> src = make_shared<PayloadAttributeImpl>(
      make_shared<BytesRef>(std::deque<char>{1, 2, 3}));

  // test clone()
  shared_ptr<PayloadAttributeImpl> clone = src->clone();
  clone->getPayload()->bytes[0] =
      10; // modify one byte, srcBytes shouldn't change
  assertEquals(L"clone() wasn't deep", 1, src->getPayload()->bytes[0]);

  // test copyTo()
  clone = make_shared<PayloadAttributeImpl>();
  src->copyTo(clone);
  clone->getPayload()->bytes[0] =
      10; // modify one byte, srcBytes shouldn't change
  assertEquals(L"clone() wasn't deep", 1, src->getPayload()->bytes[0]);
}

void TestAttributeSource::testRemoveAllAttributes()
{
  deque<type_info> attrClasses = deque<type_info>();
  attrClasses.push_back(CharTermAttribute::typeid);
  attrClasses.push_back(OffsetAttribute::typeid);
  attrClasses.push_back(FlagsAttribute::typeid);
  attrClasses.push_back(PayloadAttribute::typeid);
  attrClasses.push_back(PositionIncrementAttribute::typeid);
  attrClasses.push_back(TypeAttribute::typeid);

  // Add attributes with the default factory, then try to remove all of them
  shared_ptr<AttributeSource> defaultFactoryAttributeSource =
      make_shared<AttributeSource>();

  assertFalse(defaultFactoryAttributeSource->hasAttributes());

  for (auto attrClass : attrClasses) {
    defaultFactoryAttributeSource->addAttribute(attrClass);
    assertTrue(L"Missing added attribute " + attrClass.getSimpleName(),
               defaultFactoryAttributeSource->hasAttribute(attrClass));
  }

  defaultFactoryAttributeSource->removeAllAttributes();

  for (auto attrClass : attrClasses) {
    assertFalse(L"Didn't remove attribute " + attrClass.getSimpleName(),
                defaultFactoryAttributeSource->hasAttribute(attrClass));
  }
  assertFalse(defaultFactoryAttributeSource->hasAttributes());

  // Add attributes with the packed implementations factory, then try to remove
  // all of them
  shared_ptr<AttributeSource> packedImplsAttributeSource =
      make_shared<AttributeSource>(
          TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY);
  assertFalse(packedImplsAttributeSource->hasAttributes());

  for (auto attrClass : attrClasses) {
    packedImplsAttributeSource->addAttribute(attrClass);
    assertTrue(L"Missing added attribute " + attrClass.getSimpleName(),
               packedImplsAttributeSource->hasAttribute(attrClass));
  }

  packedImplsAttributeSource->removeAllAttributes();

  for (auto attrClass : attrClasses) {
    assertFalse(L"Didn't remove attribute " + attrClass.getSimpleName(),
                packedImplsAttributeSource->hasAttribute(attrClass));
  }
  assertFalse(packedImplsAttributeSource->hasAttributes());
}
} // namespace org::apache::lucene::util