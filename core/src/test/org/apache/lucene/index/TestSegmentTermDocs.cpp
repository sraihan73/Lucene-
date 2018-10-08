using namespace std;

#include "TestSegmentTermDocs.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;

void TestSegmentTermDocs::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  DocHelper::setupDoc(testDoc);
  info = DocHelper::writeDoc(random(), dir, testDoc);
}

void TestSegmentTermDocs::tearDown() 
{
  delete dir;
  LuceneTestCase::tearDown();
}

void TestSegmentTermDocs::test() { assertTrue(dir != nullptr); }

void TestSegmentTermDocs::testTermDocs() 
{
  // After adding the document, we should be able to read it back in
  shared_ptr<SegmentReader> reader = make_shared<SegmentReader>(
      info, Version::LATEST->major, newIOContext(random()));
  assertTrue(reader != nullptr);

  shared_ptr<TermsEnum> terms =
      reader->terms(DocHelper::TEXT_FIELD_2_KEY)->begin();
  terms->seekCeil(make_shared<BytesRef>(L"field"));
  shared_ptr<PostingsEnum> termDocs =
      TestUtil::docs(random(), terms, nullptr, PostingsEnum::FREQS);
  if (termDocs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
    int docId = termDocs->docID();
    assertTrue(docId == 0);
    int freq = termDocs->freq();
    assertTrue(freq == 3);
  }
  delete reader;
}

void TestSegmentTermDocs::testBadSeek() 
{
  {
    // After adding the document, we should be able to read it back in
    shared_ptr<SegmentReader> reader = make_shared<SegmentReader>(
        info, Version::LATEST->major, newIOContext(random()));
    assertTrue(reader != nullptr);
    shared_ptr<PostingsEnum> termDocs =
        TestUtil::docs(random(), reader, L"textField2",
                       make_shared<BytesRef>(L"bad"), nullptr, 0);

    assertNull(termDocs);
    delete reader;
  }
  {
    // After adding the document, we should be able to read it back in
    shared_ptr<SegmentReader> reader = make_shared<SegmentReader>(
        info, Version::LATEST->major, newIOContext(random()));
    assertTrue(reader != nullptr);
    shared_ptr<PostingsEnum> termDocs = TestUtil::docs(
        random(), reader, L"junk", make_shared<BytesRef>(L"bad"), nullptr, 0);
    assertNull(termDocs);
    delete reader;
  }
}

void TestSegmentTermDocs::testSkipTo() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergePolicy(newLogMergePolicy()));

  shared_ptr<Term> ta = make_shared<Term>(L"content", L"aaa");
  for (int i = 0; i < 10; i++) {
    addDoc(writer, L"aaa aaa aaa aaa");
  }

  shared_ptr<Term> tb = make_shared<Term>(L"content", L"bbb");
  for (int i = 0; i < 16; i++) {
    addDoc(writer, L"bbb bbb bbb bbb");
  }

  shared_ptr<Term> tc = make_shared<Term>(L"content", L"ccc");
  for (int i = 0; i < 50; i++) {
    addDoc(writer, L"ccc ccc ccc ccc");
  }

  // assure that we deal with a single segment
  writer->forceMerge(1);
  delete writer;

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);

  shared_ptr<PostingsEnum> tdocs = TestUtil::docs(
      random(), reader, ta->field(), make_shared<BytesRef>(ta->text()), nullptr,
      PostingsEnum::FREQS);

  // without optimization (assumption skipInterval == 16)

  // with next
  assertTrue(tdocs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(0, tdocs->docID());
  TestUtil::assertEquals(4, tdocs->freq());
  assertTrue(tdocs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(1, tdocs->docID());
  TestUtil::assertEquals(4, tdocs->freq());
  assertTrue(tdocs->advance(2) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(2, tdocs->docID());
  assertTrue(tdocs->advance(4) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(4, tdocs->docID());
  assertTrue(tdocs->advance(9) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(9, tdocs->docID());
  assertFalse(tdocs->advance(10) != DocIdSetIterator::NO_MORE_DOCS);

  // without next
  tdocs = TestUtil::docs(random(), reader, ta->field(),
                         make_shared<BytesRef>(ta->text()), nullptr, 0);

  assertTrue(tdocs->advance(0) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(0, tdocs->docID());
  assertTrue(tdocs->advance(4) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(4, tdocs->docID());
  assertTrue(tdocs->advance(9) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(9, tdocs->docID());
  assertFalse(tdocs->advance(10) != DocIdSetIterator::NO_MORE_DOCS);

  // exactly skipInterval documents and therefore with optimization

  // with next
  tdocs = TestUtil::docs(random(), reader, tb->field(),
                         make_shared<BytesRef>(tb->text()), nullptr,
                         PostingsEnum::FREQS);

  assertTrue(tdocs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(10, tdocs->docID());
  TestUtil::assertEquals(4, tdocs->freq());
  assertTrue(tdocs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(11, tdocs->docID());
  TestUtil::assertEquals(4, tdocs->freq());
  assertTrue(tdocs->advance(12) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(12, tdocs->docID());
  assertTrue(tdocs->advance(15) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(15, tdocs->docID());
  assertTrue(tdocs->advance(24) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(24, tdocs->docID());
  assertTrue(tdocs->advance(25) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(25, tdocs->docID());
  assertFalse(tdocs->advance(26) != DocIdSetIterator::NO_MORE_DOCS);

  // without next
  tdocs = TestUtil::docs(random(), reader, tb->field(),
                         make_shared<BytesRef>(tb->text()), nullptr,
                         PostingsEnum::FREQS);

  assertTrue(tdocs->advance(5) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(10, tdocs->docID());
  assertTrue(tdocs->advance(15) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(15, tdocs->docID());
  assertTrue(tdocs->advance(24) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(24, tdocs->docID());
  assertTrue(tdocs->advance(25) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(25, tdocs->docID());
  assertFalse(tdocs->advance(26) != DocIdSetIterator::NO_MORE_DOCS);

  // much more than skipInterval documents and therefore with optimization

  // with next
  tdocs = TestUtil::docs(random(), reader, tc->field(),
                         make_shared<BytesRef>(tc->text()), nullptr,
                         PostingsEnum::FREQS);

  assertTrue(tdocs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(26, tdocs->docID());
  TestUtil::assertEquals(4, tdocs->freq());
  assertTrue(tdocs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(27, tdocs->docID());
  TestUtil::assertEquals(4, tdocs->freq());
  assertTrue(tdocs->advance(28) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(28, tdocs->docID());
  assertTrue(tdocs->advance(40) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(40, tdocs->docID());
  assertTrue(tdocs->advance(57) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(57, tdocs->docID());
  assertTrue(tdocs->advance(74) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(74, tdocs->docID());
  assertTrue(tdocs->advance(75) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(75, tdocs->docID());
  assertFalse(tdocs->advance(76) != DocIdSetIterator::NO_MORE_DOCS);

  // without next
  tdocs = TestUtil::docs(random(), reader, tc->field(),
                         make_shared<BytesRef>(tc->text()), nullptr, 0);
  assertTrue(tdocs->advance(5) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(26, tdocs->docID());
  assertTrue(tdocs->advance(40) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(40, tdocs->docID());
  assertTrue(tdocs->advance(57) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(57, tdocs->docID());
  assertTrue(tdocs->advance(74) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(74, tdocs->docID());
  assertTrue(tdocs->advance(75) != DocIdSetIterator::NO_MORE_DOCS);
  TestUtil::assertEquals(75, tdocs->docID());
  assertFalse(tdocs->advance(76) != DocIdSetIterator::NO_MORE_DOCS);

  delete reader;
  delete dir;
}

void TestSegmentTermDocs::addDoc(shared_ptr<IndexWriter> writer,
                                 const wstring &value) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", value, Field::Store::NO));
  writer->addDocument(doc);
}
} // namespace org::apache::lucene::index