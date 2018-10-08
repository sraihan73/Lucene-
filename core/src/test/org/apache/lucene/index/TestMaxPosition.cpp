using namespace std;

#include "TestMaxPosition.h"

namespace org::apache::lucene::index
{
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using Token = org::apache::lucene::analysis::Token;
using Document = org::apache::lucene::document::Document;
using TextField = org::apache::lucene::document::TextField;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestMaxPosition::testTooBigPosition() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  // This is at position 1:
  shared_ptr<Token> t1 = make_shared<Token>(L"foo", 0, 3);
  t1->setPositionIncrement(2);
  if (random()->nextBoolean()) {
    t1->setPayload(make_shared<BytesRef>(std::deque<char>{0x1}));
  }
  shared_ptr<Token> t2 = make_shared<Token>(L"foo", 4, 7);
  // This should overflow max:
  t2->setPositionIncrement(IndexWriter::MAX_POSITION);
  if (random()->nextBoolean()) {
    t2->setPayload(make_shared<BytesRef>(std::deque<char>{0x1}));
  }
  doc->push_back(make_shared<TextField>(
      L"foo", make_shared<CannedTokenStream>(
                  std::deque<std::shared_ptr<Token>>{t1, t2})));
  expectThrows(invalid_argument::typeid, [&]() { iw->addDocument(doc); });

  // Document should not be visible:
  shared_ptr<IndexReader> r = DirectoryReader::open(iw);
  assertEquals(0, r->numDocs());
  delete r;

  delete iw;
  delete dir;
}

void TestMaxPosition::testMaxPosition() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  // This is at position 0:
  shared_ptr<Token> t1 = make_shared<Token>(L"foo", 0, 3);
  if (random()->nextBoolean()) {
    t1->setPayload(make_shared<BytesRef>(std::deque<char>{0x1}));
  }
  shared_ptr<Token> t2 = make_shared<Token>(L"foo", 4, 7);
  t2->setPositionIncrement(IndexWriter::MAX_POSITION);
  if (random()->nextBoolean()) {
    t2->setPayload(make_shared<BytesRef>(std::deque<char>{0x1}));
  }
  doc->push_back(make_shared<TextField>(
      L"foo", make_shared<CannedTokenStream>(
                  std::deque<std::shared_ptr<Token>>{t1, t2})));
  iw->addDocument(doc);

  // Document should be visible:
  shared_ptr<IndexReader> r = DirectoryReader::open(iw);
  assertEquals(1, r->numDocs());
  shared_ptr<PostingsEnum> postings = MultiFields::getTermPositionsEnum(
      r, L"foo", make_shared<BytesRef>(L"foo"));

  // "foo" appears in docID=0
  assertEquals(0, postings->nextDoc());

  // "foo" appears 2 times in the doc
  assertEquals(2, postings->freq());

  // first at pos=0
  assertEquals(0, postings->nextPosition());

  // next at pos=MAX
  assertEquals(IndexWriter::MAX_POSITION, postings->nextPosition());

  delete r;

  delete iw;
  delete dir;
}
} // namespace org::apache::lucene::index