using namespace std;

#include "TestNRTCachingDirectory.h"

namespace org::apache::lucene::store
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using BytesRef = org::apache::lucene::util::BytesRef;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Directory>
TestNRTCachingDirectory::getDirectory(shared_ptr<Path> path) 
{
  return make_shared<NRTCachingDirectory>(make_shared<RAMDirectory>(),
                                          .1 + 2.0 * random()->nextDouble(),
                                          .1 + 5.0 * random()->nextDouble());
}

void TestNRTCachingDirectory::testNRTAndCommit() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<NRTCachingDirectory> cachedDir =
      make_shared<NRTCachingDirectory>(dir, 2.0, 25.0);
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), cachedDir, conf);
  shared_ptr<LineFileDocs> *const docs = make_shared<LineFileDocs>(random());
  constexpr int numDocs = TestUtil::nextInt(random(), 100, 400);

  if (VERBOSE) {
    wcout << L"TEST: numDocs=" << numDocs << endl;
  }

  const deque<std::shared_ptr<BytesRef>> ids =
      deque<std::shared_ptr<BytesRef>>();
  shared_ptr<DirectoryReader> r = nullptr;
  for (int docCount = 0; docCount < numDocs; docCount++) {
    shared_ptr<Document> *const doc = docs->nextDoc();
    ids.push_back(make_shared<BytesRef>(doc[L"docid"]));
    w->addDocument(doc);
    if (random()->nextInt(20) == 17) {
      if (r == nullptr) {
        r = DirectoryReader::open(w->w);
      } else {
        shared_ptr<DirectoryReader> *const r2 =
            DirectoryReader::openIfChanged(r);
        if (r2 != nullptr) {
          r->close();
          r = r2;
        }
      }
      TestUtil::assertEquals(1 + docCount, r->numDocs());
      shared_ptr<IndexSearcher> *const s = newSearcher(r);
      // Just make sure search can run; we can't assert
      // totHits since it could be 0
      shared_ptr<TopDocs> hits = s->search(
          make_shared<TermQuery>(make_shared<Term>(L"body", L"the")), 10);
      // System.out.println("tot hits " + hits.totalHits);
    }
  }

  if (r != nullptr) {
    r->close();
  }

  // Close should force cache to clear since all files are sync'd
  delete w;

  const std::deque<wstring> cachedFiles = cachedDir->listCachedFiles();
  for (auto file : cachedFiles) {
    wcout << L"FAIL: cached file " << file << L" remains after sync" << endl;
  }
  TestUtil::assertEquals(0, cachedFiles.size());

  r = DirectoryReader::open(dir);
  for (auto id : ids) {
    TestUtil::assertEquals(1, r->docFreq(make_shared<Term>(L"docid", id)));
  }
  r->close();
  delete cachedDir;
  delete docs;
}

void TestNRTCachingDirectory::verifyCompiles() 
{
  shared_ptr<Analyzer> analyzer = nullptr;

  shared_ptr<Directory> fsDir = FSDirectory::open(createTempDir(L"verify"));
  shared_ptr<NRTCachingDirectory> cachedFSDir =
      make_shared<NRTCachingDirectory>(fsDir, 2.0, 25.0);
  shared_ptr<IndexWriterConfig> conf = make_shared<IndexWriterConfig>(analyzer);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(cachedFSDir, conf);
  delete writer;
  delete cachedFSDir;
}

void TestNRTCachingDirectory::testCreateTempOutputSameName() throw(
    runtime_error)
{

  shared_ptr<Directory> fsDir = FSDirectory::open(createTempDir(L"verify"));
  shared_ptr<NRTCachingDirectory> nrtDir =
      make_shared<NRTCachingDirectory>(fsDir, 2.0, 25.0);
  wstring name = L"foo_bar_0.tmp";
  delete nrtDir->createOutput(name, IOContext::DEFAULT);

  shared_ptr<IndexOutput> out =
      nrtDir->createTempOutput(L"foo", L"bar", IOContext::DEFAULT);
  assertFalse(name == out->getName());
  delete out;
  delete nrtDir;
  delete fsDir;
}
} // namespace org::apache::lucene::store