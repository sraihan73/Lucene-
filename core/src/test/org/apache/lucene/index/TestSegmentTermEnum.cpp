using namespace std;

#include "TestSegmentTermEnum.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestSegmentTermEnum::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
}

void TestSegmentTermEnum::tearDown() 
{
  delete dir;
  LuceneTestCase::tearDown();
}

void TestSegmentTermEnum::testTermEnum() 
{
  shared_ptr<IndexWriter> writer = nullptr;

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  // ADD 100 documents with term : aaa
  // add 100 documents with terms: aaa bbb
  // Therefore, term 'aaa' has document frequency of 200 and term 'bbb' 100
  for (int i = 0; i < 100; i++) {
    addDoc(writer, L"aaa");
    addDoc(writer, L"aaa bbb");
  }

  delete writer;

  // verify document frequency of terms in an multi segment index
  verifyDocFreq();

  // merge segments
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::APPEND));
  writer->forceMerge(1);
  delete writer;

  // verify document frequency of terms in a single segment index
  verifyDocFreq();
}

void TestSegmentTermEnum::testPrevTermAtEnd() 
{
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setCodec(TestUtil::alwaysPostingsFormat(
                   TestUtil::getDefaultPostingsFormat())));
  addDoc(writer, L"aaa bbb");
  delete writer;
  shared_ptr<LeafReader> reader = getOnlyLeafReader(DirectoryReader::open(dir));
  shared_ptr<TermsEnum> terms = reader->terms(L"content")->begin();
  assertNotNull(terms->next());
  TestUtil::assertEquals(L"aaa", terms->term()->utf8ToString());
  assertNotNull(terms->next());
  int64_t ordB;
  try {
    ordB = terms->ord();
  } catch (const UnsupportedOperationException &uoe) {
    // ok -- codec is not required to support ord
    delete reader;
    return;
  }
  TestUtil::assertEquals(L"bbb", terms->term()->utf8ToString());
  assertNull(terms->next());

  terms->seekExact(ordB);
  TestUtil::assertEquals(L"bbb", terms->term()->utf8ToString());
  delete reader;
}

void TestSegmentTermEnum::verifyDocFreq() 
{
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<TermsEnum> termEnum =
      MultiFields::getTerms(reader, L"content")->begin();

  // create enumeration of all terms
  // go to the first term (aaa)
  termEnum->next();
  // assert that term is 'aaa'
  TestUtil::assertEquals(L"aaa", termEnum->term()->utf8ToString());
  TestUtil::assertEquals(200, termEnum->docFreq());
  // go to the second term (bbb)
  termEnum->next();
  // assert that term is 'bbb'
  TestUtil::assertEquals(L"bbb", termEnum->term()->utf8ToString());
  TestUtil::assertEquals(100, termEnum->docFreq());

  // create enumeration of terms after term 'aaa',
  // including 'aaa'
  termEnum->seekCeil(make_shared<BytesRef>(L"aaa"));
  // assert that term is 'aaa'
  TestUtil::assertEquals(L"aaa", termEnum->term()->utf8ToString());
  TestUtil::assertEquals(200, termEnum->docFreq());
  // go to term 'bbb'
  termEnum->next();
  // assert that term is 'bbb'
  TestUtil::assertEquals(L"bbb", termEnum->term()->utf8ToString());
  TestUtil::assertEquals(100, termEnum->docFreq());
  delete reader;
}

void TestSegmentTermEnum::addDoc(shared_ptr<IndexWriter> writer,
                                 const wstring &value) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", value, Field::Store::NO));
  writer->addDocument(doc);
}
} // namespace org::apache::lucene::index