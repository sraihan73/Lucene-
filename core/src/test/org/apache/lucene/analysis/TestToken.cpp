using namespace std;

#include "TestToken.h"

namespace org::apache::lucene::analysis
{
using namespace org::apache::lucene::analysis::tokenattributes;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Attribute = org::apache::lucene::util::Attribute;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestToken::testCtor() 
{
  shared_ptr<Token> t = make_shared<Token>(L"hello", 0, 0);
  TestUtil::assertEquals(0, t->startOffset());
  TestUtil::assertEquals(0, t->endOffset());
  TestUtil::assertEquals(1, t->getPositionIncrement());
  TestUtil::assertEquals(1, t->getPositionLength());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"hello", t->toString());
  TestUtil::assertEquals(L"word", t->type());
  TestUtil::assertEquals(0, t->getFlags());
  assertNull(t->getPayload());
}

void TestToken::testClone() 
{
  shared_ptr<Token> t = make_shared<Token>();
  t->setOffset(0, 5);
  std::deque<wchar_t> content = (wstring(L"hello")).toCharArray();
  t->copyBuffer(content, 0, 5);
  std::deque<wchar_t> buf = t->buffer();
  shared_ptr<Token> copy = TestCharTermAttributeImpl::assertCloneIsEqual(t);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(t->toString(), copy->toString());
  assertNotSame(buf, copy->buffer());

  shared_ptr<BytesRef> pl =
      make_shared<BytesRef>(std::deque<char>{1, 2, 3, 4});
  t->setPayload(pl);
  copy = TestCharTermAttributeImpl::assertCloneIsEqual(t);
  TestUtil::assertEquals(pl, copy->getPayload());
  assertNotSame(pl, copy->getPayload());
}

void TestToken::testCopyTo() 
{
  shared_ptr<Token> t = make_shared<Token>();
  shared_ptr<Token> copy = TestCharTermAttributeImpl::assertCopyIsEqual(t);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"", t->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"", copy->toString());

  t = make_shared<Token>();
  t->setOffset(0, 5);
  std::deque<wchar_t> content = (wstring(L"hello")).toCharArray();
  t->copyBuffer(content, 0, 5);
  std::deque<wchar_t> buf = t->buffer();
  copy = TestCharTermAttributeImpl::assertCopyIsEqual(t);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(t->toString(), copy->toString());
  assertNotSame(buf, copy->buffer());

  shared_ptr<BytesRef> pl =
      make_shared<BytesRef>(std::deque<char>{1, 2, 3, 4});
  t->setPayload(pl);
  copy = TestCharTermAttributeImpl::assertCopyIsEqual(t);
  TestUtil::assertEquals(pl, copy->getPayload());
  assertNotSame(pl, copy->getPayload());
}

void TestToken::SenselessAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
}

void TestToken::SenselessAttributeImpl::clear() {}

bool TestToken::SenselessAttributeImpl::equals(any o)
{
  return (std::dynamic_pointer_cast<SenselessAttributeImpl>(o) != nullptr);
}

int TestToken::SenselessAttributeImpl::hashCode() { return 0; }

void TestToken::SenselessAttributeImpl::reflectWith(
    AttributeReflector reflector)
{
}

void TestToken::testTokenAttributeFactory() 
{
  shared_ptr<TokenStream> ts = make_shared<MockTokenizer>(
      Token::TOKEN_ATTRIBUTE_FACTORY, MockTokenizer::WHITESPACE, false,
      MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"foo bar"));

  assertTrue(L"SenselessAttribute is not implemented by SenselessAttributeImpl",
             std::dynamic_pointer_cast<SenselessAttributeImpl>(
                 ts->addAttribute(SenselessAttribute::typeid)) != nullptr);

  assertTrue(L"CharTermAttribute is not implemented by Token",
             std::dynamic_pointer_cast<Token>(
                 ts->addAttribute(CharTermAttribute::typeid)) != nullptr);
  assertTrue(L"OffsetAttribute is not implemented by Token",
             std::dynamic_pointer_cast<Token>(
                 ts->addAttribute(OffsetAttribute::typeid)) != nullptr);
  assertTrue(L"FlagsAttribute is not implemented by Token",
             std::dynamic_pointer_cast<Token>(
                 ts->addAttribute(FlagsAttribute::typeid)) != nullptr);
  assertTrue(L"PayloadAttribute is not implemented by Token",
             std::dynamic_pointer_cast<Token>(
                 ts->addAttribute(PayloadAttribute::typeid)) != nullptr);
  assertTrue(L"PositionIncrementAttribute is not implemented by Token",
             std::dynamic_pointer_cast<Token>(ts->addAttribute(
                 PositionIncrementAttribute::typeid)) != nullptr);
  assertTrue(L"TypeAttribute is not implemented by Token",
             std::dynamic_pointer_cast<Token>(
                 ts->addAttribute(TypeAttribute::typeid)) != nullptr);
}

void TestToken::testAttributeReflection() 
{
  shared_ptr<Token> t = make_shared<Token>(L"foobar", 6, 22);
  t->setFlags(8);
  t->setPositionIncrement(3);
  t->setPositionLength(11);
  t->setTermFrequency(42);
  TestUtil::assertAttributeReflection(
      t, make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
}

TestToken::HashMapAnonymousInnerClass::HashMapAnonymousInnerClass(
    shared_ptr<TestToken> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(CharTermAttribute::typeid->getName() + L"#term", L"foobar");
  this->put(TermToBytesRefAttribute::typeid->getName() + L"#bytes",
            make_shared<BytesRef>(L"foobar"));
  this->put(OffsetAttribute::typeid->getName() + L"#startOffset", 6);
  this->put(OffsetAttribute::typeid->getName() + L"#endOffset", 22);
  this->put(
      PositionIncrementAttribute::typeid->getName() + L"#positionIncrement", 3);
  this->put(PositionLengthAttribute::typeid->getName() + L"#positionLength",
            11);
  this->put(PayloadAttribute::typeid->getName() + L"#payload", nullptr);
  this->put(TypeAttribute::typeid->getName() + L"#type",
            TypeAttribute::DEFAULT_TYPE);
  this->put(FlagsAttribute::typeid->getName() + L"#flags", 8);
  this->put(TermFrequencyAttribute::typeid->getName() + L"#termFrequency", 42);
}
} // namespace org::apache::lucene::analysis