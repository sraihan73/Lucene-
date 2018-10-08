using namespace std;

#include "TestPackedTokenAttributeImpl.h"

namespace org::apache::lucene::analysis::tokenattributes
{
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestPackedTokenAttributeImpl::testClone() 
{
  shared_ptr<PackedTokenAttributeImpl> t =
      make_shared<PackedTokenAttributeImpl>();
  t->setOffset(0, 5);
  std::deque<wchar_t> content = (wstring(L"hello")).toCharArray();
  t->copyBuffer(content, 0, 5);
  std::deque<wchar_t> buf = t->buffer();
  shared_ptr<PackedTokenAttributeImpl> copy =
      TestCharTermAttributeImpl::assertCloneIsEqual(t);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(t->toString(), copy->toString());
  assertNotSame(buf, copy->buffer());
}

void TestPackedTokenAttributeImpl::testCopyTo() 
{
  shared_ptr<PackedTokenAttributeImpl> t =
      make_shared<PackedTokenAttributeImpl>();
  shared_ptr<PackedTokenAttributeImpl> copy =
      TestCharTermAttributeImpl::assertCopyIsEqual(t);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"", t->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"", copy->toString());

  t = make_shared<PackedTokenAttributeImpl>();
  t->setOffset(0, 5);
  std::deque<wchar_t> content = (wstring(L"hello")).toCharArray();
  t->copyBuffer(content, 0, 5);
  std::deque<wchar_t> buf = t->buffer();
  copy = TestCharTermAttributeImpl::assertCopyIsEqual(t);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(t->toString(), copy->toString());
  assertNotSame(buf, copy->buffer());
}

void TestPackedTokenAttributeImpl::testPackedTokenAttributeFactory() throw(
    runtime_error)
{
  shared_ptr<TokenStream> ts = make_shared<MockTokenizer>(
      TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY, MockTokenizer::WHITESPACE,
      false, MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"foo bar"));

  assertTrue(L"CharTermAttribute is not implemented by Token",
             std::dynamic_pointer_cast<PackedTokenAttributeImpl>(
                 ts->addAttribute(CharTermAttribute::typeid)) != nullptr);
  assertTrue(L"OffsetAttribute is not implemented by Token",
             std::dynamic_pointer_cast<PackedTokenAttributeImpl>(
                 ts->addAttribute(OffsetAttribute::typeid)) != nullptr);
  assertTrue(L"PositionIncrementAttribute is not implemented by Token",
             std::dynamic_pointer_cast<PackedTokenAttributeImpl>(
                 ts->addAttribute(PositionIncrementAttribute::typeid)) !=
                 nullptr);
  assertTrue(L"TypeAttribute is not implemented by Token",
             std::dynamic_pointer_cast<PackedTokenAttributeImpl>(
                 ts->addAttribute(TypeAttribute::typeid)) != nullptr);

  assertTrue(L"FlagsAttribute is not implemented by FlagsAttributeImpl",
             std::dynamic_pointer_cast<FlagsAttributeImpl>(
                 ts->addAttribute(FlagsAttribute::typeid)) != nullptr);
}

void TestPackedTokenAttributeImpl::testAttributeReflection() throw(
    runtime_error)
{
  shared_ptr<PackedTokenAttributeImpl> t =
      make_shared<PackedTokenAttributeImpl>();
  t->append(L"foobar");
  t->setOffset(6, 22);
  t->setPositionIncrement(3);
  t->setPositionLength(11);
  t->setType(L"foobar");
  t->setTermFrequency(42);
  TestUtil::assertAttributeReflection(
      t, make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
}

TestPackedTokenAttributeImpl::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestPackedTokenAttributeImpl> outerInstance)
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
  this->put(TypeAttribute::typeid->getName() + L"#type", L"foobar");
  this->put(TermFrequencyAttribute::typeid->getName() + L"#termFrequency", 42);
}
} // namespace org::apache::lucene::analysis::tokenattributes