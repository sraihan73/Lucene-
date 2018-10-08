using namespace std;

#include "TestTryDelete.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using StringField = org::apache::lucene::document::StringField;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ReferenceManager = org::apache::lucene::search::ReferenceManager;
using SearcherFactory = org::apache::lucene::search::SearcherFactory;
using SearcherManager = org::apache::lucene::search::SearcherManager;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

shared_ptr<IndexWriter>
TestTryDelete::getWriter(shared_ptr<Directory> directory) 
{
  shared_ptr<MergePolicy> policy = make_shared<LogByteSizeMergePolicy>();
  shared_ptr<IndexWriterConfig> conf =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  conf->setMergePolicy(policy);
  conf->setOpenMode(OpenMode::CREATE_OR_APPEND);

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(directory, conf);

  return writer;
}

shared_ptr<Directory> TestTryDelete::createIndex() 
{
  shared_ptr<Directory> directory = make_shared<RAMDirectory>();

  shared_ptr<IndexWriter> writer = getWriter(directory);

  for (int i = 0; i < 10; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"foo", to_wstring(i), Store::YES));
    writer->addDocument(doc);
  }

  writer->commit();
  delete writer;

  return directory;
}

void TestTryDelete::testTryDeleteDocument() 
{
  shared_ptr<Directory> directory = createIndex();

  shared_ptr<IndexWriter> writer = getWriter(directory);

  shared_ptr<ReferenceManager<std::shared_ptr<IndexSearcher>>> mgr =
      make_shared<SearcherManager>(writer, make_shared<SearcherFactory>());

  shared_ptr<IndexSearcher> searcher = mgr->acquire();

  shared_ptr<TopDocs> topDocs = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"0")), 100);
  assertEquals(1, topDocs->totalHits);

  int64_t result;
  if (random()->nextBoolean()) {
    shared_ptr<IndexReader> r = DirectoryReader::open(writer);
    result = writer->tryDeleteDocument(r, 0);
    delete r;
  } else {
    result = writer->tryDeleteDocument(searcher->getIndexReader(), 0);
  }

  // The tryDeleteDocument should have succeeded:
  assertTrue(result != -1);

  assertTrue(writer->hasDeletions());

  if (random()->nextBoolean()) {
    writer->commit();
  }

  assertTrue(writer->hasDeletions());

  mgr->maybeRefresh();

  searcher = mgr->acquire();

  topDocs = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"0")), 100);

  assertEquals(0, topDocs->totalHits);
}

void TestTryDelete::testTryDeleteDocumentCloseAndReopen() 
{
  shared_ptr<Directory> directory = createIndex();

  shared_ptr<IndexWriter> writer = getWriter(directory);

  shared_ptr<ReferenceManager<std::shared_ptr<IndexSearcher>>> mgr =
      make_shared<SearcherManager>(writer, make_shared<SearcherFactory>());

  shared_ptr<IndexSearcher> searcher = mgr->acquire();

  shared_ptr<TopDocs> topDocs = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"0")), 100);
  assertEquals(1, topDocs->totalHits);

  int64_t result =
      writer->tryDeleteDocument(DirectoryReader::open(writer), 0);

  assertTrue(result != -1);

  writer->commit();

  assertTrue(writer->hasDeletions());

  mgr->maybeRefresh();

  searcher = mgr->acquire();

  topDocs = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"0")), 100);

  assertEquals(0, topDocs->totalHits);

  delete writer;

  searcher = make_shared<IndexSearcher>(DirectoryReader::open(directory));

  topDocs = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"0")), 100);

  assertEquals(0, topDocs->totalHits);
}

void TestTryDelete::testDeleteDocuments() 
{
  shared_ptr<Directory> directory = createIndex();

  shared_ptr<IndexWriter> writer = getWriter(directory);

  shared_ptr<ReferenceManager<std::shared_ptr<IndexSearcher>>> mgr =
      make_shared<SearcherManager>(writer, make_shared<SearcherFactory>());

  shared_ptr<IndexSearcher> searcher = mgr->acquire();

  shared_ptr<TopDocs> topDocs = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"0")), 100);
  assertEquals(1, topDocs->totalHits);

  int64_t result = writer->deleteDocuments(
      {make_shared<TermQuery>(make_shared<Term>(L"foo", L"0"))});

  assertTrue(result != -1);

  // writer.commit();

  assertTrue(writer->hasDeletions());

  mgr->maybeRefresh();

  searcher = mgr->acquire();

  topDocs = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"0")), 100);

  assertEquals(0, topDocs->totalHits);
}
} // namespace org::apache::lucene::index