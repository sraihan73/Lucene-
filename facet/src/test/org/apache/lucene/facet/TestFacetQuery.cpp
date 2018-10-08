using namespace std;

#include "TestFacetQuery.h"

namespace org::apache::lucene::facet
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using SortedSetDocValuesFacetField =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesFacetField;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexableField = org::apache::lucene::index::IndexableField;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::junit::AfterClass;
using org::junit::BeforeClass;
using org::junit::Test;
shared_ptr<org::apache::lucene::store::Directory>
    TestFacetQuery::indexDirectory;
shared_ptr<org::apache::lucene::index::RandomIndexWriter>
    TestFacetQuery::indexWriter;
shared_ptr<org::apache::lucene::index::IndexReader> TestFacetQuery::indexReader;
shared_ptr<org::apache::lucene::search::IndexSearcher> TestFacetQuery::searcher;
shared_ptr<FacetsConfig> TestFacetQuery::config;
std::deque<std::shared_ptr<org::apache::lucene::index::IndexableField>> const
    TestFacetQuery::DOC_SINGLEVALUED = std::deque<
        std::shared_ptr<org::apache::lucene::index::IndexableField>>{
        make_shared<org::apache::lucene::facet::sortedset::
                        SortedSetDocValuesFacetField>(L"Author",
                                                      L"Mark Twain")};
std::deque<std::shared_ptr<org::apache::lucene::index::IndexableField>> const
    TestFacetQuery::DOC_MULTIVALUED = std::deque<std::shared_ptr<
        org::apache::lucene::facet::sortedset::SortedSetDocValuesFacetField>>{
        make_shared<org::apache::lucene::facet::sortedset::
                        SortedSetDocValuesFacetField>(L"Author",
                                                      L"Kurt Vonnegut")};
std::deque<std::shared_ptr<org::apache::lucene::index::IndexableField>> const
    TestFacetQuery::DOC_NOFACET = std::deque<
        std::shared_ptr<org::apache::lucene::index::IndexableField>>{
        make_shared<org::apache::lucene::document::TextField>(
            L"Hello", L"World",
            org::apache::lucene::document::Field::Store::YES)};

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void createTestIndex() throws
// java.io.IOException
void TestFacetQuery::createTestIndex() 
{
  indexDirectory = newDirectory();
  // create and open an index writer
  indexWriter = make_shared<RandomIndexWriter>(
      random(), indexDirectory,
      newIndexWriterConfig(make_shared<MockAnalyzer>(
          random(), MockTokenizer::WHITESPACE, false)));

  config = make_shared<FacetsConfig>();

  indexDocuments({DOC_SINGLEVALUED, DOC_MULTIVALUED, DOC_NOFACET});

  indexReader = indexWriter->getReader();
  // prepare searcher to search against
  searcher = newSearcher(indexReader);
}

void TestFacetQuery::indexDocuments(deque<IndexableField> &docs) throw(
    IOException)
{
  for (shared_ptr<IndexableField> fields : docs) {
    for (auto field : fields) {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(field);
      indexWriter->addDocument(config->build(doc));
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void closeTestIndex() throws
// java.io.IOException
void TestFacetQuery::closeTestIndex() 
{
  IOUtils::close({indexReader, indexWriter, indexDirectory});
  indexReader.reset();
  indexWriter.reset();
  indexDirectory.reset();
  searcher.reset();
  config.reset();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSingleValued() throws Exception
void TestFacetQuery::testSingleValued() 
{
  shared_ptr<TopDocs> topDocs =
      searcher->search(make_shared<FacetQuery>(L"Author", L"Mark Twain"), 10);
  assertEquals(1, topDocs->totalHits);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMultiValued() throws Exception
void TestFacetQuery::testMultiValued() 
{
  shared_ptr<TopDocs> topDocs =
      searcher->search(make_shared<MultiFacetQuery>(
                           L"Author", std::deque<wstring>{L"Mark Twain"},
                           std::deque<wstring>{L"Kurt Vonnegut"}),
                       10);
  assertEquals(2, topDocs->totalHits);
}
} // namespace org::apache::lucene::facet