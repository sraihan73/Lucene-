using namespace std;

#include "TestDrillSideways.h"

namespace org::apache::lucene::facet
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using DrillSidewaysResult =
    org::apache::lucene::facet::DrillSideways::DrillSidewaysResult;
using DefaultSortedSetDocValuesReaderState =
    org::apache::lucene::facet::sortedset::DefaultSortedSetDocValuesReaderState;
using SortedSetDocValuesFacetField =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesFacetField;
using SortedSetDocValuesReaderState =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesReaderState;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using ConstantScoreScorer = org::apache::lucene::search::ConstantScoreScorer;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using InPlaceMergeSorter = org::apache::lucene::util::InPlaceMergeSorter;
using InfoStream = org::apache::lucene::util::InfoStream;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<DrillSideways> TestDrillSideways::getNewDrillSideways(
    shared_ptr<IndexSearcher> searcher, shared_ptr<FacetsConfig> config,
    shared_ptr<SortedSetDocValuesReaderState> state)
{
  return make_shared<DrillSideways>(searcher, config, state);
}

shared_ptr<DrillSideways>
TestDrillSideways::getNewDrillSideways(shared_ptr<IndexSearcher> searcher,
                                       shared_ptr<FacetsConfig> config,
                                       shared_ptr<TaxonomyReader> taxoReader)
{
  return make_shared<DrillSideways>(searcher, config, taxoReader);
}

shared_ptr<DrillSideways>
TestDrillSideways::getNewDrillSidewaysScoreSubdocsAtOnce(
    shared_ptr<IndexSearcher> searcher, shared_ptr<FacetsConfig> config,
    shared_ptr<TaxonomyReader> taxoReader)
{
  return make_shared<DrillSidewaysAnonymousInnerClass>(
      shared_from_this(), searcher, config, taxoReader);
}

TestDrillSideways::DrillSidewaysAnonymousInnerClass::
    DrillSidewaysAnonymousInnerClass(
        shared_ptr<TestDrillSideways> outerInstance,
        shared_ptr<IndexSearcher> searcher,
        shared_ptr<org::apache::lucene::facet::FacetsConfig> config,
        shared_ptr<TaxonomyReader> taxoReader)
    : DrillSideways(searcher, config, taxoReader)
{
  this->outerInstance = outerInstance;
}

bool TestDrillSideways::DrillSidewaysAnonymousInnerClass::scoreSubDocsAtOnce()
{
  return true;
}

shared_ptr<DrillSideways>
TestDrillSideways::getNewDrillSidewaysBuildFacetsResult(
    shared_ptr<IndexSearcher> searcher, shared_ptr<FacetsConfig> config,
    shared_ptr<TaxonomyReader> taxoReader)
{
  return make_shared<DrillSidewaysAnonymousInnerClass2>(
      shared_from_this(), searcher, config, taxoReader);
}

TestDrillSideways::DrillSidewaysAnonymousInnerClass2::
    DrillSidewaysAnonymousInnerClass2(
        shared_ptr<TestDrillSideways> outerInstance,
        shared_ptr<IndexSearcher> searcher,
        shared_ptr<org::apache::lucene::facet::FacetsConfig> config,
        shared_ptr<TaxonomyReader> taxoReader)
    : DrillSideways(searcher, config, taxoReader)
{
  this->outerInstance = outerInstance;
  this->config = config;
  this->taxoReader = taxoReader;
}

shared_ptr<Facets>
TestDrillSideways::DrillSidewaysAnonymousInnerClass2::buildFacetsResult(
    shared_ptr<FacetsCollector> drillDowns,
    std::deque<std::shared_ptr<FacetsCollector>> &drillSideways,
    std::deque<wstring> &drillSidewaysDims) 
{
  unordered_map<wstring, std::shared_ptr<Facets>> drillSidewaysFacets =
      unordered_map<wstring, std::shared_ptr<Facets>>();
  shared_ptr<Facets> drillDownFacets =
      outerInstance->getTaxonomyFacetCounts(taxoReader, config, drillDowns);
  if (drillSideways.size() > 0) {
    for (int i = 0; i < drillSideways.size(); i++) {
      drillSidewaysFacets.emplace(drillSidewaysDims[i],
                                  outerInstance->getTaxonomyFacetCounts(
                                      taxoReader, config, drillSideways[i]));
    }
  }

  if (drillSidewaysFacets.empty()) {
    return drillDownFacets;
  } else {
    return make_shared<MultiFacets>(drillSidewaysFacets, drillDownFacets);
  }
}

void TestDrillSideways::testBasic() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  // Writes facet ords to a separate directory from the
  // main index:
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir,
                                           IndexWriterConfig::OpenMode::CREATE);

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setHierarchical(L"Publish Date", true);

  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Bob"));
  doc->push_back(
      make_shared<FacetField>(L"Publish Date", L"2010", L"10", L"15"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Lisa"));
  doc->push_back(
      make_shared<FacetField>(L"Publish Date", L"2010", L"10", L"20"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Lisa"));
  doc->push_back(make_shared<FacetField>(L"Publish Date", L"2012", L"1", L"1"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Susan"));
  doc->push_back(make_shared<FacetField>(L"Publish Date", L"2012", L"1", L"7"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Frank"));
  doc->push_back(make_shared<FacetField>(L"Publish Date", L"1999", L"5", L"5"));
  writer->addDocument(config->build(taxoWriter, doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());

  // System.out.println("searcher=" + searcher);

  // NRT open
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<DrillSideways> ds =
      getNewDrillSideways(searcher, config, taxoReader);

  //  case: drill-down on a single field; in this
  // case the drill-sideways + drill-down counts ==
  // drill-down of just the query:
  shared_ptr<DrillDownQuery> ddq = make_shared<DrillDownQuery>(config);
  ddq->add(L"Author", {L"Lisa"});
  shared_ptr<DrillSidewaysResult> r = ds->search(nullptr, ddq, 10);
  TestUtil::assertEquals(2, r->hits->totalHits);
  // Publish Date is only drill-down, and Lisa published
  // one in 2012 and one in 2010:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=Publish Date path=[] value=2 childCount=2\n  2010 (1)\n  2012 "
      L"(1)\n",
      r->facets->getTopChildren(10, L"Publish Date")->toString());

  // Author is drill-sideways + drill-down: Lisa
  // (drill-down) published twice, and Frank/Susan/Bob
  // published once:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=Author path=[] value=5 childCount=4\n  Lisa "
                         L"(2)\n  Bob (1)\n  Susan (1)\n  Frank (1)\n",
                         r->facets->getTopChildren(10, L"Author")->toString());

  // Same simple case, but no baseQuery (pure browse):
  // drill-down on a single field; in this case the
  // drill-sideways + drill-down counts == drill-down of
  // just the query:
  ddq = make_shared<DrillDownQuery>(config);
  ddq->add(L"Author", {L"Lisa"});
  r = ds->search(nullptr, ddq, 10);

  TestUtil::assertEquals(2, r->hits->totalHits);
  // Publish Date is only drill-down, and Lisa published
  // one in 2012 and one in 2010:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=Publish Date path=[] value=2 childCount=2\n  2010 (1)\n  2012 "
      L"(1)\n",
      r->facets->getTopChildren(10, L"Publish Date")->toString());

  // Author is drill-sideways + drill-down: Lisa
  // (drill-down) published twice, and Frank/Susan/Bob
  // published once:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=Author path=[] value=5 childCount=4\n  Lisa "
                         L"(2)\n  Bob (1)\n  Susan (1)\n  Frank (1)\n",
                         r->facets->getTopChildren(10, L"Author")->toString());

  // Another simple case: drill-down on single fields
  // but OR of two values
  ddq = make_shared<DrillDownQuery>(config);
  ddq->add(L"Author", {L"Lisa"});
  ddq->add(L"Author", {L"Bob"});
  r = ds->search(nullptr, ddq, 10);
  TestUtil::assertEquals(3, r->hits->totalHits);
  // Publish Date is only drill-down: Lisa and Bob
  // (drill-down) published twice in 2010 and once in 2012:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=Publish Date path=[] value=3 childCount=2\n  2010 (2)\n  2012 "
      L"(1)\n",
      r->facets->getTopChildren(10, L"Publish Date")->toString());
  // Author is drill-sideways + drill-down: Lisa
  // (drill-down) published twice, and Frank/Susan/Bob
  // published once:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=Author path=[] value=5 childCount=4\n  Lisa "
                         L"(2)\n  Bob (1)\n  Susan (1)\n  Frank (1)\n",
                         r->facets->getTopChildren(10, L"Author")->toString());

  assertTrue(std::dynamic_pointer_cast<MultiFacets>(r->facets) != nullptr);
  deque<std::shared_ptr<FacetResult>> allResults = r->facets->getAllDims(10);
  TestUtil::assertEquals(2, allResults.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=Author path=[] value=5 childCount=4\n  Lisa "
                         L"(2)\n  Bob (1)\n  Susan (1)\n  Frank (1)\n",
                         allResults[0]->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=Publish Date path=[] value=3 childCount=2\n  "
                         L"2010 (2)\n  2012 (1)\n",
                         allResults[1]->toString());

  // More interesting case: drill-down on two fields
  ddq = make_shared<DrillDownQuery>(config);
  ddq->add(L"Author", {L"Lisa"});
  ddq->add(L"Publish Date", {L"2010"});
  r = ds->search(nullptr, ddq, 10);
  TestUtil::assertEquals(1, r->hits->totalHits);
  // Publish Date is drill-sideways + drill-down: Lisa
  // (drill-down) published once in 2010 and once in 2012:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=Publish Date path=[] value=2 childCount=2\n  2010 (1)\n  2012 "
      L"(1)\n",
      r->facets->getTopChildren(10, L"Publish Date")->toString());
  // Author is drill-sideways + drill-down:
  // only Lisa & Bob published (once each) in 2010:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=Author path=[] value=2 childCount=2\n  Bob (1)\n  Lisa (1)\n",
      r->facets->getTopChildren(10, L"Author")->toString());

  // Even more interesting case: drill down on two fields,
  // but one of them is OR
  ddq = make_shared<DrillDownQuery>(config);

  // Drill down on Lisa or Bob:
  ddq->add(L"Author", {L"Lisa"});
  ddq->add(L"Publish Date", {L"2010"});
  ddq->add(L"Author", {L"Bob"});
  r = ds->search(nullptr, ddq, 10);
  TestUtil::assertEquals(2, r->hits->totalHits);
  // Publish Date is both drill-sideways + drill-down:
  // Lisa or Bob published twice in 2010 and once in 2012:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=Publish Date path=[] value=3 childCount=2\n  2010 (2)\n  2012 "
      L"(1)\n",
      r->facets->getTopChildren(10, L"Publish Date")->toString());
  // Author is drill-sideways + drill-down:
  // only Lisa & Bob published (once each) in 2010:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=Author path=[] value=2 childCount=2\n  Bob (1)\n  Lisa (1)\n",
      r->facets->getTopChildren(10, L"Author")->toString());

  // Test drilling down on invalid field:
  ddq = make_shared<DrillDownQuery>(config);
  ddq->add(L"Foobar", {L"Baz"});
  r = ds->search(nullptr, ddq, 10);
  TestUtil::assertEquals(0, r->hits->totalHits);
  assertNull(r->facets->getTopChildren(10, L"Publish Date"));
  assertNull(r->facets->getTopChildren(10, L"Foobar"));

  // Test drilling down on valid term or'd with invalid term:
  ddq = make_shared<DrillDownQuery>(config);
  ddq->add(L"Author", {L"Lisa"});
  ddq->add(L"Author", {L"Tom"});
  r = ds->search(nullptr, ddq, 10);
  TestUtil::assertEquals(2, r->hits->totalHits);
  // Publish Date is only drill-down, and Lisa published
  // one in 2012 and one in 2010:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=Publish Date path=[] value=2 childCount=2\n  2010 (1)\n  2012 "
      L"(1)\n",
      r->facets->getTopChildren(10, L"Publish Date")->toString());
  // Author is drill-sideways + drill-down: Lisa
  // (drill-down) published twice, and Frank/Susan/Bob
  // published once:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=Author path=[] value=5 childCount=4\n  Lisa "
                         L"(2)\n  Bob (1)\n  Susan (1)\n  Frank (1)\n",
                         r->facets->getTopChildren(10, L"Author")->toString());

  // LUCENE-4915: test drilling down on a dimension but
  // NOT facet counting it:
  ddq = make_shared<DrillDownQuery>(config);
  ddq->add(L"Author", {L"Lisa"});
  ddq->add(L"Author", {L"Tom"});
  r = ds->search(nullptr, ddq, 10);
  TestUtil::assertEquals(2, r->hits->totalHits);
  // Publish Date is only drill-down, and Lisa published
  // one in 2012 and one in 2010:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=Publish Date path=[] value=2 childCount=2\n  2010 (1)\n  2012 "
      L"(1)\n",
      r->facets->getTopChildren(10, L"Publish Date")->toString());

  // Test main query gets null scorer:
  ddq = make_shared<DrillDownQuery>(
      config, make_shared<TermQuery>(make_shared<Term>(L"foobar", L"baz")));
  ddq->add(L"Author", {L"Lisa"});
  r = ds->search(nullptr, ddq, 10);

  TestUtil::assertEquals(0, r->hits->totalHits);
  assertNull(r->facets->getTopChildren(10, L"Publish Date"));
  assertNull(r->facets->getTopChildren(10, L"Author"));
  delete writer;
  IOUtils::close(
      {searcher->getIndexReader(), taxoReader, taxoWriter, dir, taxoDir});
}

void TestDrillSideways::testSometimesInvalidDrillDown() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // Writes facet ords to a separate directory from the
  // main index:
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir,
                                           IndexWriterConfig::OpenMode::CREATE);

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setHierarchical(L"Publish Date", true);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Bob"));
  doc->push_back(
      make_shared<FacetField>(L"Publish Date", L"2010", L"10", L"15"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Lisa"));
  doc->push_back(
      make_shared<FacetField>(L"Publish Date", L"2010", L"10", L"20"));
  writer->addDocument(config->build(taxoWriter, doc));

  writer->commit();

  // 2nd segment has no Author:
  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Foobar", L"Lisa"));
  doc->push_back(make_shared<FacetField>(L"Publish Date", L"2012", L"1", L"1"));
  writer->addDocument(config->build(taxoWriter, doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());

  // System.out.println("searcher=" + searcher);

  // NRT open
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<DrillDownQuery> ddq = make_shared<DrillDownQuery>(config);
  ddq->add(L"Author", {L"Lisa"});
  shared_ptr<DrillSidewaysResult> r =
      getNewDrillSideways(searcher, config, taxoReader)
          ->search(nullptr, ddq, 10);

  TestUtil::assertEquals(1, r->hits->totalHits);
  // Publish Date is only drill-down, and Lisa published
  // one in 2012 and one in 2010:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=Publish Date path=[] value=1 childCount=1\n  2010 (1)\n",
      r->facets->getTopChildren(10, L"Publish Date")->toString());
  // Author is drill-sideways + drill-down: Lisa
  // (drill-down) published once, and Bob
  // published once:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=Author path=[] value=2 childCount=2\n  Bob (1)\n  Lisa (1)\n",
      r->facets->getTopChildren(10, L"Author")->toString());

  delete writer;
  IOUtils::close(
      {searcher->getIndexReader(), taxoReader, taxoWriter, dir, taxoDir});
}

void TestDrillSideways::testMultipleRequestsPerDim() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // Writes facet ords to a separate directory from the
  // main index:
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir,
                                           IndexWriterConfig::OpenMode::CREATE);

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setHierarchical(L"dim", true);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"dim", L"a", L"x"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"dim", L"a", L"y"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"dim", L"a", L"z"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"dim", L"b"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"dim", L"c"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"dim", L"d"));
  writer->addDocument(config->build(taxoWriter, doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());

  // System.out.println("searcher=" + searcher);

  // NRT open
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<DrillDownQuery> ddq = make_shared<DrillDownQuery>(config);
  ddq->add(L"dim", {L"a"});
  shared_ptr<DrillSidewaysResult> r =
      getNewDrillSideways(searcher, config, taxoReader)
          ->search(nullptr, ddq, 10);

  TestUtil::assertEquals(3, r->hits->totalHits);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=dim path=[] value=6 childCount=4\n  a (3)\n  b "
                         L"(1)\n  c (1)\n  d (1)\n",
                         r->facets->getTopChildren(10, L"dim")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=dim path=[a] value=3 childCount=3\n  x (1)\n  y (1)\n  z (1)\n",
      r->facets->getTopChildren(10, L"dim", {L"a"})->toString());

  delete writer;
  IOUtils::close(
      {searcher->getIndexReader(), taxoReader, taxoWriter, dir, taxoDir});
}

TestDrillSideways::Doc::Doc() {}

int TestDrillSideways::Doc::compareTo(shared_ptr<Doc> other)
{
  return id.compare(other->id);
}

wstring TestDrillSideways::randomContentToken(bool isQuery)
{
  double d = random()->nextDouble();
  if (isQuery) {
    if (d < 0.33) {
      return L"a";
    } else if (d < 0.66) {
      return L"b";
    } else {
      return L"c";
    }
  } else {
    if (d <= aChance) {
      return L"a";
    } else if (d < aChance + bChance) {
      return L"b";
    } else {
      return L"c";
    }
  }
}

void TestDrillSideways::testRandom() 
{

  while (aChance == 0.0) {
    aChance = random()->nextDouble();
  }
  while (bChance == 0.0) {
    bChance = random()->nextDouble();
  }
  while (cChance == 0.0) {
    cChance = random()->nextDouble();
  }
  // aChance = .01;
  // bChance = 0.5;
  // cChance = 1.0;
  double sum = aChance + bChance + cChance;
  aChance /= sum;
  bChance /= sum;
  cChance /= sum;

  int numDims = TestUtil::nextInt(random(), 2, 5);
  // int numDims = 3;
  int numDocs = atLeast(3000);
  // int numDocs = 20;
  if (VERBOSE) {
    wcout << L"numDims=" << numDims << L" numDocs=" << numDocs << L" aChance="
          << aChance << L" bChance=" << bChance << L" cChance=" << cChance
          << endl;
  }
  std::deque<std::deque<wstring>> dimValues(numDims);
  int valueCount = 2;

  for (int dim = 0; dim < numDims; dim++) {
    shared_ptr<Set<wstring>> values = unordered_set<wstring>();
    while (values->size() < valueCount) {
      wstring s = TestUtil::randomRealisticUnicodeString(random());
      // std::wstring s = _TestUtil.randomString(random());
      if (s.length() > 0) {
        values->add(s);
      }
    }
    dimValues[dim] = values->toArray(std::deque<wstring>(values->size()));
    valueCount *= 2;
  }

  deque<std::shared_ptr<Doc>> docs = deque<std::shared_ptr<Doc>>();
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Doc> doc = make_shared<Doc>();
    doc->id = L"" + to_wstring(i);
    doc->contentToken = randomContentToken(false);
    doc->dims = std::deque<int>(numDims);
    doc->dims2 = std::deque<int>(numDims);
    for (int dim = 0; dim < numDims; dim++) {
      if (random()->nextInt(5) == 3) {
        // This doc is missing this dim:
        doc->dims[dim] = -1;
      } else if (dimValues[dim].size() <= 4) {
        int dimUpto = 0;
        doc->dims[dim] = dimValues[dim].size() - 1;
        while (dimUpto < dimValues[dim].size()) {
          if (random()->nextBoolean()) {
            doc->dims[dim] = dimUpto;
            break;
          }
          dimUpto++;
        }
      } else {
        doc->dims[dim] = random()->nextInt(dimValues[dim].size());
      }

      if (random()->nextInt(5) == 3) {
        // 2nd value:
        doc->dims2[dim] = random()->nextInt(dimValues[dim].size());
      } else {
        doc->dims2[dim] = -1;
      }
    }
    docs.push_back(doc);
  }

  shared_ptr<Directory> d = newDirectory();
  shared_ptr<Directory> td = newDirectory();

  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setInfoStream(InfoStream::NO_OUTPUT);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), d, iwc);
  shared_ptr<DirectoryTaxonomyWriter> tw = make_shared<DirectoryTaxonomyWriter>(
      td, IndexWriterConfig::OpenMode::CREATE);
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  for (int i = 0; i < numDims; i++) {
    config->setMultiValued(L"dim" + to_wstring(i), true);
  }

  bool doUseDV = random()->nextBoolean();

  for (auto rawDoc : docs) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newStringField(L"id", rawDoc->id, Field::Store::YES));
    doc->push_back(make_shared<SortedDocValuesField>(
        L"id", make_shared<BytesRef>(rawDoc->id)));
    doc->push_back(
        newStringField(L"content", rawDoc->contentToken, Field::Store::NO));

    if (VERBOSE) {
      wcout << L"  doc id=" << rawDoc->id << L" token=" << rawDoc->contentToken
            << endl;
    }
    for (int dim = 0; dim < numDims; dim++) {
      int dimValue = rawDoc->dims[dim];
      if (dimValue != -1) {
        if (doUseDV) {
          doc->push_back(make_shared<SortedSetDocValuesFacetField>(
              L"dim" + to_wstring(dim), dimValues[dim][dimValue]));
        } else {
          doc->push_back(make_shared<FacetField>(L"dim" + to_wstring(dim),
                                                 dimValues[dim][dimValue]));
        }
        doc->push_back(make_shared<StringField>(L"dim" + to_wstring(dim),
                                                dimValues[dim][dimValue],
                                                Field::Store::YES));
        if (VERBOSE) {
          wcout << L"    dim" << dim << L"="
                << make_shared<BytesRef>(dimValues[dim][dimValue]) << endl;
        }
      }
      int dimValue2 = rawDoc->dims2[dim];
      if (dimValue2 != -1) {
        if (doUseDV) {
          doc->push_back(make_shared<SortedSetDocValuesFacetField>(
              L"dim" + to_wstring(dim), dimValues[dim][dimValue2]));
        } else {
          doc->push_back(make_shared<FacetField>(L"dim" + to_wstring(dim),
                                                 dimValues[dim][dimValue2]));
        }
        doc->push_back(make_shared<StringField>(L"dim" + to_wstring(dim),
                                                dimValues[dim][dimValue2],
                                                Field::Store::YES));
        if (VERBOSE) {
          wcout << L"      dim" << dim << L"="
                << make_shared<BytesRef>(dimValues[dim][dimValue2]) << endl;
        }
      }
    }

    w->addDocument(config->build(tw, doc));
  }

  if (random()->nextBoolean()) {
    // Randomly delete a few docs:
    int numDel =
        TestUtil::nextInt(random(), 1, static_cast<int>(numDocs * 0.05));
    if (VERBOSE) {
      wcout << L"delete " << numDel << endl;
    }
    int delCount = 0;
    while (delCount < numDel) {
      shared_ptr<Doc> doc = docs[random()->nextInt(docs.size())];
      if (!doc->deleted) {
        if (VERBOSE) {
          wcout << L"  delete id=" << doc->id << endl;
        }
        doc->deleted = true;
        w->deleteDocuments(make_shared<Term>(L"id", doc->id));
        delCount++;
      }
    }
  }

  if (random()->nextBoolean()) {
    if (VERBOSE) {
      wcout << L"TEST: forceMerge(1)..." << endl;
    }
    w->forceMerge(1);
  }
  shared_ptr<IndexReader> r = w->getReader();

  shared_ptr<SortedSetDocValuesReaderState> *const sortedSetDVState;
  shared_ptr<IndexSearcher> s = newSearcher(r);

  if (doUseDV) {
    sortedSetDVState =
        make_shared<DefaultSortedSetDocValuesReaderState>(s->getIndexReader());
  } else {
    sortedSetDVState.reset();
  }

  if (VERBOSE) {
    wcout << L"r.numDocs() = " << r->numDocs() << endl;
  }

  // NRT open
  shared_ptr<TaxonomyReader> tr = make_shared<DirectoryTaxonomyReader>(tw);

  int numIters = atLeast(10);

  for (int iter = 0; iter < numIters; iter++) {

    wstring contentToken =
        random()->nextInt(30) == 17 ? L"" : randomContentToken(true);
    int numDrillDown = TestUtil::nextInt(random(), 1, min(4, numDims));
    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << L" baseQuery=" << contentToken
            << L" numDrillDown=" << numDrillDown << L" useSortedSetDV="
            << doUseDV << endl;
    }

    std::deque<std::deque<wstring>> drillDowns(numDims);

    int count = 0;
    bool anyMultiValuedDrillDowns = false;
    while (count < numDrillDown) {
      int dim = random()->nextInt(numDims);
      if (drillDowns[dim].empty()) {
        if (random()->nextBoolean()) {
          // Drill down on one value:
          drillDowns[dim] = std::deque<wstring>{
              dimValues[dim][random()->nextInt(dimValues[dim].size())]};
        } else {
          int orCount =
              TestUtil::nextInt(random(), 1, min(5, dimValues[dim].size()));
          drillDowns[dim] = std::deque<wstring>(orCount);
          anyMultiValuedDrillDowns |= orCount > 1;
          for (int i = 0; i < orCount; i++) {
            while (true) {
              wstring value =
                  dimValues[dim][random()->nextInt(dimValues[dim].size())];
              for (int j = 0; j < i; j++) {
                if (value == drillDowns[dim][j]) {
                  value = L"";
                  break;
                }
              }
              if (value != L"") {
                drillDowns[dim][i] = value;
                break;
              }
            }
          }
        }
        if (VERBOSE) {
          std::deque<std::shared_ptr<BytesRef>> values(drillDowns[dim].size());
          for (int i = 0; i < values.size(); i++) {
            values[i] = make_shared<BytesRef>(drillDowns[dim][i]);
          }
          // C++ TODO: There is no native C++ equivalent to 'toString':
          wcout << L"  dim" << dim << L"=" << Arrays->toString(values) << endl;
        }
        count++;
      }
    }

    shared_ptr<Query> baseQuery;
    if (contentToken == L"") {
      baseQuery = make_shared<MatchAllDocsQuery>();
    } else {
      baseQuery =
          make_shared<TermQuery>(make_shared<Term>(L"content", contentToken));
    }

    shared_ptr<DrillDownQuery> ddq =
        make_shared<DrillDownQuery>(config, baseQuery);

    for (int dim = 0; dim < numDims; dim++) {
      if (drillDowns[dim].size() > 0) {
        for (auto value : drillDowns[dim]) {
          ddq->add(L"dim" + to_wstring(dim), {value});
        }
      }
    }

    shared_ptr<Query> filter;
    if (random()->nextInt(7) == 6) {
      if (VERBOSE) {
        wcout << L"  only-even filter" << endl;
      }
      filter = make_shared<QueryAnonymousInnerClass>(shared_from_this());
    } else {
      filter.reset();
    }

    // Verify docs are always collected in order.  If we
    // had an AssertingScorer it could catch it when
    // Weight.scoresDocsOutOfOrder lies!:
    getNewDrillSideways(s, config, tr)
        ->search(ddq, make_shared<SimpleCollectorAnonymousInnerClass>(
                          shared_from_this()));

    // Also separately verify that DS respects the
    // scoreSubDocsAtOnce method, to ensure that all
    // subScorers are on the same docID:
    if (!anyMultiValuedDrillDowns) {
      // Can only do this test when there are no OR'd
      // drill-down values, because in that case it's
      // easily possible for one of the DD terms to be on
      // a future docID:
      getNewDrillSidewaysScoreSubdocsAtOnce(s, config, tr)
          ->search(ddq, make_shared<AssertingSubDocsAtOnceCollector>());
    }

    shared_ptr<TestFacetResult> expected = slowDrillSidewaysSearch(
        s, docs, contentToken, drillDowns, dimValues, filter);

    shared_ptr<Sort> sort = make_shared<Sort>(
        make_shared<SortField>(L"id", SortField::Type::STRING));
    shared_ptr<DrillSideways> ds;
    if (doUseDV) {
      ds = getNewDrillSideways(s, config, sortedSetDVState);
    } else {
      ds = getNewDrillSidewaysBuildFacetsResult(s, config, tr);
    }

    // Retrieve all facets:
    shared_ptr<DrillSidewaysResult> actual =
        ds->search(ddq, filter, nullptr, numDocs, sort, true, true);

    shared_ptr<TopDocs> hits = s->search(baseQuery, numDocs);
    unordered_map<wstring, float> scores = unordered_map<wstring, float>();
    for (auto sd : hits->scoreDocs) {
      scores.emplace(s->doc(sd->doc)[L"id"], sd->score);
    }
    if (VERBOSE) {
      wcout << L"  verify all facets" << endl;
    }
    verifyEquals(dimValues, s, expected, actual, scores, doUseDV);

    // Make sure drill down doesn't change score:
    shared_ptr<Query> q = ddq;
    if (filter != nullptr) {
      q = (make_shared<BooleanQuery::Builder>())
              ->add(q, BooleanClause::Occur::MUST)
              ->add(filter, BooleanClause::Occur::FILTER)
              ->build();
    }
    shared_ptr<TopDocs> ddqHits = s->search(q, numDocs);
    TestUtil::assertEquals(expected->hits.size(), ddqHits->totalHits);
    for (int i = 0; i < expected->hits.size(); i++) {
      // Score should be IDENTICAL:
      assertEquals(scores[expected->hits[i]->id], ddqHits->scoreDocs[i]->score,
                   0.0f);
    }
  }

  delete w;
  IOUtils::close({r, tr, tw, d, td});
}

TestDrillSideways::QueryAnonymousInnerClass::QueryAnonymousInnerClass(
    shared_ptr<TestDrillSideways> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Weight> TestDrillSideways::QueryAnonymousInnerClass::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

TestDrillSideways::QueryAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass::
        ConstantScoreWeightAnonymousInnerClass(
            shared_ptr<QueryAnonymousInnerClass> outerInstance, float boost)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer> TestDrillSideways::QueryAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass::scorer(
        shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<DocIdSetIterator> approximation =
      DocIdSetIterator::all(context->reader()->maxDoc());
  return make_shared<ConstantScoreScorer>(
      shared_from_this(), score(),
      make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(),
                                                       approximation, context));
}

TestDrillSideways::QueryAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass::
        TwoPhaseIteratorAnonymousInnerClass::
            TwoPhaseIteratorAnonymousInnerClass(
                shared_ptr<ConstantScoreWeightAnonymousInnerClass>
                    outerInstance,
                shared_ptr<DocIdSetIterator> approximation,
                shared_ptr<LeafReaderContext> context)
    : org::apache::lucene::search::TwoPhaseIterator(approximation)
{
  this->outerInstance = outerInstance;
  this->context = context;
  this->approximation = approximation;
}

bool TestDrillSideways::QueryAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass::
        TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  int docID = approximation->docID();
  return (static_cast<Integer>(context->reader()->document(docID)[L"id"]) &
          1) == 0;
}

float TestDrillSideways::QueryAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass::
        TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 1000.0f;
}

bool TestDrillSideways::QueryAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass::isCacheable(
        shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

// C++ TODO: There is no native C++ equivalent to 'toString':
wstring TestDrillSideways::QueryAnonymousInnerClass::Term::toString(
    const wstring &field)
{
  return L"drillSidewaysTestFilter";
}

bool TestDrillSideways::QueryAnonymousInnerClass::equals(any o)
{
  return o == shared_from_this();
}

int TestDrillSideways::QueryAnonymousInnerClass::hashCode()
{
  return System::identityHashCode(shared_from_this());
}

TestDrillSideways::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(
        shared_ptr<TestDrillSideways> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestDrillSideways::SimpleCollectorAnonymousInnerClass::collect(int doc)
{
  assert(doc > lastDocID);
  lastDocID = doc;
}

void TestDrillSideways::SimpleCollectorAnonymousInnerClass::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  lastDocID = -1;
}

bool TestDrillSideways::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return false;
}

TestDrillSideways::Counters::Counters(
    std::deque<std::deque<wstring>> &dimValues)
{
  counts = std::deque<std::deque<int>>(dimValues.size());
  for (int dim = 0; dim < dimValues.size(); dim++) {
    counts[dim] = std::deque<int>(dimValues[dim].size());
  }
}

void TestDrillSideways::Counters::inc(std::deque<int> &dims,
                                      std::deque<int> &dims2)
{
  inc(dims, dims2, -1);
}

void TestDrillSideways::Counters::inc(std::deque<int> &dims,
                                      std::deque<int> &dims2, int onlyDim)
{
  assert(dims.size() == counts.size());
  assert(dims2.size() == counts.size());
  for (int dim = 0; dim < dims.size(); dim++) {
    if (onlyDim == -1 || dim == onlyDim) {
      if (dims[dim] != -1) {
        counts[dim][dims[dim]]++;
      }
      if (dims2[dim] != -1 && dims2[dim] != dims[dim]) {
        counts[dim][dims2[dim]]++;
      }
    }
  }
}

TestDrillSideways::TestFacetResult::TestFacetResult() {}

std::deque<int> TestDrillSideways::getTopNOrds(std::deque<int> &counts,
                                                std::deque<wstring> &values,
                                                int topN)
{
  const std::deque<int> ids = std::deque<int>(counts.size());
  for (int i = 0; i < ids.size(); i++) {
    ids[i] = i;
  }

  // Naive (on purpose, to reduce bug in tester/gold):
  // sort all ids, then return top N slice:
  make_shared<InPlaceMergeSorterAnonymousInnerClass>(shared_from_this(), counts,
                                                     values, ids)
      .sort(0, ids.size());

  if (topN > ids.size()) {
    topN = ids.size();
  }

  int numSet = topN;
  for (int i = 0; i < topN; i++) {
    if (counts[ids[i]] == 0) {
      numSet = i;
      break;
    }
  }

  std::deque<int> topNIDs(numSet);
  System::arraycopy(ids, 0, topNIDs, 0, topNIDs.size());
  return topNIDs;
}

TestDrillSideways::InPlaceMergeSorterAnonymousInnerClass::
    InPlaceMergeSorterAnonymousInnerClass(
        shared_ptr<TestDrillSideways> outerInstance, deque<int> &counts,
        deque<wstring> &values, deque<int> &ids)
{
  this->outerInstance = outerInstance;
  this->counts = counts;
  this->values = values;
  this->ids = ids;
}

void TestDrillSideways::InPlaceMergeSorterAnonymousInnerClass::swap(int i,
                                                                    int j)
{
  int id = ids[i];
  ids[i] = ids[j];
  ids[j] = id;
}

int TestDrillSideways::InPlaceMergeSorterAnonymousInnerClass::compare(int i,
                                                                      int j)
{
  int counti = counts[ids[i]];
  int countj = counts[ids[j]];
  // Sort by count descending...
  if (counti > countj) {
    return -1;
  } else if (counti < countj) {
    return 1;
  } else {
    // ... then by label ascending:
    return (make_shared<BytesRef>(values[ids[i]]))
        ->compareTo(make_shared<BytesRef>(values[ids[j]]));
  }
}

shared_ptr<TestFacetResult> TestDrillSideways::slowDrillSidewaysSearch(
    shared_ptr<IndexSearcher> s, deque<std::shared_ptr<Doc>> &docs,
    const wstring &contentToken, std::deque<std::deque<wstring>> &drillDowns,
    std::deque<std::deque<wstring>> &dimValues,
    shared_ptr<Query> onlyEven) 
{
  int numDims = dimValues.size();

  deque<std::shared_ptr<Doc>> hits = deque<std::shared_ptr<Doc>>();
  shared_ptr<Counters> drillDownCounts = make_shared<Counters>(dimValues);
  std::deque<std::shared_ptr<Counters>> drillSidewaysCounts(dimValues.size());
  for (int dim = 0; dim < numDims; dim++) {
    drillSidewaysCounts[dim] = make_shared<Counters>(dimValues);
  }

  if (VERBOSE) {
    wcout << L"  compute expected" << endl;
  }

  for (auto doc : docs) {
    if (doc->deleted) {
      continue;
    }
    if (onlyEven != nullptr & (stoi(doc->id) & 1) != 0) {
      continue;
    }
    if (contentToken == L"" || doc->contentToken == contentToken) {
      int failDim = -1;
      for (int dim = 0; dim < numDims; dim++) {
        if (drillDowns[dim].size() > 0) {
          wstring docValue =
              doc->dims[dim] == -1 ? nullptr : dimValues[dim][doc->dims[dim]];
          wstring docValue2 =
              doc->dims2[dim] == -1 ? nullptr : dimValues[dim][doc->dims2[dim]];
          bool matches = false;
          for (auto value : drillDowns[dim]) {
            if (value.equals(docValue) || value.equals(docValue2)) {
              matches = true;
              break;
            }
          }
          if (!matches) {
            if (failDim == -1) {
              // Doc could be a near-miss, if no other dim fails
              failDim = dim;
            } else {
              // Doc isn't a hit nor a near-miss
              goto nextDocContinue;
            }
          }
        }
      }

      if (failDim == -1) {
        if (VERBOSE) {
          wcout << L"    exp: id=" << doc->id << L" is a hit" << endl;
        }
        // Hit:
        hits.push_back(doc);
        drillDownCounts->inc(doc->dims, doc->dims2);
        for (int dim = 0; dim < dimValues.size(); dim++) {
          drillSidewaysCounts[dim]->inc(doc->dims, doc->dims2);
        }
      } else {
        if (VERBOSE) {
          wcout << L"    exp: id=" << doc->id << L" is a near-miss on dim="
                << failDim << endl;
        }
        drillSidewaysCounts[failDim]->inc(doc->dims, doc->dims2, failDim);
      }
    }
  nextDocContinue:;
  }
nextDocBreak:

  unordered_map<wstring, int> idToDocID = unordered_map<wstring, int>();
  for (int i = 0; i < s->getIndexReader()->maxDoc(); i++) {
    idToDocID.emplace(s->doc(i)[L"id"], i);
  }

  sort(hits.begin(), hits.end());

  shared_ptr<TestFacetResult> res = make_shared<TestFacetResult>();
  res->hits = hits;
  res->counts = std::deque<std::deque<int>>(numDims);
  res->uniqueCounts = std::deque<int>(numDims);
  for (int dim = 0; dim < numDims; dim++) {
    if (drillDowns[dim].size() > 0) {
      res->counts[dim] = drillSidewaysCounts[dim]->counts[dim];
    } else {
      res->counts[dim] = drillDownCounts->counts[dim];
    }
    int uniqueCount = 0;
    for (int j = 0; j < res->counts[dim].length; j++) {
      if (res->counts[dim][j] != 0) {
        uniqueCount++;
      }
    }
    res->uniqueCounts[dim] = uniqueCount;
  }

  return res;
}

void TestDrillSideways::verifyEquals(
    std::deque<std::deque<wstring>> &dimValues, shared_ptr<IndexSearcher> s,
    shared_ptr<TestFacetResult> expected,
    shared_ptr<DrillSidewaysResult> actual,
    unordered_map<wstring, float> &scores,
    bool isSortedSetDV) 
{
  if (VERBOSE) {
    wcout << L"  verify totHits=" << expected->hits.size() << endl;
  }
  TestUtil::assertEquals(expected->hits.size(), actual->hits->totalHits);
  TestUtil::assertEquals(expected->hits.size(), actual->hits->scoreDocs.size());
  for (int i = 0; i < expected->hits.size(); i++) {
    if (VERBOSE) {
      wcout << L"    hit " << i << L" expected=" << expected->hits[i]->id
            << endl;
    }
    TestUtil::assertEquals(expected->hits.get(i)->id,
                           s->doc(actual->hits->scoreDocs[i]->doc)->get(L"id"));
    // Score should be IDENTICAL:
    assertEquals(scores[expected->hits.get(i)->id],
                 actual->hits->scoreDocs[i]->score, 0.0f);
  }

  for (int dim = 0; dim < expected->counts.size(); dim++) {
    int topN = random()->nextBoolean()
                   ? dimValues[dim].size()
                   : TestUtil::nextInt(random(), 1, dimValues[dim].size());
    shared_ptr<FacetResult> fr =
        actual->facets->getTopChildren(topN, L"dim" + to_wstring(dim));
    if (VERBOSE) {
      wcout << L"    dim" << dim << L" topN=" << topN << L" (vs "
            << dimValues[dim].size() << L" unique values)" << endl;
      wcout << L"      actual" << endl;
    }

    int idx = 0;
    unordered_map<wstring, int> actualValues = unordered_map<wstring, int>();

    if (fr != nullptr) {
      for (auto labelValue : fr->labelValues) {
        actualValues.emplace(labelValue->label, labelValue->value->intValue());
        if (VERBOSE) {
          wcout << L"        " << idx << L": "
                << make_shared<BytesRef>(labelValue->label) << L": "
                << labelValue->value << endl;
          idx++;
        }
      }
      assertEquals(L"dim=" + to_wstring(dim), expected->uniqueCounts[dim],
                   fr->childCount);
    }

    if (topN < dimValues[dim].size()) {
      std::deque<int> topNIDs =
          getTopNOrds(expected->counts[dim], dimValues[dim], topN);
      if (VERBOSE) {
        idx = 0;
        wcout << L"      expected (sorted)" << endl;
        for (int i = 0; i < topNIDs.size(); i++) {
          int expectedOrd = topNIDs[i];
          wstring value = dimValues[dim][expectedOrd];
          wcout << L"        " << idx << L": " << make_shared<BytesRef>(value)
                << L": " << expected->counts[dim][expectedOrd] << endl;
          idx++;
        }
      }
      if (VERBOSE) {
        wcout << L"      topN=" << topN << L" expectedTopN=" << topNIDs.size()
              << endl;
      }

      if (fr != nullptr) {
        TestUtil::assertEquals(topNIDs.size(), fr->labelValues.size());
      } else {
        TestUtil::assertEquals(0, topNIDs.size());
      }
      for (int i = 0; i < topNIDs.size(); i++) {
        int expectedOrd = topNIDs[i];
        TestUtil::assertEquals(expected->counts[dim][expectedOrd],
                               fr->labelValues[i]->value.intValue());
        if (isSortedSetDV) {
          // Tie-break facet labels are only in unicode
          // order with SortedSetDVFacets:
          assertEquals(L"value @ idx=" + to_wstring(i),
                       dimValues[dim][expectedOrd], fr->labelValues[i]->label);
        }
      }
    } else {

      if (VERBOSE) {
        idx = 0;
        wcout << L"      expected (unsorted)" << endl;
        for (int i = 0; i < dimValues[dim].size(); i++) {
          wstring value = dimValues[dim][i];
          if (expected->counts[dim][i] != 0) {
            wcout << L"        " << idx << L": " << make_shared<BytesRef>(value)
                  << L": " << expected->counts[dim][i] << endl;
            idx++;
          }
        }
      }

      int setCount = 0;
      for (int i = 0; i < dimValues[dim].size(); i++) {
        wstring value = dimValues[dim][i];
        if (expected->counts[dim][i] != 0) {
          assertTrue(actualValues.find(value) != actualValues.end());
          TestUtil::assertEquals(expected->counts[dim][i], actualValues[value]);
          setCount++;
        } else {
          assertFalse(actualValues.find(value) != actualValues.end());
        }
      }
      TestUtil::assertEquals(setCount, actualValues.size());
    }
  }
}

void TestDrillSideways::testEmptyIndex() 
{
  // LUCENE-5045: make sure DrillSideways works with an empty index
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir,
                                           IndexWriterConfig::OpenMode::CREATE);
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  // Count "Author"
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  shared_ptr<DrillSideways> ds =
      getNewDrillSideways(searcher, config, taxoReader);
  shared_ptr<DrillDownQuery> ddq = make_shared<DrillDownQuery>(config);
  ddq->add(L"Author", {L"Lisa"});

  shared_ptr<DrillSidewaysResult> r =
      ds->search(ddq, 10); // this used to fail on IllegalArgEx
  TestUtil::assertEquals(0, r->hits->totalHits);

  r = ds->search(
      ddq, nullptr, nullptr, 10,
      make_shared<Sort>(make_shared<SortField>(L"foo", SortField::Type::INT)),
      false, false); // this used to fail on IllegalArgEx
  TestUtil::assertEquals(0, r->hits->totalHits);

  delete writer;
  IOUtils::close(
      {taxoWriter, searcher->getIndexReader(), taxoReader, dir, taxoDir});
}

void TestDrillSideways::testScorer() 
{
  // LUCENE-6001 some scorers, eg ReqExlScorer, can hit NPE if cost is called
  // after nextDoc
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  // Writes facet ords to a separate directory from the
  // main index:
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir,
                                           IndexWriterConfig::OpenMode::CREATE);

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"foo bar", Field::Store::NO));
  doc->push_back(make_shared<FacetField>(L"Author", L"Bob"));
  doc->push_back(make_shared<FacetField>(L"dim", L"a"));
  writer->addDocument(config->build(taxoWriter, doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());

  // NRT open
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<DrillSideways> ds =
      getNewDrillSideways(searcher, config, taxoReader);

  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"foo")),
          BooleanClause::Occur::MUST);
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"bar")),
          BooleanClause::Occur::MUST_NOT);
  shared_ptr<DrillDownQuery> ddq =
      make_shared<DrillDownQuery>(config, bq->build());
  ddq->add(L"field", {L"foo"});
  ddq->add(L"author", bq->build());
  ddq->add(L"dim", bq->build());
  shared_ptr<DrillSidewaysResult> r = ds->search(nullptr, ddq, 10);
  TestUtil::assertEquals(0, r->hits->totalHits);

  delete writer;
  IOUtils::close(
      {searcher->getIndexReader(), taxoReader, taxoWriter, dir, taxoDir});
}
} // namespace org::apache::lucene::facet