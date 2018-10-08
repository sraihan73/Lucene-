using namespace std;

#include "TestTaxonomyFacetAssociations.h"

namespace org::apache::lucene::facet::taxonomy
{
using Document = org::apache::lucene::document::Document;
using DrillDownQuery = org::apache::lucene::facet::DrillDownQuery;
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::store::Directory>
    TestTaxonomyFacetAssociations::dir;
shared_ptr<org::apache::lucene::index::IndexReader>
    TestTaxonomyFacetAssociations::reader;
shared_ptr<org::apache::lucene::store::Directory>
    TestTaxonomyFacetAssociations::taxoDir;
shared_ptr<TaxonomyReader> TestTaxonomyFacetAssociations::taxoReader;
shared_ptr<org::apache::lucene::facet::FacetsConfig>
    TestTaxonomyFacetAssociations::config;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestTaxonomyFacetAssociations::beforeClass() 
{
  dir = newDirectory();
  taxoDir = newDirectory();
  // preparations - index, taxonomy, content

  shared_ptr<TaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);

  // Cannot mix ints & floats in the same indexed field:
  config = make_shared<FacetsConfig>();
  config->setIndexFieldName(L"int", L"$facets.int");
  config->setMultiValued(L"int", true);
  config->setIndexFieldName(L"float", L"$facets.float");
  config->setMultiValued(L"float", true);

  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // index documents, 50% have only 'b' and all have 'a'
  for (int i = 0; i < 110; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // every 11th document is added empty, this used to cause the association
    // aggregators to go into an infinite loop
    if (i % 11 != 0) {
      doc->push_back(make_shared<IntAssociationFacetField>(2, L"int", L"a"));
      doc->push_back(
          make_shared<FloatAssociationFacetField>(0.5f, L"float", L"a"));
      if (i % 2 == 0) { // 50
        doc->push_back(make_shared<IntAssociationFacetField>(3, L"int", L"b"));
        doc->push_back(
            make_shared<FloatAssociationFacetField>(0.2f, L"float", L"b"));
      }
    }
    writer->addDocument(config->build(taxoWriter, doc));
  }

  delete taxoWriter;
  reader = writer->getReader();
  delete writer;
  taxoReader = make_shared<DirectoryTaxonomyReader>(taxoDir);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestTaxonomyFacetAssociations::afterClass() 
{
  delete reader;
  reader.reset();
  delete dir;
  dir.reset();
  delete taxoReader;
  taxoReader.reset();
  delete taxoDir;
  taxoDir.reset();
}

void TestTaxonomyFacetAssociations::testIntSumAssociation() 
{

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  searcher->search(make_shared<MatchAllDocsQuery>(), fc);

  shared_ptr<Facets> facets = make_shared<TaxonomyFacetSumIntAssociations>(
      L"$facets.int", taxoReader, config, fc);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"dim=int path=[] value=-1 childCount=2\n  a (200)\n  b (150)\n",
               facets->getTopChildren(10, L"int")->toString());
  assertEquals(L"Wrong count for category 'a'!", 200,
               facets->getSpecificValue(L"int", {L"a"})->intValue());
  assertEquals(L"Wrong count for category 'b'!", 150,
               facets->getSpecificValue(L"int", {L"b"})->intValue());
}

void TestTaxonomyFacetAssociations::testFloatSumAssociation() throw(
    runtime_error)
{
  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  searcher->search(make_shared<MatchAllDocsQuery>(), fc);

  shared_ptr<Facets> facets = make_shared<TaxonomyFacetSumFloatAssociations>(
      L"$facets.float", taxoReader, config, fc);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"dim=float path=[] value=-1.0 childCount=2\n  a (50.0)\n  b "
               L"(9.999995)\n",
               facets->getTopChildren(10, L"float")->toString());
  assertEquals(L"Wrong count for category 'a'!", 50.0f,
               facets->getSpecificValue(L"float", {L"a"})->floatValue(),
               0.00001);
  assertEquals(L"Wrong count for category 'b'!", 10.0f,
               facets->getSpecificValue(L"float", {L"b"})->floatValue(),
               0.00001);
}

void TestTaxonomyFacetAssociations::testIntAndFloatAssocation() throw(
    runtime_error)
{
  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  searcher->search(make_shared<MatchAllDocsQuery>(), fc);

  shared_ptr<Facets> facets = make_shared<TaxonomyFacetSumFloatAssociations>(
      L"$facets.float", taxoReader, config, fc);
  assertEquals(L"Wrong count for category 'a'!", 50.0f,
               facets->getSpecificValue(L"float", {L"a"})->floatValue(),
               0.00001);
  assertEquals(L"Wrong count for category 'b'!", 10.0f,
               facets->getSpecificValue(L"float", {L"b"})->floatValue(),
               0.00001);

  facets = make_shared<TaxonomyFacetSumIntAssociations>(L"$facets.int",
                                                        taxoReader, config, fc);
  assertEquals(L"Wrong count for category 'a'!", 200,
               facets->getSpecificValue(L"int", {L"a"})->intValue());
  assertEquals(L"Wrong count for category 'b'!", 150,
               facets->getSpecificValue(L"int", {L"b"})->intValue());
}

void TestTaxonomyFacetAssociations::testWrongIndexFieldName() throw(
    runtime_error)
{
  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  searcher->search(make_shared<MatchAllDocsQuery>(), fc);
  shared_ptr<Facets> facets =
      make_shared<TaxonomyFacetSumFloatAssociations>(taxoReader, config, fc);
  expectThrows(invalid_argument::typeid,
               [&]() { facets->getSpecificValue(L"float"); });

  expectThrows(invalid_argument::typeid,
               [&]() { facets->getTopChildren(10, L"float"); });
}

void TestTaxonomyFacetAssociations::testMixedTypesInSameIndexField() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<TaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntAssociationFacetField>(14, L"a", L"x"));
  doc->push_back(make_shared<FloatAssociationFacetField>(55.0f, L"b", L"y"));
  expectThrows(invalid_argument::typeid,
               [&]() { writer->addDocument(config->build(taxoWriter, doc)); });
  delete writer;
  IOUtils::close({taxoWriter, dir, taxoDir});
}

void TestTaxonomyFacetAssociations::testNoHierarchy() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<TaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setHierarchical(L"a", true);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntAssociationFacetField>(14, L"a", L"x"));
  expectThrows(invalid_argument::typeid,
               [&]() { writer->addDocument(config->build(taxoWriter, doc)); });

  delete writer;
  IOUtils::close({taxoWriter, dir, taxoDir});
}

void TestTaxonomyFacetAssociations::testRequireDimCount() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<TaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setRequireDimCount(L"a", true);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntAssociationFacetField>(14, L"a", L"x"));
  expectThrows(invalid_argument::typeid,
               [&]() { writer->addDocument(config->build(taxoWriter, doc)); });

  delete writer;
  IOUtils::close({taxoWriter, dir, taxoDir});
}

void TestTaxonomyFacetAssociations::testIntSumAssociationDrillDown() throw(
    runtime_error)
{
  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<DrillDownQuery> q = make_shared<DrillDownQuery>(config);
  q->add(L"int", {L"b"});
  searcher->search(q, fc);

  shared_ptr<Facets> facets = make_shared<TaxonomyFacetSumIntAssociations>(
      L"$facets.int", taxoReader, config, fc);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"dim=int path=[] value=-1 childCount=2\n  b (150)\n  a (100)\n",
               facets->getTopChildren(10, L"int")->toString());
  assertEquals(L"Wrong count for category 'a'!", 100,
               facets->getSpecificValue(L"int", {L"a"})->intValue());
  assertEquals(L"Wrong count for category 'b'!", 150,
               facets->getSpecificValue(L"int", {L"b"})->intValue());
}
} // namespace org::apache::lucene::facet::taxonomy