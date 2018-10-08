using namespace std;

#include "TestExternalCodecs.h"

namespace org::apache::lucene
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using AssertingCodec = org::apache::lucene::codecs::asserting::AssertingCodec;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<PostingsFormat>
TestExternalCodecs::CustomPerFieldCodec::getPostingsFormatForField(
    const wstring &field)
{
  if (field == L"field2" || field == L"id") {
    return memoryFormat;
  } else if (field == L"field1") {
    return defaultFormat;
  } else {
    return ramFormat;
  }
}

void TestExternalCodecs::testPerFieldCodec() 
{

  constexpr int NUM_DOCS = atLeast(173);
  if (VERBOSE) {
    wcout << L"TEST: NUM_DOCS=" << NUM_DOCS << endl;
  }

  shared_ptr<BaseDirectoryWrapper> dir = newDirectory();
  dir->setCheckIndexOnClose(false); // we use a custom codec provider
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setCodec(make_shared<CustomPerFieldCodec>())
               ->setMergePolicy(newLogMergePolicy(3)));
  shared_ptr<Document> doc = make_shared<Document>();
  // uses default codec:
  doc->push_back(newTextField(L"field1",
                              L"this field uses the standard codec as the test",
                              Field::Store::NO));
  // uses memory codec:
  shared_ptr<Field> field2 =
      newTextField(L"field2", L"this field uses the memory codec as the test",
                   Field::Store::NO);
  doc->push_back(field2);

  shared_ptr<Field> idField = newStringField(L"id", L"", Field::Store::NO);

  doc->push_back(idField);
  for (int i = 0; i < NUM_DOCS; i++) {
    idField->setStringValue(L"" + to_wstring(i));
    w->addDocument(doc);
    if ((i + 1) % 10 == 0) {
      w->commit();
    }
  }
  if (VERBOSE) {
    wcout << L"TEST: now delete id=77" << endl;
  }
  w->deleteDocuments({make_shared<Term>(L"id", L"77")});

  shared_ptr<IndexReader> r = DirectoryReader::open(w);

  TestUtil::assertEquals(NUM_DOCS - 1, r->numDocs());
  shared_ptr<IndexSearcher> s = newSearcher(r);
  TestUtil::assertEquals(
      NUM_DOCS - 1,
      s->search(
           make_shared<TermQuery>(make_shared<Term>(L"field1", L"standard")), 1)
          ->totalHits);
  TestUtil::assertEquals(
      NUM_DOCS - 1,
      s->search(make_shared<TermQuery>(make_shared<Term>(L"field2", L"memory")),
                1)
          ->totalHits);
  delete r;

  if (VERBOSE) {
    wcout << L"\nTEST: now delete 2nd doc" << endl;
  }
  w->deleteDocuments({make_shared<Term>(L"id", L"44")});

  if (VERBOSE) {
    wcout << L"\nTEST: now force merge" << endl;
  }
  w->forceMerge(1);
  if (VERBOSE) {
    wcout << L"\nTEST: now open reader" << endl;
  }
  r = DirectoryReader::open(w);
  TestUtil::assertEquals(NUM_DOCS - 2, r->maxDoc());
  TestUtil::assertEquals(NUM_DOCS - 2, r->numDocs());
  s = newSearcher(r);
  TestUtil::assertEquals(
      NUM_DOCS - 2,
      s->search(
           make_shared<TermQuery>(make_shared<Term>(L"field1", L"standard")), 1)
          ->totalHits);
  TestUtil::assertEquals(
      NUM_DOCS - 2,
      s->search(make_shared<TermQuery>(make_shared<Term>(L"field2", L"memory")),
                1)
          ->totalHits);
  TestUtil::assertEquals(
      1, s->search(make_shared<TermQuery>(make_shared<Term>(L"id", L"76")), 1)
             ->totalHits);
  TestUtil::assertEquals(
      0, s->search(make_shared<TermQuery>(make_shared<Term>(L"id", L"77")), 1)
             ->totalHits);
  TestUtil::assertEquals(
      0, s->search(make_shared<TermQuery>(make_shared<Term>(L"id", L"44")), 1)
             ->totalHits);

  if (VERBOSE) {
    wcout << L"\nTEST: now close NRT reader" << endl;
  }
  delete r;

  delete w;

  delete dir;
}
} // namespace org::apache::lucene