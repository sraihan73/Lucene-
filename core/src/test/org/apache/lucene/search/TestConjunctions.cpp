using namespace std;

#include "TestConjunctions.h"

namespace org::apache::lucene::search
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring TestConjunctions::F1 = L"title";
const wstring TestConjunctions::F2 = L"body";

void TestConjunctions::setUp() 
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
  searcher = newSearcher(reader);
  searcher->setSimilarity(make_shared<TFSimilarity>());
}

shared_ptr<Document> TestConjunctions::doc(const wstring &v1, const wstring &v2)
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(F1, v1, Store::YES));
  doc->push_back(make_shared<TextField>(F2, v2, Store::YES));
  return doc;
}

void TestConjunctions::testTermConjunctionsWithOmitTF() 
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<TermQuery>(make_shared<Term>(F1, L"nutch")),
          BooleanClause::Occur::MUST);
  bq->add(make_shared<TermQuery>(make_shared<Term>(F2, L"is")),
          BooleanClause::Occur::MUST);
  shared_ptr<TopDocs> td = searcher->search(bq->build(), 3);
  assertEquals(1, td->totalHits);
  assertEquals(3.0F, td->scoreDocs[0]->score,
               0.001F); // f1:nutch + f2:is + f2:is
}

void TestConjunctions::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

int64_t
TestConjunctions::TFSimilarity::computeNorm(shared_ptr<FieldInvertState> state)
{
  return 1; // we dont care
}

shared_ptr<Similarity::SimWeight> TestConjunctions::TFSimilarity::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  return make_shared<SimWeightAnonymousInnerClass>(shared_from_this());
}

TestConjunctions::TFSimilarity::SimWeightAnonymousInnerClass::
    SimWeightAnonymousInnerClass(shared_ptr<TFSimilarity> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Similarity::SimScorer> TestConjunctions::TFSimilarity::simScorer(
    shared_ptr<Similarity::SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<SimScorerAnonymousInnerClass>(shared_from_this());
}

TestConjunctions::TFSimilarity::SimScorerAnonymousInnerClass::
    SimScorerAnonymousInnerClass(shared_ptr<TFSimilarity> outerInstance)
{
  this->outerInstance = outerInstance;
}

float TestConjunctions::TFSimilarity::SimScorerAnonymousInnerClass::score(
    int doc, float freq)
{
  return freq;
}

float TestConjunctions::TFSimilarity::SimScorerAnonymousInnerClass::
    computeSlopFactor(int distance)
{
  return 1.0F;
}

float TestConjunctions::TFSimilarity::SimScorerAnonymousInnerClass::
    computePayloadFactor(int doc, int start, int end,
                         shared_ptr<BytesRef> payload)
{
  return 1.0F;
}

void TestConjunctions::testScorerGetChildren() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"a b", Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<BooleanQuery::Builder> b = make_shared<BooleanQuery::Builder>();
  b->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"a")),
         BooleanClause::Occur::MUST);
  b->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"b")),
         BooleanClause::Occur::FILTER);
  shared_ptr<Query> q = b->build();
  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);
  const std::deque<bool> setScorerCalled = std::deque<bool>(1);
  s->search(q, make_shared<SimpleCollectorAnonymousInnerClass>(
                   shared_from_this(), doc, s, setScorerCalled));
  assertTrue(setScorerCalled[0]);
  IOUtils::close({r, w, dir});
}

TestConjunctions::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(
        shared_ptr<TestConjunctions> outerInstance, shared_ptr<Document> doc,
        shared_ptr<org::apache::lucene::search::IndexSearcher> s,
        deque<bool> &setScorerCalled)
{
  this->outerInstance = outerInstance;
  this->doc = doc;
  this->s = s;
  this->setScorerCalled = setScorerCalled;
}

void TestConjunctions::SimpleCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> s) 
{
  shared_ptr<deque<std::shared_ptr<Scorer::ChildScorer>>> childScorers =
      s->getChildren();
  setScorerCalled[0] = true;
  assertEquals(2, childScorers->size());
  shared_ptr<Set<wstring>> terms = unordered_set<wstring>();
  for (auto childScorer : childScorers) {
    shared_ptr<Query> query = childScorer->child->getWeight()->getQuery();
    assertTrue(std::dynamic_pointer_cast<TermQuery>(query) != nullptr);
    shared_ptr<Term> term =
        (std::static_pointer_cast<TermQuery>(query))->getTerm();
    assertEquals(L"field", term->field());
    terms->add(term->text());
  }
  assertEquals(2, terms->size());
  assertTrue(terms->contains(L"a"));
  assertTrue(terms->contains(L"b"));
}

void TestConjunctions::SimpleCollectorAnonymousInnerClass::collect(int doc) {}

bool TestConjunctions::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return true;
}
} // namespace org::apache::lucene::search