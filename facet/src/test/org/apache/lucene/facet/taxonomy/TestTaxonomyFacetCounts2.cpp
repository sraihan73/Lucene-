using namespace std;

#include "TestTaxonomyFacetCounts2.h"

namespace org::apache::lucene::facet::taxonomy
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using StringField = org::apache::lucene::document::StringField;
using FacetField = org::apache::lucene::facet::FacetField;
using FacetResult = org::apache::lucene::facet::FacetResult;
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using LabelAndValue = org::apache::lucene::facet::LabelAndValue;
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::junit::AfterClass;
using org::junit::BeforeClass;
using org::junit::Test;
const shared_ptr<org::apache::lucene::index::Term> TestTaxonomyFacetCounts2::A =
    make_shared<org::apache::lucene::index::Term>(L"f", L"a");
const wstring CP_A, TestTaxonomyFacetCounts2::CP_B = L"B";
const wstring CP_C, TestTaxonomyFacetCounts2::CP_D = L"D";
std::deque<std::shared_ptr<org::apache::lucene::facet::FacetField>> const
    CATEGORIES_A,
    TestTaxonomyFacetCounts2::CATEGORIES_B;
std::deque<std::shared_ptr<org::apache::lucene::facet::FacetField>> const
    CATEGORIES_C,
    TestTaxonomyFacetCounts2::CATEGORIES_D;

TestTaxonomyFacetCounts2::StaticConstructor::StaticConstructor()
{
  CATEGORIES_A = std::deque<std::shared_ptr<FacetField>>(NUM_CHILDREN_CP_A);
  for (int i = 0; i < NUM_CHILDREN_CP_A; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    CATEGORIES_A[i] = make_shared<FacetField>(CP_A, Integer::toString(i));
  }
  CATEGORIES_B = std::deque<std::shared_ptr<FacetField>>(NUM_CHILDREN_CP_B);
  for (int i = 0; i < NUM_CHILDREN_CP_B; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    CATEGORIES_B[i] = make_shared<FacetField>(CP_B, Integer::toString(i));
  }

  // NO_PARENTS categories
  CATEGORIES_C = std::deque<std::shared_ptr<FacetField>>(NUM_CHILDREN_CP_C);
  for (int i = 0; i < NUM_CHILDREN_CP_C; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    CATEGORIES_C[i] = make_shared<FacetField>(CP_C, Integer::toString(i));
  }

  // Multi-level categories
  CATEGORIES_D = std::deque<std::shared_ptr<FacetField>>(NUM_CHILDREN_CP_D);
  for (int i = 0; i < NUM_CHILDREN_CP_D; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring val = Integer::toString(i);
    CATEGORIES_D[i] =
        make_shared<FacetField>(CP_D, val, val + val); // e.g. D/1/11, D/2/22...
  }
}

TestTaxonomyFacetCounts2::StaticConstructor
    TestTaxonomyFacetCounts2::staticConstructor;
shared_ptr<org::apache::lucene::store::Directory> indexDir,
    TestTaxonomyFacetCounts2::taxoDir;
unordered_map<wstring, int> allExpectedCounts,
    TestTaxonomyFacetCounts2::termExpectedCounts;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void
// afterClassCountingFacetsAggregatorTest() throws Exception
void TestTaxonomyFacetCounts2::afterClassCountingFacetsAggregatorTest() throw(
    runtime_error)
{
  IOUtils::close({indexDir, taxoDir});
  indexDir = taxoDir = nullptr;
}

deque<std::shared_ptr<FacetField>>
TestTaxonomyFacetCounts2::randomCategories(shared_ptr<Random> random)
{
  // add random categories from the two dimensions, ensuring that the same
  // category is not added twice.
  int numFacetsA = random->nextInt(3) + 1; // 1-3
  int numFacetsB = random->nextInt(2) + 1; // 1-2
  deque<std::shared_ptr<FacetField>> categories_a =
      deque<std::shared_ptr<FacetField>>();
  categories_a.addAll(Arrays::asList(CATEGORIES_A));
  deque<std::shared_ptr<FacetField>> categories_b =
      deque<std::shared_ptr<FacetField>>();
  categories_b.addAll(Arrays::asList(CATEGORIES_B));
  Collections::shuffle(categories_a, random);
  Collections::shuffle(categories_b, random);

  deque<std::shared_ptr<FacetField>> categories =
      deque<std::shared_ptr<FacetField>>();
  categories.addAll(categories_a.subList(0, numFacetsA));
  categories.addAll(categories_b.subList(0, numFacetsB));

  // add the NO_PARENT categories
  categories.push_back(CATEGORIES_C[TestTaxonomyFacetCounts2::random()->nextInt(
      NUM_CHILDREN_CP_C)]);
  categories.push_back(CATEGORIES_D[TestTaxonomyFacetCounts2::random()->nextInt(
      NUM_CHILDREN_CP_D)]);

  return categories;
}

void TestTaxonomyFacetCounts2::addField(shared_ptr<Document> doc)
{
  doc->push_back(make_shared<StringField>(A->field(), A->text(), Store::NO));
}

void TestTaxonomyFacetCounts2::addFacets(
    shared_ptr<Document> doc, shared_ptr<FacetsConfig> config,
    bool updateTermExpectedCounts) 
{
  deque<std::shared_ptr<FacetField>> docCategories =
      randomCategories(random());
  for (auto ff : docCategories) {
    doc->push_back(ff);
    wstring cp = ff->dim + L"/" + ff->path[0];
    allExpectedCounts.emplace(cp, allExpectedCounts[cp] + 1);
    if (updateTermExpectedCounts) {
      termExpectedCounts.emplace(cp, termExpectedCounts[cp] + 1);
    }
  }
  // add 1 to each NO_PARENTS dimension
  allExpectedCounts.emplace(CP_B, allExpectedCounts[CP_B] + 1);
  allExpectedCounts.emplace(CP_C, allExpectedCounts[CP_C] + 1);
  allExpectedCounts.emplace(CP_D, allExpectedCounts[CP_D] + 1);
  if (updateTermExpectedCounts) {
    termExpectedCounts.emplace(CP_B, termExpectedCounts[CP_B] + 1);
    termExpectedCounts.emplace(CP_C, termExpectedCounts[CP_C] + 1);
    termExpectedCounts.emplace(CP_D, termExpectedCounts[CP_D] + 1);
  }
}

shared_ptr<FacetsConfig> TestTaxonomyFacetCounts2::getConfig()
{
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setMultiValued(L"A", true);
  config->setMultiValued(L"B", true);
  config->setRequireDimCount(L"B", true);
  config->setHierarchical(L"D", true);
  return config;
}

void TestTaxonomyFacetCounts2::indexDocsNoFacets(
    shared_ptr<IndexWriter> indexWriter) 
{
  int numDocs = atLeast(2);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    addField(doc);
    indexWriter->addDocument(doc);
  }
  indexWriter->commit(); // flush a segment
}

void TestTaxonomyFacetCounts2::indexDocsWithFacetsNoTerms(
    shared_ptr<IndexWriter> indexWriter, shared_ptr<TaxonomyWriter> taxoWriter,
    unordered_map<wstring, int> &expectedCounts) 
{
  shared_ptr<Random> random = TestTaxonomyFacetCounts2::random();
  int numDocs = atLeast(random, 2);
  shared_ptr<FacetsConfig> config = getConfig();
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    addFacets(doc, config, false);
    indexWriter->addDocument(config->build(taxoWriter, doc));
  }
  indexWriter->commit(); // flush a segment
}

void TestTaxonomyFacetCounts2::indexDocsWithFacetsAndTerms(
    shared_ptr<IndexWriter> indexWriter, shared_ptr<TaxonomyWriter> taxoWriter,
    unordered_map<wstring, int> &expectedCounts) 
{
  shared_ptr<Random> random = TestTaxonomyFacetCounts2::random();
  int numDocs = atLeast(random, 2);
  shared_ptr<FacetsConfig> config = getConfig();
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    addFacets(doc, config, true);
    addField(doc);
    indexWriter->addDocument(config->build(taxoWriter, doc));
  }
  indexWriter->commit(); // flush a segment
}

void TestTaxonomyFacetCounts2::indexDocsWithFacetsAndSomeTerms(
    shared_ptr<IndexWriter> indexWriter, shared_ptr<TaxonomyWriter> taxoWriter,
    unordered_map<wstring, int> &expectedCounts) 
{
  shared_ptr<Random> random = TestTaxonomyFacetCounts2::random();
  int numDocs = atLeast(random, 2);
  shared_ptr<FacetsConfig> config = getConfig();
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    bool hasContent = random->nextBoolean();
    if (hasContent) {
      addField(doc);
    }
    addFacets(doc, config, hasContent);
    indexWriter->addDocument(config->build(taxoWriter, doc));
  }
  indexWriter->commit(); // flush a segment
}

unordered_map<wstring, int> TestTaxonomyFacetCounts2::newCounts()
{
  unordered_map<wstring, int> counts = unordered_map<wstring, int>();
  counts.emplace(CP_A, 0);
  counts.emplace(CP_B, 0);
  counts.emplace(CP_C, 0);
  counts.emplace(CP_D, 0);
  for (auto ff : CATEGORIES_A) {
    counts.emplace(ff->dim + L"/" + ff->path[0], 0);
  }
  for (auto ff : CATEGORIES_B) {
    counts.emplace(ff->dim + L"/" + ff->path[0], 0);
  }
  for (auto ff : CATEGORIES_C) {
    counts.emplace(ff->dim + L"/" + ff->path[0], 0);
  }
  for (auto ff : CATEGORIES_D) {
    counts.emplace(ff->dim + L"/" + ff->path[0], 0);
  }
  return counts;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void
// beforeClassCountingFacetsAggregatorTest() throws Exception
void TestTaxonomyFacetCounts2::beforeClassCountingFacetsAggregatorTest() throw(
    runtime_error)
{
  indexDir = newDirectory();
  taxoDir = newDirectory();

  // create an index which has:
  // 1. Segment with no categories, but matching results
  // 2. Segment w/ categories, but no results
  // 3. Segment w/ categories and results
  // 4. Segment w/ categories, but only some results

  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setMergePolicy(NoMergePolicy::INSTANCE); // prevent merges, so we can
                                                 // control the index segments
  shared_ptr<IndexWriter> indexWriter =
      make_shared<IndexWriter>(indexDir, conf);
  shared_ptr<TaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);

  allExpectedCounts = newCounts();
  termExpectedCounts = newCounts();

  // segment w/ no categories
  indexDocsNoFacets(indexWriter);

  // segment w/ categories, no content
  indexDocsWithFacetsNoTerms(indexWriter, taxoWriter, allExpectedCounts);

  // segment w/ categories and content
  indexDocsWithFacetsAndTerms(indexWriter, taxoWriter, allExpectedCounts);

  // segment w/ categories and some content
  indexDocsWithFacetsAndSomeTerms(indexWriter, taxoWriter, allExpectedCounts);

  delete indexWriter;
  IOUtils::close({taxoWriter});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDifferentNumResults() throws Exception
void TestTaxonomyFacetCounts2::testDifferentNumResults() 
{
  // test the collector w/ FacetRequests and different numResults
  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoDir);
  shared_ptr<IndexSearcher> searcher = newSearcher(indexReader);

  shared_ptr<FacetsCollector> sfc = make_shared<FacetsCollector>();
  shared_ptr<TermQuery> q = make_shared<TermQuery>(A);
  searcher->search(q, sfc);
  shared_ptr<Facets> facets =
      getTaxonomyFacetCounts(taxoReader, getConfig(), sfc);
  shared_ptr<FacetResult> result =
      facets->getTopChildren(NUM_CHILDREN_CP_A, CP_A);
  assertEquals(-1, result->value->intValue());
  for (auto labelValue : result->labelValues) {
    assertEquals(termExpectedCounts[CP_A + L"/" + labelValue->label],
                 labelValue->value);
  }
  result = facets->getTopChildren(NUM_CHILDREN_CP_B, CP_B);
  assertEquals(termExpectedCounts[CP_B], result->value);
  for (auto labelValue : result->labelValues) {
    assertEquals(termExpectedCounts[CP_B + L"/" + labelValue->label],
                 labelValue->value);
  }

  IOUtils::close({indexReader, taxoReader});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAllCounts() throws Exception
void TestTaxonomyFacetCounts2::testAllCounts() 
{
  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoDir);
  shared_ptr<IndexSearcher> searcher = newSearcher(indexReader);

  shared_ptr<FacetsCollector> sfc = make_shared<FacetsCollector>();
  searcher->search(make_shared<MatchAllDocsQuery>(), sfc);

  shared_ptr<Facets> facets =
      getTaxonomyFacetCounts(taxoReader, getConfig(), sfc);

  shared_ptr<FacetResult> result =
      facets->getTopChildren(NUM_CHILDREN_CP_A, CP_A);
  assertEquals(-1, result->value->intValue());
  int prevValue = numeric_limits<int>::max();
  for (auto labelValue : result->labelValues) {
    assertEquals(allExpectedCounts[CP_A + L"/" + labelValue->label],
                 labelValue->value);
    assertTrue(L"wrong sort order of sub results: labelValue.value=" +
                   labelValue->value + L" prevValue=" + to_wstring(prevValue),
               labelValue->value->intValue() <= prevValue);
    prevValue = labelValue->value->intValue();
  }

  result = facets->getTopChildren(NUM_CHILDREN_CP_B, CP_B);
  assertEquals(allExpectedCounts[CP_B], result->value);
  prevValue = numeric_limits<int>::max();
  for (auto labelValue : result->labelValues) {
    assertEquals(allExpectedCounts[CP_B + L"/" + labelValue->label],
                 labelValue->value);
    assertTrue(L"wrong sort order of sub results: labelValue.value=" +
                   labelValue->value + L" prevValue=" + to_wstring(prevValue),
               labelValue->value->intValue() <= prevValue);
    prevValue = labelValue->value->intValue();
  }

  IOUtils::close({indexReader, taxoReader});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBigNumResults() throws Exception
void TestTaxonomyFacetCounts2::testBigNumResults() 
{
  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoDir);
  shared_ptr<IndexSearcher> searcher = newSearcher(indexReader);

  shared_ptr<FacetsCollector> sfc = make_shared<FacetsCollector>();
  searcher->search(make_shared<MatchAllDocsQuery>(), sfc);

  shared_ptr<Facets> facets =
      getTaxonomyFacetCounts(taxoReader, getConfig(), sfc);

  shared_ptr<FacetResult> result =
      facets->getTopChildren(numeric_limits<int>::max(), CP_A);
  assertEquals(-1, result->value->intValue());
  for (auto labelValue : result->labelValues) {
    assertEquals(allExpectedCounts[CP_A + L"/" + labelValue->label],
                 labelValue->value);
  }
  result = facets->getTopChildren(numeric_limits<int>::max(), CP_B);
  assertEquals(allExpectedCounts[CP_B], result->value);
  for (auto labelValue : result->labelValues) {
    assertEquals(allExpectedCounts[CP_B + L"/" + labelValue->label],
                 labelValue->value);
  }

  IOUtils::close({indexReader, taxoReader});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNoParents() throws Exception
void TestTaxonomyFacetCounts2::testNoParents() 
{
  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoDir);
  shared_ptr<IndexSearcher> searcher = newSearcher(indexReader);

  shared_ptr<FacetsCollector> sfc = make_shared<FacetsCollector>();
  searcher->search(make_shared<MatchAllDocsQuery>(), sfc);

  shared_ptr<Facets> facets =
      getTaxonomyFacetCounts(taxoReader, getConfig(), sfc);

  shared_ptr<FacetResult> result =
      facets->getTopChildren(NUM_CHILDREN_CP_C, CP_C);
  assertEquals(allExpectedCounts[CP_C], result->value);
  for (auto labelValue : result->labelValues) {
    assertEquals(allExpectedCounts[CP_C + L"/" + labelValue->label],
                 labelValue->value);
  }
  result = facets->getTopChildren(NUM_CHILDREN_CP_D, CP_D);
  assertEquals(allExpectedCounts[CP_C], result->value);
  for (auto labelValue : result->labelValues) {
    assertEquals(allExpectedCounts[CP_D + L"/" + labelValue->label],
                 labelValue->value);
  }

  IOUtils::close({indexReader, taxoReader});
}
} // namespace org::apache::lucene::facet::taxonomy