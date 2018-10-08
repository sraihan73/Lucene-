using namespace std;

#include "TestMultiPhraseEnum.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestMultiPhraseEnum::testOneDocument() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(L"field", L"foo bar", Field::Store::NO));
  writer->addDocument(doc);

  shared_ptr<DirectoryReader> ir = DirectoryReader::open(writer);
  delete writer;

  shared_ptr<PostingsEnum> p1 = getOnlyLeafReader(ir)->postings(
      make_shared<Term>(L"field", L"foo"), PostingsEnum::POSITIONS);
  shared_ptr<PostingsEnum> p2 = getOnlyLeafReader(ir)->postings(
      make_shared<Term>(L"field", L"bar"), PostingsEnum::POSITIONS);
  shared_ptr<PostingsEnum> union_ =
      make_shared<MultiPhraseQuery::UnionPostingsEnum>(Arrays::asList(p1, p2));

  assertEquals(-1, union_->docID());

  assertEquals(0, union_->nextDoc());
  assertEquals(2, union_->freq());
  assertEquals(0, union_->nextPosition());
  assertEquals(1, union_->nextPosition());

  assertEquals(DocIdSetIterator::NO_MORE_DOCS, union_->nextDoc());

  ir->close();
  delete dir;
}

void TestMultiPhraseEnum::testSomeDocuments() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"field", L"foo", Field::Store::NO));
  writer->addDocument(doc);

  writer->addDocument(make_shared<Document>());

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(L"field", L"foo bar", Field::Store::NO));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"field", L"bar", Field::Store::NO));
  writer->addDocument(doc);

  writer->forceMerge(1);
  shared_ptr<DirectoryReader> ir = DirectoryReader::open(writer);
  delete writer;

  shared_ptr<PostingsEnum> p1 = getOnlyLeafReader(ir)->postings(
      make_shared<Term>(L"field", L"foo"), PostingsEnum::POSITIONS);
  shared_ptr<PostingsEnum> p2 = getOnlyLeafReader(ir)->postings(
      make_shared<Term>(L"field", L"bar"), PostingsEnum::POSITIONS);
  shared_ptr<PostingsEnum> union_ =
      make_shared<MultiPhraseQuery::UnionPostingsEnum>(Arrays::asList(p1, p2));

  assertEquals(-1, union_->docID());

  assertEquals(0, union_->nextDoc());
  assertEquals(1, union_->freq());
  assertEquals(0, union_->nextPosition());

  assertEquals(2, union_->nextDoc());
  assertEquals(2, union_->freq());
  assertEquals(0, union_->nextPosition());
  assertEquals(1, union_->nextPosition());

  assertEquals(3, union_->nextDoc());
  assertEquals(1, union_->freq());
  assertEquals(0, union_->nextPosition());

  assertEquals(DocIdSetIterator::NO_MORE_DOCS, union_->nextDoc());

  ir->close();
  delete dir;
}
} // namespace org::apache::lucene::search