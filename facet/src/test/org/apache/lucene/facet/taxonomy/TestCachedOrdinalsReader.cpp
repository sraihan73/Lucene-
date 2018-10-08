using namespace std;

#include "TestCachedOrdinalsReader.h"

namespace org::apache::lucene::facet::taxonomy
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using FacetField = org::apache::lucene::facet::FacetField;
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithThreads() throws Exception
void TestCachedOrdinalsReader::testWithThreads() 
{
  // LUCENE-5303: OrdinalsCache used the ThreadLocal BinaryDV instead of
  // reader.getCoreCacheKey().
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(indexDir, conf);
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"A", L"1"));
  writer->addDocument(config->build(taxoWriter, doc));
  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"A", L"2"));
  writer->addDocument(config->build(taxoWriter, doc));

  shared_ptr<DirectoryReader> *const reader = DirectoryReader::open(writer);
  shared_ptr<CachedOrdinalsReader> *const ordsReader =
      make_shared<CachedOrdinalsReader>(make_shared<DocValuesOrdinalsReader>(
          FacetsConfig::DEFAULT_INDEX_FIELD_NAME));
  std::deque<std::shared_ptr<Thread>> threads(3);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), L"CachedOrdsThread-" + to_wstring(i), reader,
        ordsReader);
  }

  int64_t ramBytesUsed = 0;
  for (auto t : threads) {
    t->start();
    t->join();
    if (ramBytesUsed == 0) {
      ramBytesUsed = ordsReader->ramBytesUsed();
    } else {
      assertEquals(ramBytesUsed, ordsReader->ramBytesUsed());
    }
  }

  delete writer;
  IOUtils::close({taxoWriter, reader, indexDir, taxoDir});
}

TestCachedOrdinalsReader::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestCachedOrdinalsReader> outerInstance,
    wstring L"CachedOrdsThread-" + i, shared_ptr<DirectoryReader> reader,
    shared_ptr<org::apache::lucene::facet::taxonomy::CachedOrdinalsReader>
        ordsReader)
    : Thread(L"CachedOrdsThread-" + i)
{
  this->outerInstance = outerInstance;
  this->reader = reader;
  this->ordsReader = ordsReader;
}

void TestCachedOrdinalsReader::ThreadAnonymousInnerClass::run()
{
  for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
    try {
      ordsReader->getReader(context);
    } catch (const IOException &e) {
      throw runtime_error(e);
    }
  }
}
} // namespace org::apache::lucene::facet::taxonomy