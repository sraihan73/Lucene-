using namespace std;

#include "TestCachingTokenFilter.h"

namespace org::apache::lucene::analysis
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using Document = org::apache::lucene::document::Document;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiFields = org::apache::lucene::index::MultiFields;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;

void TestCachingTokenFilter::testCaching() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<AtomicInteger> resetCount = make_shared<AtomicInteger>(0);
  shared_ptr<TokenStream> stream = make_shared<TokenStreamAnonymousInnerClass>(
      shared_from_this(), resetCount);

  stream = make_shared<CachingTokenFilter>(stream);

  doc->push_back(make_shared<TextField>(L"preanalyzed", stream));

  // 1) we consume all tokens twice before we add the doc to the index
  assertFalse(
      (std::static_pointer_cast<CachingTokenFilter>(stream))->isCached());
  stream->reset();
  assertFalse(
      (std::static_pointer_cast<CachingTokenFilter>(stream))->isCached());
  checkTokens(stream);
  stream->reset();
  checkTokens(stream);
  assertTrue(
      (std::static_pointer_cast<CachingTokenFilter>(stream))->isCached());

  // 2) now add the document to the index and verify if all tokens are indexed
  //    don't reset the stream here, the DocumentWriter should do that
  //    implicitly
  writer->addDocument(doc);

  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<PostingsEnum> termPositions = MultiFields::getTermPositionsEnum(
      reader, L"preanalyzed", make_shared<BytesRef>(L"term1"));
  assertTrue(termPositions->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  assertEquals(1, termPositions->freq());
  assertEquals(0, termPositions->nextPosition());

  termPositions = MultiFields::getTermPositionsEnum(
      reader, L"preanalyzed", make_shared<BytesRef>(L"term2"));
  assertTrue(termPositions->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  assertEquals(2, termPositions->freq());
  assertEquals(1, termPositions->nextPosition());
  assertEquals(3, termPositions->nextPosition());

  termPositions = MultiFields::getTermPositionsEnum(
      reader, L"preanalyzed", make_shared<BytesRef>(L"term3"));
  assertTrue(termPositions->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  assertEquals(1, termPositions->freq());
  assertEquals(2, termPositions->nextPosition());
  delete reader;
  delete writer;
  // 3) reset stream and consume tokens again
  stream->reset();
  checkTokens(stream);

  assertEquals(1, resetCount->get());

  delete dir;
}

TestCachingTokenFilter::TokenStreamAnonymousInnerClass::
    TokenStreamAnonymousInnerClass(
        shared_ptr<TestCachingTokenFilter> outerInstance,
        shared_ptr<AtomicInteger> resetCount)
{
  this->outerInstance = outerInstance;
  this->resetCount = resetCount;
  index = 0;
  termAtt = addAttribute(CharTermAttribute::typeid);
  offsetAtt = addAttribute(OffsetAttribute::typeid);
}

void TestCachingTokenFilter::TokenStreamAnonymousInnerClass::reset() throw(
    IOException)
{
  outerInstance->super->reset();
  resetCount->incrementAndGet();
}

bool TestCachingTokenFilter::TokenStreamAnonymousInnerClass::incrementToken()
{
  if (index == outerInstance->tokens.size()) {
    return false;
  } else {
    clearAttributes();
    termAtt->append(outerInstance->tokens[index++]);
    offsetAtt::setOffset(0, 0);
    return true;
  }
}

void TestCachingTokenFilter::testDoubleResetFails() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<TokenStream> *const input =
      analyzer->tokenStream(L"field", L"abc");
  shared_ptr<CachingTokenFilter> buffer =
      make_shared<CachingTokenFilter>(input);
  buffer->reset(); // ok
  shared_ptr<IllegalStateException> e =
      expectThrows(IllegalStateException::typeid, [&]() { buffer->reset(); });
  assertEquals(L"double reset()", e->getMessage());
}

void TestCachingTokenFilter::checkTokens(shared_ptr<TokenStream> stream) throw(
    IOException)
{
  int count = 0;

  shared_ptr<CharTermAttribute> termAtt =
      stream->getAttribute(CharTermAttribute::typeid);
  while (stream->incrementToken()) {
    assertTrue(count < tokens.size());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(tokens[count], termAtt->toString());
    count++;
  }

  assertEquals(tokens.size(), count);
}
} // namespace org::apache::lucene::analysis