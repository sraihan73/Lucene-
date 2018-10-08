using namespace std;

#include "TestBooleanQueryVisitSubscorers.h"

namespace org::apache::lucene::search
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using TextField = org::apache::lucene::document::TextField;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using ChildScorer = org::apache::lucene::search::Scorer::ChildScorer;
using BasicStats = org::apache::lucene::search::similarities::BasicStats;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring TestBooleanQueryVisitSubscorers::F1 = L"title";
const wstring TestBooleanQueryVisitSubscorers::F2 = L"body";

void TestBooleanQueryVisitSubscorers::setUp() 
{
  LuceneTestCase::setUp();
  analyzer = make_shared<MockAnalyzer>(random());
  dir = newDirectory();
  shared_ptr<IndexWriterConfig> config = newIndexWriterConfig(analyzer);
  config->setMergePolicy(newLogMergePolicy()); // we will use docids to validate
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, config);
  writer->addDocument(
      doc(L"lucene", L"lucene is a very popular search engine library"));
  writer->addDocument(doc(
      L"solr", L"solr is a very popular search server and is using lucene"));
  writer->addDocument(
      doc(L"nutch", L"nutch is an internet search engine with web crawler and "
                    L"is using lucene and hadoop"));
  reader = writer->getReader();
  delete writer;
  // we do not use newSearcher because the assertingXXX layers break
  // the toString representations we are relying on
  // TODO: clean that up
  searcher = make_shared<IndexSearcher>(reader);
  searcher->setSimilarity(make_shared<ClassicSimilarity>());
  scorerSearcher = make_shared<ScorerIndexSearcher>(reader);
  scorerSearcher->setSimilarity(make_shared<CountingSimilarity>());
}

void TestBooleanQueryVisitSubscorers::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestBooleanQueryVisitSubscorers::testDisjunctions() 
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<TermQuery>(make_shared<Term>(F1, L"lucene")),
          BooleanClause::Occur::SHOULD);
  bq->add(make_shared<TermQuery>(make_shared<Term>(F2, L"lucene")),
          BooleanClause::Occur::SHOULD);
  bq->add(make_shared<TermQuery>(make_shared<Term>(F2, L"search")),
          BooleanClause::Occur::SHOULD);
  unordered_map<int, int> tfs = getDocCounts(scorerSearcher, bq->build());
  assertEquals(3, tfs.size()); // 3 documents
  assertEquals(3, tfs[0]);     // f1:lucene + f2:lucene + f2:search
  assertEquals(2, tfs[1]);     // f2:search + f2:lucene
  assertEquals(2, tfs[2]);     // f2:search + f2:lucene
}

void TestBooleanQueryVisitSubscorers::testNestedDisjunctions() throw(
    IOException)
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<TermQuery>(make_shared<Term>(F1, L"lucene")),
          BooleanClause::Occur::SHOULD);
  shared_ptr<BooleanQuery::Builder> bq2 = make_shared<BooleanQuery::Builder>();
  bq2->add(make_shared<TermQuery>(make_shared<Term>(F2, L"lucene")),
           BooleanClause::Occur::SHOULD);
  bq2->add(make_shared<TermQuery>(make_shared<Term>(F2, L"search")),
           BooleanClause::Occur::SHOULD);
  bq->add(bq2->build(), BooleanClause::Occur::SHOULD);
  unordered_map<int, int> tfs = getDocCounts(scorerSearcher, bq->build());
  assertEquals(3, tfs.size()); // 3 documents
  assertEquals(3, tfs[0]);     // f1:lucene + f2:lucene + f2:search
  assertEquals(2, tfs[1]);     // f2:search + f2:lucene
  assertEquals(2, tfs[2]);     // f2:search + f2:lucene
}

void TestBooleanQueryVisitSubscorers::testConjunctions() 
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<TermQuery>(make_shared<Term>(F2, L"lucene")),
          BooleanClause::Occur::MUST);
  bq->add(make_shared<TermQuery>(make_shared<Term>(F2, L"is")),
          BooleanClause::Occur::MUST);
  unordered_map<int, int> tfs = getDocCounts(scorerSearcher, bq->build());
  assertEquals(3, tfs.size()); // 3 documents
  assertEquals(2, tfs[0]);     // f2:lucene + f2:is
  assertEquals(3, tfs[1]);     // f2:is + f2:is + f2:lucene
  assertEquals(3, tfs[2]);     // f2:is + f2:is + f2:lucene
}

shared_ptr<Document> TestBooleanQueryVisitSubscorers::doc(const wstring &v1,
                                                          const wstring &v2)
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(F1, v1, Store::YES));
  doc->push_back(make_shared<TextField>(F2, v2, Store::YES));
  return doc;
}

unordered_map<int, int> TestBooleanQueryVisitSubscorers::getDocCounts(
    shared_ptr<IndexSearcher> searcher,
    shared_ptr<Query> query) 
{
  shared_ptr<MyCollector> collector = make_shared<MyCollector>();
  searcher->search(query, collector);
  return collector->docCounts;
}

TestBooleanQueryVisitSubscorers::MyCollector::MyCollector()
    : FilterCollector(TopScoreDocCollector::create(10))
{
}

shared_ptr<LeafCollector>
TestBooleanQueryVisitSubscorers::MyCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  constexpr int docBase = context->docBase;
  return make_shared<FilterLeafCollectorAnonymousInnerClass>(
      shared_from_this(), FilterCollector::getLeafCollector(context), docBase);
}

TestBooleanQueryVisitSubscorers::MyCollector::
    FilterLeafCollectorAnonymousInnerClass::
        FilterLeafCollectorAnonymousInnerClass(
            shared_ptr<MyCollector> outerInstance,
            shared_ptr<org::apache::lucene::search::LeafCollector>
                getLeafCollector,
            int docBase)
    : FilterLeafCollector(getLeafCollector)
{
  this->outerInstance = outerInstance;
  this->docBase = docBase;
}

void TestBooleanQueryVisitSubscorers::MyCollector::
    FilterLeafCollectorAnonymousInnerClass::setScorer(
        shared_ptr<Scorer> scorer) 
{
  outerInstance->super->setScorer(scorer);
  outerInstance->tqsSet->clear();
  outerInstance->fillLeaves(scorer, outerInstance->tqsSet);
}

void TestBooleanQueryVisitSubscorers::MyCollector::
    FilterLeafCollectorAnonymousInnerClass::collect(int doc) 
{
  int freq = 0;
  for (auto scorer : outerInstance->tqsSet) {
    if (doc == scorer->docID()) {
      freq += scorer->score();
    }
  }
  outerInstance->docCounts.emplace(doc + docBase, freq);
  outerInstance->super->collect(doc);
}

void TestBooleanQueryVisitSubscorers::MyCollector::fillLeaves(
    shared_ptr<Scorer> scorer,
    shared_ptr<Set<std::shared_ptr<Scorer>>> set) 
{
  if (std::dynamic_pointer_cast<TermQuery>(scorer->getWeight()->getQuery()) !=
      nullptr) {
    set->add(scorer);
  } else {
    for (auto child : scorer->getChildren()) {
      fillLeaves(child->child, set);
    }
  }
}

shared_ptr<TopDocs> TestBooleanQueryVisitSubscorers::MyCollector::topDocs()
{
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: return ((TopDocsCollector<?>) in).topDocs();
      return (std::static_pointer_cast<TopDocsCollector<?>>(in_))->topDocs();
}

int TestBooleanQueryVisitSubscorers::MyCollector::freq(int doc) throw(
    IOException)
{
  return docCounts[doc];
}

void TestBooleanQueryVisitSubscorers::testDisjunctionMatches() throw(
    IOException)
{
  shared_ptr<BooleanQuery::Builder> bq1 = make_shared<BooleanQuery::Builder>();
  bq1->add(make_shared<TermQuery>(make_shared<Term>(F1, L"lucene")),
           Occur::SHOULD);
  bq1->add(make_shared<PhraseQuery>(F2, L"search", L"engine"), Occur::SHOULD);

  shared_ptr<Weight> w1 = scorerSearcher->createWeight(
      scorerSearcher->rewrite(bq1->build()), true, 1);
  shared_ptr<Scorer> s1 = w1->scorer(reader->leaves()[0]);
  assertEquals(0, s1->begin().nextDoc());
  assertEquals(2, s1->getChildren()->size());

  shared_ptr<BooleanQuery::Builder> bq2 = make_shared<BooleanQuery::Builder>();
  bq2->add(make_shared<TermQuery>(make_shared<Term>(F1, L"lucene")),
           Occur::SHOULD);
  bq2->add(make_shared<PhraseQuery>(F2, L"search", L"library"), Occur::SHOULD);

  shared_ptr<Weight> w2 = scorerSearcher->createWeight(
      scorerSearcher->rewrite(bq2->build()), true, 1);
  shared_ptr<Scorer> s2 = w2->scorer(reader->leaves()[0]);
  assertEquals(0, s2->begin().nextDoc());
  assertEquals(1, s2->getChildren()->size());
}

void TestBooleanQueryVisitSubscorers::testMinShouldMatchMatches() throw(
    IOException)
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<TermQuery>(make_shared<Term>(F1, L"lucene")),
          Occur::SHOULD);
  bq->add(make_shared<TermQuery>(make_shared<Term>(F2, L"lucene")),
          Occur::SHOULD);
  bq->add(make_shared<PhraseQuery>(F2, L"search", L"library"), Occur::SHOULD);
  bq->setMinimumNumberShouldMatch(2);

  shared_ptr<Weight> w = scorerSearcher->createWeight(
      scorerSearcher->rewrite(bq->build()), true, 1);
  shared_ptr<Scorer> s = w->scorer(reader->leaves()[0]);
  assertEquals(0, s->begin().nextDoc());
  assertEquals(2, s->getChildren()->size());
}

void TestBooleanQueryVisitSubscorers::
    testGetChildrenMinShouldMatchSumScorer() 
{
  shared_ptr<BooleanQuery::Builder> *const query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(F2, L"nutch")),
             Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(F2, L"web")),
             Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(F2, L"crawler")),
             Occur::SHOULD);
  query->setMinimumNumberShouldMatch(2);
  query->add(make_shared<MatchAllDocsQuery>(), Occur::MUST);
  shared_ptr<ScorerSummarizingCollector> collector =
      make_shared<ScorerSummarizingCollector>();
  searcher->search(query->build(), collector);
  assertEquals(1, collector->getNumHits());
  assertFalse(collector->getSummaries().empty());
  for (auto summary : collector->getSummaries()) {
    assertEquals(wstring(L"ConjunctionScorer\n") +
                     L"    MUST ConstantScoreScorer\n" +
                     L"    MUST MinShouldMatchSumScorer\n" +
                     L"            SHOULD TermScorer body:crawler\n" +
                     L"            SHOULD TermScorer body:web\n" +
                     L"            SHOULD TermScorer body:nutch",
                 summary);
  }
}

void TestBooleanQueryVisitSubscorers::testGetChildrenBoosterScorer() throw(
    IOException)
{
  shared_ptr<BooleanQuery::Builder> *const query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(F2, L"nutch")),
             Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(F2, L"miss")),
             Occur::SHOULD);
  shared_ptr<ScorerSummarizingCollector> collector =
      make_shared<ScorerSummarizingCollector>();
  scorerSearcher->search(query->build(), collector);
  assertEquals(1, collector->getNumHits());
  assertFalse(collector->getSummaries().empty());
  for (auto summary : collector->getSummaries()) {
    assertEquals(L"TermScorer body:nutch", summary);
  }
}

int TestBooleanQueryVisitSubscorers::ScorerSummarizingCollector::getNumHits()
{
  return numHits[0];
}

deque<wstring>
TestBooleanQueryVisitSubscorers::ScorerSummarizingCollector::getSummaries()
{
  return summaries;
}

bool TestBooleanQueryVisitSubscorers::ScorerSummarizingCollector::needsScores()
{
  return true;
}

shared_ptr<LeafCollector>
TestBooleanQueryVisitSubscorers::ScorerSummarizingCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<LeafCollectorAnonymousInnerClass>(shared_from_this());
}

TestBooleanQueryVisitSubscorers::ScorerSummarizingCollector::
    LeafCollectorAnonymousInnerClass::LeafCollectorAnonymousInnerClass(
        shared_ptr<ScorerSummarizingCollector> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestBooleanQueryVisitSubscorers::ScorerSummarizingCollector::
    LeafCollectorAnonymousInnerClass::setScorer(
        shared_ptr<Scorer> scorer) 
{
  shared_ptr<StringBuilder> *const builder = make_shared<StringBuilder>();
  summarizeScorer(builder, scorer, 0);
  outerInstance->summaries.push_back(builder->toString());
}

void TestBooleanQueryVisitSubscorers::ScorerSummarizingCollector::
    LeafCollectorAnonymousInnerClass::collect(int doc) 
{
  outerInstance->numHits[0]++;
}

void TestBooleanQueryVisitSubscorers::ScorerSummarizingCollector::
    summarizeScorer(shared_ptr<StringBuilder> builder,
                    shared_ptr<Scorer> scorer,
                    int const indent) 
{
  builder->append(scorer->getClass().getSimpleName());
  if (std::dynamic_pointer_cast<TermScorer>(scorer) != nullptr) {
    shared_ptr<TermQuery> termQuery =
        std::static_pointer_cast<TermQuery>(scorer->getWeight()->getQuery());
    builder->append(L" ")
        ->append(termQuery->getTerm()->field())
        ->append(L":")
        ->append(termQuery->getTerm()->text());
  }
  for (auto childScorer : scorer->getChildren()) {
    ScorerSummarizingCollector::indent(builder, indent + 1)
        ->append(childScorer->relationship)
        ->append(L" ");
    summarizeScorer(builder, childScorer->child, indent + 2);
  }
}

shared_ptr<StringBuilder>
TestBooleanQueryVisitSubscorers::ScorerSummarizingCollector::indent(
    shared_ptr<StringBuilder> builder, int const indent)
{
  if (builder->length() != 0) {
    builder->append(L"\n");
  }
  for (int i = 0; i < indent; i++) {
    builder->append(L"    ");
  }
  return builder;
}

int64_t TestBooleanQueryVisitSubscorers::CountingSimilarity::computeNorm(
    shared_ptr<FieldInvertState> state)
{
  return 1;
}

shared_ptr<Similarity::SimWeight>
TestBooleanQueryVisitSubscorers::CountingSimilarity::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  return make_shared<BasicStats>(L"", boost);
}

shared_ptr<Similarity::SimScorer>
TestBooleanQueryVisitSubscorers::CountingSimilarity::simScorer(
    shared_ptr<Similarity::SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<SimScorerAnonymousInnerClass>(shared_from_this());
}

TestBooleanQueryVisitSubscorers::CountingSimilarity::
    SimScorerAnonymousInnerClass::SimScorerAnonymousInnerClass(
        shared_ptr<CountingSimilarity> outerInstance)
{
  this->outerInstance = outerInstance;
}

float TestBooleanQueryVisitSubscorers::CountingSimilarity::
    SimScorerAnonymousInnerClass::score(int doc, float freq) 
{
  return freq;
}

float TestBooleanQueryVisitSubscorers::CountingSimilarity::
    SimScorerAnonymousInnerClass::computeSlopFactor(int distance)
{
  return 1;
}

float TestBooleanQueryVisitSubscorers::CountingSimilarity::
    SimScorerAnonymousInnerClass::computePayloadFactor(
        int doc, int start, int end, shared_ptr<BytesRef> payload)
{
  return 1;
}
} // namespace org::apache::lucene::search