using namespace std;

#include "TestFacetsConfig.h"

namespace org::apache::lucene::facet
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestFacetsConfig::testPathToStringAndBack() 
{
  int iters = atLeast(1000);
  for (int i = 0; i < iters; i++) {
    int numParts = TestUtil::nextInt(random(), 1, 6);
    std::deque<wstring> parts(numParts);
    for (int j = 0; j < numParts; j++) {
      wstring s;
      while (true) {
        s = TestUtil::randomUnicodeString(random());
        if (s.length() > 0) {
          break;
        }
      }
      parts[j] = s;
    }

    wstring s = FacetsConfig::pathToString(parts);
    std::deque<wstring> parts2 = FacetsConfig::stringToPath(s);
    assertTrue(Arrays::equals(parts, parts2));
  }
}

void TestFacetsConfig::testAddSameDocTwice() 
{
  // LUCENE-5367: this was a problem with the previous code, making sure it
  // works with the new code.
  shared_ptr<Directory> indexDir = newDirectory(), taxoDir = newDirectory();
  shared_ptr<IndexWriter> indexWriter = make_shared<IndexWriter>(
      indexDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<FacetsConfig> facetsConfig = make_shared<FacetsConfig>();
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"a", L"b"));
  doc = facetsConfig->build(taxoWriter, doc);
  // these two addDocument() used to fail
  indexWriter->addDocument(doc);
  indexWriter->addDocument(doc);
  delete indexWriter;
  IOUtils::close({taxoWriter});

  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoDir);
  shared_ptr<IndexSearcher> searcher = newSearcher(indexReader);
  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
  searcher->search(make_shared<MatchAllDocsQuery>(), fc);

  shared_ptr<Facets> facets =
      getTaxonomyFacetCounts(taxoReader, facetsConfig, fc);
  shared_ptr<FacetResult> res = facets->getTopChildren(10, L"a");
  TestUtil::assertEquals(1, res->labelValues.size());
  TestUtil::assertEquals(2, res->labelValues[0]->value);
  IOUtils::close({indexReader, taxoReader});

  IOUtils::close({indexDir, taxoDir});
}

void TestFacetsConfig::testCustomDefault()
{
  shared_ptr<FacetsConfig> config =
      make_shared<FacetsConfigAnonymousInnerClass>(shared_from_this());

  assertTrue(config->getDimConfig(L"foobar")->hierarchical);
}

TestFacetsConfig::FacetsConfigAnonymousInnerClass::
    FacetsConfigAnonymousInnerClass(shared_ptr<TestFacetsConfig> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<DimConfig>
TestFacetsConfig::FacetsConfigAnonymousInnerClass::getDefaultDimConfig()
{
  shared_ptr<DimConfig> config = make_shared<DimConfig>();
  config->hierarchical = true;
  return config;
}
} // namespace org::apache::lucene::facet