using namespace std;

#include "TestOrdinalMappingLeafReader.h"

namespace org::apache::lucene::facet::taxonomy
{
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using FacetField = org::apache::lucene::facet::FacetField;
using FacetResult = org::apache::lucene::facet::FacetResult;
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using LabelAndValue = org::apache::lucene::facet::LabelAndValue;
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using MemoryOrdinalMap = org::apache::lucene::facet::taxonomy::directory::
    DirectoryTaxonomyWriter::MemoryOrdinalMap;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using MultiDocValues = org::apache::lucene::index::MultiDocValues;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::junit::Before;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before @Override public void setUp() throws Exception
void TestOrdinalMappingLeafReader::setUp() 
{
  FacetTestCase::setUp();
  facetConfig->setMultiValued(L"tag", true);
  facetConfig->setIndexFieldName(L"tag",
                                 L"$tags"); // add custom index field name
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTaxonomyMergeUtils() throws Exception
void TestOrdinalMappingLeafReader::testTaxonomyMergeUtils() 
{
  shared_ptr<Directory> srcIndexDir = newDirectory();
  shared_ptr<Directory> srcTaxoDir = newDirectory();
  buildIndexWithFacets(srcIndexDir, srcTaxoDir, true);

  shared_ptr<Directory> targetIndexDir = newDirectory();
  shared_ptr<Directory> targetTaxoDir = newDirectory();
  buildIndexWithFacets(targetIndexDir, targetTaxoDir, false);

  shared_ptr<IndexWriter> destIndexWriter =
      make_shared<IndexWriter>(targetIndexDir, newIndexWriterConfig(nullptr));
  shared_ptr<DirectoryTaxonomyWriter> destTaxoWriter =
      make_shared<DirectoryTaxonomyWriter>(targetTaxoDir);
  try {
    TaxonomyMergeUtils::merge(srcIndexDir, srcTaxoDir,
                              make_shared<MemoryOrdinalMap>(), destIndexWriter,
                              destTaxoWriter, facetConfig);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({destIndexWriter, destTaxoWriter});
  }
  verifyResults(targetIndexDir, targetTaxoDir);

  IOUtils::close({targetIndexDir, targetTaxoDir, srcIndexDir, srcTaxoDir});
}

void TestOrdinalMappingLeafReader::verifyResults(
    shared_ptr<Directory> indexDir,
    shared_ptr<Directory> taxoDir) 
{
  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoDir);
  shared_ptr<IndexSearcher> searcher = newSearcher(indexReader);

  shared_ptr<FacetsCollector> collector = make_shared<FacetsCollector>();
  FacetsCollector::search(searcher, make_shared<MatchAllDocsQuery>(), 10,
                          collector);

  // tag facets
  shared_ptr<Facets> tagFacets = make_shared<FastTaxonomyFacetCounts>(
      L"$tags", taxoReader, facetConfig, collector);
  shared_ptr<FacetResult> result = tagFacets->getTopChildren(10, L"tag");
  for (auto lv : result->labelValues) {
    if (VERBOSE) {
      wcout << lv << endl;
    }
    assertEquals(NUM_DOCS, lv->value->intValue());
  }

  // id facets
  shared_ptr<Facets> idFacets =
      make_shared<FastTaxonomyFacetCounts>(taxoReader, facetConfig, collector);
  shared_ptr<FacetResult> idResult = idFacets->getTopChildren(10, L"id");
  assertEquals(NUM_DOCS, idResult->childCount);
  assertEquals(NUM_DOCS * 2, idResult->value); // each "id" appears twice

  shared_ptr<BinaryDocValues> bdv =
      MultiDocValues::getBinaryValues(indexReader, L"bdv");
  shared_ptr<BinaryDocValues> cbdv =
      MultiDocValues::getBinaryValues(indexReader, L"cbdv");
  for (int i = 0; i < indexReader->maxDoc(); i++) {
    assertEquals(i, bdv->nextDoc());
    assertEquals(i, cbdv->nextDoc());
    assertEquals(stoi(cbdv->binaryValue()->utf8ToString()),
                 stoi(bdv->binaryValue()->utf8ToString()) * 2);
  }
  IOUtils::close({indexReader, taxoReader});
}

void TestOrdinalMappingLeafReader::buildIndexWithFacets(
    shared_ptr<Directory> indexDir, shared_ptr<Directory> taxoDir,
    bool asc) 
{
  shared_ptr<IndexWriterConfig> config = newIndexWriterConfig(nullptr);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), indexDir, config);

  shared_ptr<DirectoryTaxonomyWriter> taxonomyWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  for (int i = 1; i <= NUM_DOCS; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    for (int j = i; j <= NUM_DOCS; j++) {
      int facetValue = asc ? j : NUM_DOCS - j;
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(
          make_shared<FacetField>(L"tag", Integer::toString(facetValue)));
    }
    // add a facet under default dim config
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<FacetField>(L"id", Integer::toString(i)));

    // make sure OrdinalMappingLeafReader ignores non-facet BinaryDocValues
    // fields
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<BinaryDocValuesField>(
        L"bdv", make_shared<BytesRef>(Integer::toString(i))));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<BinaryDocValuesField>(
        L"cbdv", make_shared<BytesRef>(Integer::toString(i * 2))));
    writer->addDocument(facetConfig->build(taxonomyWriter, doc));
  }
  taxonomyWriter->commit();
  delete taxonomyWriter;
  writer->commit();
  delete writer;
}
} // namespace org::apache::lucene::facet::taxonomy