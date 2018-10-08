using namespace std;

#include "TestManyFields.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const shared_ptr<org::apache::lucene::document::FieldType>
    TestManyFields::storedTextType =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::TextField::TYPE_NOT_STORED);

void TestManyFields::testManyFields() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(10));
  for (int j = 0; j < 100; j++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newField(L"a" + to_wstring(j), L"aaa" + to_wstring(j), storedTextType));
    doc->push_back(
        newField(L"b" + to_wstring(j), L"aaa" + to_wstring(j), storedTextType));
    doc->push_back(
        newField(L"c" + to_wstring(j), L"aaa" + to_wstring(j), storedTextType));
    doc->push_back(newField(L"d" + to_wstring(j), L"aaa", storedTextType));
    doc->push_back(newField(L"e" + to_wstring(j), L"aaa", storedTextType));
    doc->push_back(newField(L"f" + to_wstring(j), L"aaa", storedTextType));
    writer->addDocument(doc);
  }
  delete writer;

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  assertEquals(100, reader->maxDoc());
  assertEquals(100, reader->numDocs());
  for (int j = 0; j < 100; j++) {
    assertEquals(1, reader->docFreq(make_shared<Term>(L"a" + to_wstring(j),
                                                      L"aaa" + to_wstring(j))));
    assertEquals(1, reader->docFreq(make_shared<Term>(L"b" + to_wstring(j),
                                                      L"aaa" + to_wstring(j))));
    assertEquals(1, reader->docFreq(make_shared<Term>(L"c" + to_wstring(j),
                                                      L"aaa" + to_wstring(j))));
    assertEquals(
        1, reader->docFreq(make_shared<Term>(L"d" + to_wstring(j), L"aaa")));
    assertEquals(
        1, reader->docFreq(make_shared<Term>(L"e" + to_wstring(j), L"aaa")));
    assertEquals(
        1, reader->docFreq(make_shared<Term>(L"f" + to_wstring(j), L"aaa")));
  }
  delete reader;
  delete dir;
}

void TestManyFields::testDiverseDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setRAMBufferSizeMB(0.5));
  int n = atLeast(1);
  for (int i = 0; i < n; i++) {
    // First, docs where every term is unique (heavy on
    // Posting instances)
    for (int j = 0; j < 100; j++) {
      shared_ptr<Document> doc = make_shared<Document>();
      for (int k = 0; k < 100; k++) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        doc->push_back(newField(
            L"field", Integer::toString(random()->nextInt()), storedTextType));
      }
      writer->addDocument(doc);
    }

    // Next, many single term docs where only one term
    // occurs (heavy on byte blocks)
    for (int j = 0; j < 100; j++) {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(newField(L"field",
                              L"aaa aaa aaa aaa aaa aaa aaa aaa aaa aaa",
                              storedTextType));
      writer->addDocument(doc);
    }

    // Next, many single term docs where only one term
    // occurs but the terms are very long (heavy on
    // char[] arrays)
    for (int j = 0; j < 100; j++) {
      shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring x = Integer::toString(j) + L".";
      for (int k = 0; k < 1000; k++) {
        b->append(x);
      }
      wstring longTerm = b->toString();

      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(newField(L"field", longTerm, storedTextType));
      writer->addDocument(doc);
    }
  }
  delete writer;

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  int64_t totalHits =
      searcher
          ->search(make_shared<TermQuery>(make_shared<Term>(L"field", L"aaa")),
                   1)
          ->totalHits;
  assertEquals(n * 100, totalHits);
  delete reader;

  delete dir;
}

void TestManyFields::testRotatingFieldNames() 
{
  shared_ptr<Directory> dir =
      newFSDirectory(createTempDir(L"TestIndexWriter.testChangingFields"));
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setRAMBufferSizeMB(0.2);
  iwc->setMaxBufferedDocs(-1);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  int upto = 0;

  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setOmitNorms(true);

  int firstDocCount = -1;
  for (int iter = 0; iter < 10; iter++) {
    constexpr int startFlushCount = w->getFlushCount();
    int docCount = 0;
    while (w->getFlushCount() == startFlushCount) {
      shared_ptr<Document> doc = make_shared<Document>();
      for (int i = 0; i < 10; i++) {
        doc->push_back(
            make_shared<Field>(L"field" + to_wstring(upto++), L"content", ft));
      }
      w->addDocument(doc);
      docCount++;
    }

    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << L" flushed after docCount=" << docCount
            << endl;
    }

    if (iter == 0) {
      firstDocCount = docCount;
    }

    assertTrue(
        L"flushed after too few docs: first segment flushed at docCount=" +
            to_wstring(firstDocCount) +
            L", but current segment flushed after docCount=" +
            to_wstring(docCount) + L"; iter=" + to_wstring(iter),
        (static_cast<float>(docCount)) / firstDocCount > 0.9);

    if (upto > 5000) {
      // Start re-using field names after a while
      // ... important because otherwise we can OOME due
      // to too many FieldInfo instances.
      upto = 0;
    }
  }
  delete w;
  delete dir;
}
} // namespace org::apache::lucene::index