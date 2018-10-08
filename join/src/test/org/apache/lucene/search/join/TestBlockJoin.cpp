using namespace std;

#include "TestBlockJoin.h"

namespace org::apache::lucene::search::join
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Store = org::apache::lucene::document::Field::Store;
using IntPoint = org::apache::lucene::document::IntPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using LogDocMergePolicy = org::apache::lucene::index::LogDocMergePolicy;
using MultiFields = org::apache::lucene::index::MultiFields;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using Term = org::apache::lucene::index::Term;
using namespace org::apache::lucene::search;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BasicStats = org::apache::lucene::search::similarities::BasicStats;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using SimilarityBase =
    org::apache::lucene::search::similarities::SimilarityBase;
using Directory = org::apache::lucene::store::Directory;
using BitSet = org::apache::lucene::util::BitSet;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Document> TestBlockJoin::makeResume(const wstring &name,
                                               const wstring &country)
{
  shared_ptr<Document> resume = make_shared<Document>();
  resume->push_back(newStringField(L"docType", L"resume", Field::Store::NO));
  resume->push_back(newStringField(L"name", name, Field::Store::YES));
  resume->push_back(newStringField(L"country", country, Field::Store::NO));
  return resume;
}

shared_ptr<Document> TestBlockJoin::makeJob(const wstring &skill, int year)
{
  shared_ptr<Document> job = make_shared<Document>();
  job->push_back(newStringField(L"skill", skill, Field::Store::YES));
  job->push_back(make_shared<IntPoint>(L"year", year));
  job->push_back(make_shared<StoredField>(L"year", year));
  return job;
}

shared_ptr<Document>
TestBlockJoin::makeQualification(const wstring &qualification, int year)
{
  shared_ptr<Document> job = make_shared<Document>();
  job->push_back(
      newStringField(L"qualification", qualification, Field::Store::YES));
  job->push_back(make_shared<IntPoint>(L"year", year));
  return job;
}

void TestBlockJoin::testExtractTerms() 
{
  shared_ptr<TermQuery> termQuery =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"value"));
  shared_ptr<QueryBitSetProducer> bitSetProducer =
      make_shared<QueryBitSetProducer>(make_shared<MatchNoDocsQuery>());
  shared_ptr<ToParentBlockJoinQuery> toParentBlockJoinQuery =
      make_shared<ToParentBlockJoinQuery>(termQuery, bitSetProducer,
                                          ScoreMode::None);
  shared_ptr<ToChildBlockJoinQuery> toChildBlockJoinQuery =
      make_shared<ToChildBlockJoinQuery>(toParentBlockJoinQuery,
                                         bitSetProducer);

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(
      directory,
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  delete w;
  shared_ptr<IndexReader> indexReader = DirectoryReader::open(directory);
  shared_ptr<IndexSearcher> indexSearcher =
      make_shared<IndexSearcher>(indexReader);

  shared_ptr<Weight> weight =
      toParentBlockJoinQuery->createWeight(indexSearcher, false, 1.0f);
  shared_ptr<Set<std::shared_ptr<Term>>> terms =
      unordered_set<std::shared_ptr<Term>>();
  weight->extractTerms(terms);
  std::deque<std::shared_ptr<Term>> termArr =
      terms->toArray(std::deque<std::shared_ptr<Term>>(0));
  TestUtil::assertEquals(1, termArr.size());

  weight = toChildBlockJoinQuery->createWeight(indexSearcher, false, 1.0f);
  terms = unordered_set<>();
  weight->extractTerms(terms);
  termArr = terms->toArray(std::deque<std::shared_ptr<Term>>(0));
  TestUtil::assertEquals(1, termArr.size());

  delete indexReader;
  delete directory;
}

void TestBlockJoin::testEmptyChildFilter() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<IndexWriterConfig> *const config =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  config->setMergePolicy(NoMergePolicy::INSTANCE);
  // we don't want to merge - since we rely on certain segment setup
  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, config);

  const deque<std::shared_ptr<Document>> docs =
      deque<std::shared_ptr<Document>>();

  docs.push_back(makeJob(L"java", 2007));
  docs.push_back(makeJob(L"python", 2010));
  docs.push_back(makeResume(L"Lisa", L"United Kingdom"));
  w->addDocuments(docs);

  docs.clear();
  docs.push_back(makeJob(L"ruby", 2005));
  docs.push_back(makeJob(L"java", 2006));
  docs.push_back(makeResume(L"Frank", L"United States"));
  w->addDocuments(docs);
  w->commit();

  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  delete w;
  shared_ptr<IndexSearcher> s = newSearcher(r);
  shared_ptr<BitSetProducer> parentsFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"docType", L"resume")));
  CheckJoinIndex::check(r, parentsFilter);

  shared_ptr<BooleanQuery::Builder> childQuery =
      make_shared<BooleanQuery::Builder>();
  childQuery->add(make_shared<BooleanClause>(
      make_shared<TermQuery>(make_shared<Term>(L"skill", L"java")),
      Occur::MUST));
  childQuery->add(make_shared<BooleanClause>(
      IntPoint::newRangeQuery(L"year", 2006, 2011), Occur::MUST));

  shared_ptr<ToParentBlockJoinQuery> childJoinQuery =
      make_shared<ToParentBlockJoinQuery>(childQuery->build(), parentsFilter,
                                          ScoreMode::Avg);

  shared_ptr<BooleanQuery::Builder> fullQuery =
      make_shared<BooleanQuery::Builder>();
  fullQuery->add(make_shared<BooleanClause>(childJoinQuery, Occur::MUST));
  fullQuery->add(make_shared<BooleanClause>(make_shared<MatchAllDocsQuery>(),
                                            Occur::MUST));
  shared_ptr<TopDocs> topDocs = s->search(fullQuery->build(), 2);
  TestUtil::assertEquals(2, topDocs->totalHits);
  TestUtil::assertEquals(asSet({L"Lisa", L"Frank"}),
                         asSet({s->doc(topDocs->scoreDocs[0]->doc)[L"name"],
                                s->doc(topDocs->scoreDocs[1]->doc)[L"name"]}));

  shared_ptr<ParentChildrenBlockJoinQuery> childrenQuery =
      make_shared<ParentChildrenBlockJoinQuery>(
          parentsFilter, childQuery->build(), topDocs->scoreDocs[0]->doc);
  shared_ptr<TopDocs> matchingChildren = s->search(childrenQuery, 1);
  TestUtil::assertEquals(1, matchingChildren->totalHits);
  TestUtil::assertEquals(L"java",
                         s->doc(matchingChildren->scoreDocs[0]->doc)[L"skill"]);

  childrenQuery = make_shared<ParentChildrenBlockJoinQuery>(
      parentsFilter, childQuery->build(), topDocs->scoreDocs[1]->doc);
  matchingChildren = s->search(childrenQuery, 1);
  TestUtil::assertEquals(1, matchingChildren->totalHits);
  TestUtil::assertEquals(L"java",
                         s->doc(matchingChildren->scoreDocs[0]->doc)[L"skill"]);

  delete r;
  delete dir;
}

void TestBlockJoin::testBQShouldJoinedChild() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  const deque<std::shared_ptr<Document>> docs =
      deque<std::shared_ptr<Document>>();

  docs.push_back(makeJob(L"java", 2007));
  docs.push_back(makeJob(L"python", 2010));
  docs.push_back(makeResume(L"Lisa", L"United Kingdom"));
  w->addDocuments(docs);

  docs.clear();
  docs.push_back(makeJob(L"ruby", 2005));
  docs.push_back(makeJob(L"java", 2006));
  docs.push_back(makeResume(L"Frank", L"United States"));
  w->addDocuments(docs);

  shared_ptr<IndexReader> r = w->getReader();
  delete w;
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  // IndexSearcher s = new IndexSearcher(r);

  // Create a filter that defines "parent" documents in the index - in this case
  // resumes
  shared_ptr<BitSetProducer> parentsFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"docType", L"resume")));
  CheckJoinIndex::check(r, parentsFilter);

  // Define child document criteria (finds an example of relevant work
  // experience)
  shared_ptr<BooleanQuery::Builder> childQuery =
      make_shared<BooleanQuery::Builder>();
  childQuery->add(make_shared<BooleanClause>(
      make_shared<TermQuery>(make_shared<Term>(L"skill", L"java")),
      Occur::MUST));
  childQuery->add(make_shared<BooleanClause>(
      IntPoint::newRangeQuery(L"year", 2006, 2011), Occur::MUST));

  // Define parent document criteria (find a resident in the UK)
  shared_ptr<Query> parentQuery =
      make_shared<TermQuery>(make_shared<Term>(L"country", L"United Kingdom"));

  // Wrap the child document query to 'join' any matches
  // up to corresponding parent:
  shared_ptr<ToParentBlockJoinQuery> childJoinQuery =
      make_shared<ToParentBlockJoinQuery>(childQuery->build(), parentsFilter,
                                          ScoreMode::Avg);

  // Combine the parent and nested child queries into a single query for a
  // candidate
  shared_ptr<BooleanQuery::Builder> fullQuery =
      make_shared<BooleanQuery::Builder>();
  fullQuery->add(make_shared<BooleanClause>(parentQuery, Occur::SHOULD));
  fullQuery->add(make_shared<BooleanClause>(childJoinQuery, Occur::SHOULD));

  shared_ptr<TopDocs> *const topDocs = s->search(fullQuery->build(), 2);
  TestUtil::assertEquals(2, topDocs->totalHits);
  TestUtil::assertEquals(asSet({L"Lisa", L"Frank"}),
                         asSet({s->doc(topDocs->scoreDocs[0]->doc)[L"name"],
                                s->doc(topDocs->scoreDocs[1]->doc)[L"name"]}));

  shared_ptr<ParentChildrenBlockJoinQuery> childrenQuery =
      make_shared<ParentChildrenBlockJoinQuery>(
          parentsFilter, childQuery->build(), topDocs->scoreDocs[0]->doc);
  shared_ptr<TopDocs> matchingChildren = s->search(childrenQuery, 1);
  TestUtil::assertEquals(1, matchingChildren->totalHits);
  TestUtil::assertEquals(L"java",
                         s->doc(matchingChildren->scoreDocs[0]->doc)[L"skill"]);

  childrenQuery = make_shared<ParentChildrenBlockJoinQuery>(
      parentsFilter, childQuery->build(), topDocs->scoreDocs[1]->doc);
  matchingChildren = s->search(childrenQuery, 1);
  TestUtil::assertEquals(1, matchingChildren->totalHits);
  TestUtil::assertEquals(L"java",
                         s->doc(matchingChildren->scoreDocs[0]->doc)[L"skill"]);

  delete r;
  delete dir;
}

void TestBlockJoin::testSimple() 
{

  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  const deque<std::shared_ptr<Document>> docs =
      deque<std::shared_ptr<Document>>();

  docs.push_back(makeJob(L"java", 2007));
  docs.push_back(makeJob(L"python", 2010));
  docs.push_back(makeResume(L"Lisa", L"United Kingdom"));
  w->addDocuments(docs);

  docs.clear();
  docs.push_back(makeJob(L"ruby", 2005));
  docs.push_back(makeJob(L"java", 2006));
  docs.push_back(makeResume(L"Frank", L"United States"));
  w->addDocuments(docs);

  shared_ptr<IndexReader> r = w->getReader();
  delete w;
  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  // Create a filter that defines "parent" documents in the index - in this case
  // resumes
  shared_ptr<BitSetProducer> parentsFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"docType", L"resume")));
  CheckJoinIndex::check(r, parentsFilter);

  // Define child document criteria (finds an example of relevant work
  // experience)
  shared_ptr<BooleanQuery::Builder> childQuery =
      make_shared<BooleanQuery::Builder>();
  childQuery->add(make_shared<BooleanClause>(
      make_shared<TermQuery>(make_shared<Term>(L"skill", L"java")),
      Occur::MUST));
  childQuery->add(make_shared<BooleanClause>(
      IntPoint::newRangeQuery(L"year", 2006, 2011), Occur::MUST));

  // Define parent document criteria (find a resident in the UK)
  shared_ptr<Query> parentQuery =
      make_shared<TermQuery>(make_shared<Term>(L"country", L"United Kingdom"));

  // Wrap the child document query to 'join' any matches
  // up to corresponding parent:
  shared_ptr<ToParentBlockJoinQuery> childJoinQuery =
      make_shared<ToParentBlockJoinQuery>(childQuery->build(), parentsFilter,
                                          ScoreMode::Avg);

  // Combine the parent and nested child queries into a single query for a
  // candidate
  shared_ptr<BooleanQuery::Builder> fullQuery =
      make_shared<BooleanQuery::Builder>();
  fullQuery->add(make_shared<BooleanClause>(parentQuery, Occur::MUST));
  fullQuery->add(make_shared<BooleanClause>(childJoinQuery, Occur::MUST));

  CheckHits::checkHitCollector(random(), fullQuery->build(), L"country", s,
                               std::deque<int>{2});

  shared_ptr<TopDocs> topDocs = s->search(fullQuery->build(), 1);

  // assertEquals(1, results.totalHitCount);
  TestUtil::assertEquals(1, topDocs->totalHits);
  shared_ptr<Document> parentDoc = s->doc(topDocs->scoreDocs[0]->doc);
  TestUtil::assertEquals(L"Lisa", parentDoc[L"name"]);

  shared_ptr<ParentChildrenBlockJoinQuery> childrenQuery =
      make_shared<ParentChildrenBlockJoinQuery>(
          parentsFilter, childQuery->build(), topDocs->scoreDocs[0]->doc);
  shared_ptr<TopDocs> matchingChildren = s->search(childrenQuery, 1);
  TestUtil::assertEquals(1, matchingChildren->totalHits);
  TestUtil::assertEquals(L"java",
                         s->doc(matchingChildren->scoreDocs[0]->doc)[L"skill"]);

  // System.out.println("TEST: now test up");

  // Now join "up" (map_obj parent hits to child docs) instead...:
  shared_ptr<ToChildBlockJoinQuery> parentJoinQuery =
      make_shared<ToChildBlockJoinQuery>(parentQuery, parentsFilter);
  shared_ptr<BooleanQuery::Builder> fullChildQuery =
      make_shared<BooleanQuery::Builder>();
  fullChildQuery->add(make_shared<BooleanClause>(parentJoinQuery, Occur::MUST));
  fullChildQuery->add(
      make_shared<BooleanClause>(childQuery->build(), Occur::MUST));

  // System.out.println("FULL: " + fullChildQuery);
  shared_ptr<TopDocs> hits = s->search(fullChildQuery->build(), 10);
  TestUtil::assertEquals(1, hits->totalHits);
  shared_ptr<Document> childDoc = s->doc(hits->scoreDocs[0]->doc);
  // System.out.println("CHILD = " + childDoc + " docID=" +
  // hits.scoreDocs[0].doc);
  TestUtil::assertEquals(L"java", childDoc[L"skill"]);
  TestUtil::assertEquals(2007, childDoc->getField(L"year")->numericValue());
  TestUtil::assertEquals(
      L"Lisa",
      getParentDoc(r, parentsFilter, hits->scoreDocs[0]->doc)[L"name"]);

  // Test with filter on child docs:
  fullChildQuery->add(
      make_shared<TermQuery>(make_shared<Term>(L"skill", L"foosball")),
      Occur::FILTER);
  TestUtil::assertEquals(0, s->search(fullChildQuery->build(), 1)->totalHits);

  delete r;
  delete dir;
}

shared_ptr<Query> TestBlockJoin::skill(const wstring &skill)
{
  return make_shared<TermQuery>(make_shared<Term>(L"skill", skill));
}

void TestBlockJoin::testSimpleFilter() 
{

  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  const deque<std::shared_ptr<Document>> docs =
      deque<std::shared_ptr<Document>>();
  docs.push_back(makeJob(L"java", 2007));
  docs.push_back(makeJob(L"python", 2010));
  Collections::shuffle(docs, random());
  docs.push_back(makeResume(L"Lisa", L"United Kingdom"));

  const deque<std::shared_ptr<Document>> docs2 =
      deque<std::shared_ptr<Document>>();
  docs2.push_back(makeJob(L"ruby", 2005));
  docs2.push_back(makeJob(L"java", 2006));
  Collections::shuffle(docs2, random());
  docs2.push_back(makeResume(L"Frank", L"United States"));

  addSkillless(w);
  bool turn = random()->nextBoolean();
  w->addDocuments(turn ? docs : docs2);

  addSkillless(w);

  w->addDocuments(!turn ? docs : docs2);

  addSkillless(w);

  shared_ptr<IndexReader> r = w->getReader();
  delete w;
  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  // Create a filter that defines "parent" documents in the index - in this case
  // resumes
  shared_ptr<BitSetProducer> parentsFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"docType", L"resume")));
  CheckJoinIndex::check(r, parentsFilter);

  // Define child document criteria (finds an example of relevant work
  // experience)
  shared_ptr<BooleanQuery::Builder> childQuery =
      make_shared<BooleanQuery::Builder>();
  childQuery->add(make_shared<BooleanClause>(
      make_shared<TermQuery>(make_shared<Term>(L"skill", L"java")),
      Occur::MUST));
  childQuery->add(make_shared<BooleanClause>(
      IntPoint::newRangeQuery(L"year", 2006, 2011), Occur::MUST));

  // Define parent document criteria (find a resident in the UK)
  shared_ptr<Query> parentQuery =
      make_shared<TermQuery>(make_shared<Term>(L"country", L"United Kingdom"));

  // Wrap the child document query to 'join' any matches
  // up to corresponding parent:
  shared_ptr<ToParentBlockJoinQuery> childJoinQuery =
      make_shared<ToParentBlockJoinQuery>(childQuery->build(), parentsFilter,
                                          ScoreMode::Avg);

  assertEquals(L"no filter - both passed", 2,
               s->search(childJoinQuery, 10)->totalHits);

  shared_ptr<Query> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(childJoinQuery, Occur::MUST)
          ->add(
              make_shared<TermQuery>(make_shared<Term>(L"docType", L"resume")),
              Occur::FILTER)
          ->build();
  assertEquals(L"dummy filter passes everyone ", 2,
               s->search(query, 10)->totalHits);
  query = (make_shared<BooleanQuery::Builder>())
              ->add(childJoinQuery, Occur::MUST)
              ->add(make_shared<TermQuery>(
                        make_shared<Term>(L"docType", L"resume")),
                    Occur::FILTER)
              ->build();
  assertEquals(L"dummy filter passes everyone ", 2,
               s->search(query, 10)->totalHits);

  // not found test
  query =
      (make_shared<BooleanQuery::Builder>())
          ->add(childJoinQuery, Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"country", L"Oz")),
                Occur::FILTER)
          ->build();
  assertEquals(L"noone live there", 0, s->search(query, 1)->totalHits);

  // apply the UK filter by the searcher
  query = (make_shared<BooleanQuery::Builder>())
              ->add(childJoinQuery, Occur::MUST)
              ->add(parentQuery, Occur::FILTER)
              ->build();
  shared_ptr<TopDocs> ukOnly = s->search(query, 1);
  assertEquals(L"has filter - single passed", 1, ukOnly->totalHits);
  TestUtil::assertEquals(L"Lisa",
                         r->document(ukOnly->scoreDocs[0]->doc)[L"name"]);

  query = (make_shared<BooleanQuery::Builder>())
              ->add(childJoinQuery, Occur::MUST)
              ->add(make_shared<TermQuery>(
                        make_shared<Term>(L"country", L"United States")),
                    Occur::FILTER)
              ->build();
  // looking for US candidates
  shared_ptr<TopDocs> usThen = s->search(query, 1);
  assertEquals(L"has filter - single passed", 1, usThen->totalHits);
  TestUtil::assertEquals(L"Frank",
                         r->document(usThen->scoreDocs[0]->doc)[L"name"]);

  shared_ptr<TermQuery> us =
      make_shared<TermQuery>(make_shared<Term>(L"country", L"United States"));
  assertEquals(
      L"@ US we have java and ruby", 2,
      s->search(make_shared<ToChildBlockJoinQuery>(us, parentsFilter), 10)
          ->totalHits);

  query = (make_shared<BooleanQuery::Builder>())
              ->add(make_shared<ToChildBlockJoinQuery>(us, parentsFilter),
                    Occur::MUST)
              ->add(skill(L"java"), Occur::FILTER)
              ->build();
  assertEquals(L"java skills in US", 1, s->search(query, 10)->totalHits);

  shared_ptr<BooleanQuery::Builder> rubyPython =
      make_shared<BooleanQuery::Builder>();
  rubyPython->add(make_shared<TermQuery>(make_shared<Term>(L"skill", L"ruby")),
                  Occur::SHOULD);
  rubyPython->add(
      make_shared<TermQuery>(make_shared<Term>(L"skill", L"python")),
      Occur::SHOULD);
  query = (make_shared<BooleanQuery::Builder>())
              ->add(make_shared<ToChildBlockJoinQuery>(us, parentsFilter),
                    Occur::MUST)
              ->add(rubyPython->build(), Occur::FILTER)
              ->build();
  assertEquals(L"ruby skills in US", 1, s->search(query, 10)->totalHits);

  delete r;
  delete dir;
}

void TestBlockJoin::addSkillless(shared_ptr<RandomIndexWriter> w) throw(
    IOException)
{
  if (random()->nextBoolean()) {
    w->addDocument(makeResume(L"Skillless", random()->nextBoolean()
                                                ? L"United Kingdom"
                                                : L"United States"));
  }
}

shared_ptr<Document>
TestBlockJoin::getParentDoc(shared_ptr<IndexReader> reader,
                            shared_ptr<BitSetProducer> parents,
                            int childDocID) 
{
  const deque<std::shared_ptr<LeafReaderContext>> leaves = reader->leaves();
  constexpr int subIndex = ReaderUtil::subIndex(childDocID, leaves);
  shared_ptr<LeafReaderContext> *const leaf = leaves[subIndex];
  shared_ptr<BitSet> *const bits = parents->getBitSet(leaf);
  return leaf->reader()->document(bits->nextSetBit(childDocID - leaf->docBase));
}

void TestBlockJoin::testBoostBug() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;
  shared_ptr<IndexSearcher> s = newSearcher(r);

  shared_ptr<ToParentBlockJoinQuery> q = make_shared<ToParentBlockJoinQuery>(
      make_shared<MatchNoDocsQuery>(),
      make_shared<QueryBitSetProducer>(make_shared<MatchAllDocsQuery>()),
      ScoreMode::Avg);
  QueryUtils::check(random(), q, s);
  s->search(q, 10);
  shared_ptr<BooleanQuery::Builder> bqB = make_shared<BooleanQuery::Builder>();
  bqB->add(q, BooleanClause::Occur::MUST);
  shared_ptr<BooleanQuery> bq = bqB->build();
  s->search(make_shared<BoostQuery>(bq, 2.0f), 10);
  delete r;
  delete dir;
}

std::deque<std::deque<wstring>>
TestBlockJoin::getRandomFields(int maxUniqueValues)
{

  const std::deque<std::deque<wstring>> fields =
      std::deque<std::deque<wstring>>(TestUtil::nextInt(random(), 2, 4));
  for (int fieldID = 0; fieldID < fields.size(); fieldID++) {
    constexpr int valueCount;
    if (fieldID == 0) {
      valueCount = 2;
    } else {
      valueCount = TestUtil::nextInt(random(), 1, maxUniqueValues);
    }

    const std::deque<wstring> values = fields[fieldID] =
        std::deque<wstring>(valueCount);
    for (int i = 0; i < valueCount; i++) {
      values[i] = TestUtil::randomRealisticUnicodeString(random());
      // values[i] = TestUtil.randomSimpleString(random());
    }
  }

  return fields;
}

shared_ptr<Term> TestBlockJoin::randomParentTerm(std::deque<wstring> &values)
{
  return make_shared<Term>(L"parent0",
                           values[random()->nextInt(values.size())]);
}

shared_ptr<Term> TestBlockJoin::randomChildTerm(std::deque<wstring> &values)
{
  return make_shared<Term>(L"child0", values[random()->nextInt(values.size())]);
}

shared_ptr<Sort> TestBlockJoin::getRandomSort(const wstring &prefix,
                                              int numFields)
{
  const deque<std::shared_ptr<SortField>> sortFields =
      deque<std::shared_ptr<SortField>>();
  // TODO: sometimes sort by score; problem is scores are
  // not comparable across the two indices
  // sortFields.add(SortField.FIELD_SCORE);
  if (random()->nextBoolean()) {
    sortFields.push_back(make_shared<SortField>(
        prefix + random()->nextInt(numFields), SortField::Type::STRING,
        random()->nextBoolean()));
  } else if (random()->nextBoolean()) {
    sortFields.push_back(make_shared<SortField>(
        prefix + random()->nextInt(numFields), SortField::Type::STRING,
        random()->nextBoolean()));
    sortFields.push_back(make_shared<SortField>(
        prefix + random()->nextInt(numFields), SortField::Type::STRING,
        random()->nextBoolean()));
  }
  // Break ties:
  sortFields.push_back(
      make_shared<SortField>(prefix + L"ID", SortField::Type::INT));
  return make_shared<Sort>(sortFields.toArray(
      std::deque<std::shared_ptr<SortField>>(sortFields.size())));
}

void TestBlockJoin::testRandom() 
{
  // We build two indices at once: one normalized (which
  // ToParentBlockJoinQuery/Collector,
  // ToChildBlockJoinQuery can query) and the other w/
  // the same docs, just fully denormalized:
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<Directory> *const joinDir = newDirectory();

  constexpr int maxNumChildrenPerParent = 20;
  constexpr int numParentDocs = TestUtil::nextInt(
      random(), 100 * RANDOM_MULTIPLIER, 300 * RANDOM_MULTIPLIER);
  // final int numParentDocs = 30;

  // Values for parent fields:
  const std::deque<std::deque<wstring>> parentFields =
      getRandomFields(numParentDocs / 2);
  // Values for child fields:
  const std::deque<std::deque<wstring>> childFields =
      getRandomFields(numParentDocs);

  constexpr bool doDeletes = random()->nextBoolean();
  const deque<int> toDelete = deque<int>();

  // TODO: parallel star join, nested join cases too!
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<RandomIndexWriter> *const joinW =
      make_shared<RandomIndexWriter>(random(), joinDir);
  for (int parentDocID = 0; parentDocID < numParentDocs; parentDocID++) {
    shared_ptr<Document> parentDoc = make_shared<Document>();
    shared_ptr<Document> parentJoinDoc = make_shared<Document>();
    shared_ptr<Field> id = make_shared<StoredField>(L"parentID", parentDocID);
    parentDoc->push_back(id);
    parentJoinDoc->push_back(id);
    parentJoinDoc->push_back(
        newStringField(L"isParent", L"x", Field::Store::NO));
    id = make_shared<NumericDocValuesField>(L"parentID", parentDocID);
    parentDoc->push_back(id);
    parentJoinDoc->push_back(id);
    parentJoinDoc->push_back(
        newStringField(L"isParent", L"x", Field::Store::NO));
    for (int field = 0; field < parentFields.size(); field++) {
      if (random()->nextDouble() < 0.9) {
        wstring s =
            parentFields[field][random()->nextInt(parentFields[field].size())];
        shared_ptr<Field> f =
            newStringField(L"parent" + to_wstring(field), s, Field::Store::NO);
        parentDoc->push_back(f);
        parentJoinDoc->push_back(f);

        f = make_shared<SortedDocValuesField>(L"parent" + to_wstring(field),
                                              make_shared<BytesRef>(s));
        parentDoc->push_back(f);
        parentJoinDoc->push_back(f);
      }
    }

    if (doDeletes) {
      parentDoc->push_back(make_shared<IntPoint>(L"blockID", parentDocID));
      parentJoinDoc->push_back(make_shared<IntPoint>(L"blockID", parentDocID));
    }

    const deque<std::shared_ptr<Document>> joinDocs =
        deque<std::shared_ptr<Document>>();

    if (VERBOSE) {
      shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
      sb->append(L"parentID=")->append(parentDoc[L"parentID"]);
      for (int fieldID = 0; fieldID < parentFields.size(); fieldID++) {
        wstring s = parentDoc[L"parent" + to_wstring(fieldID)];
        if (s != L"") {
          sb->append(L" parent" + to_wstring(fieldID) + L"=" + s);
        }
      }
      wcout << L"  " << sb->toString() << endl;
    }

    constexpr int numChildDocs =
        TestUtil::nextInt(random(), 1, maxNumChildrenPerParent);
    for (int childDocID = 0; childDocID < numChildDocs; childDocID++) {
      // Denormalize: copy all parent fields into child doc:
      shared_ptr<Document> childDoc = TestUtil::cloneDocument(parentDoc);
      shared_ptr<Document> joinChildDoc = make_shared<Document>();
      joinDocs.push_back(joinChildDoc);

      shared_ptr<Field> childID =
          make_shared<StoredField>(L"childID", childDocID);
      childDoc->push_back(childID);
      joinChildDoc->push_back(childID);
      childID = make_shared<NumericDocValuesField>(L"childID", childDocID);
      childDoc->push_back(childID);
      joinChildDoc->push_back(childID);

      for (int childFieldID = 0; childFieldID < childFields.size();
           childFieldID++) {
        if (random()->nextDouble() < 0.9) {
          wstring s =
              childFields[childFieldID]
                         [random()->nextInt(childFields[childFieldID].size())];
          shared_ptr<Field> f = newStringField(
              L"child" + to_wstring(childFieldID), s, Field::Store::NO);
          childDoc->push_back(f);
          joinChildDoc->push_back(f);

          f = make_shared<SortedDocValuesField>(
              L"child" + to_wstring(childFieldID), make_shared<BytesRef>(s));
          childDoc->push_back(f);
          joinChildDoc->push_back(f);
        }
      }

      if (VERBOSE) {
        shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
        sb->append(L"childID=")->append(joinChildDoc[L"childID"]);
        for (int fieldID = 0; fieldID < childFields.size(); fieldID++) {
          wstring s = joinChildDoc[L"child" + to_wstring(fieldID)];
          if (s != L"") {
            sb->append(L" child" + to_wstring(fieldID) + L"=" + s);
          }
        }
        wcout << L"    " << sb->toString() << endl;
      }

      if (doDeletes) {
        joinChildDoc->push_back(make_shared<IntPoint>(L"blockID", parentDocID));
      }

      w->addDocument(childDoc);
    }

    // Parent last:
    joinDocs.push_back(parentJoinDoc);
    joinW->addDocuments(joinDocs);

    if (doDeletes && random()->nextInt(30) == 7) {
      toDelete.push_back(parentDocID);
    }
  }

  if (!toDelete.empty()) {
    shared_ptr<Query> query = IntPoint::newSetQuery(L"blockID", toDelete);
    w->deleteDocuments(query);
    joinW->deleteDocuments(query);
  }

  shared_ptr<IndexReader> *const r = w->getReader();
  delete w;
  shared_ptr<IndexReader> *const joinR = joinW->getReader();
  delete joinW;

  if (VERBOSE) {
    wcout << L"TEST: reader=" << r << endl;
    wcout << L"TEST: joinReader=" << joinR << endl;

    shared_ptr<Bits> liveDocs = MultiFields::getLiveDocs(joinR);
    for (int docIDX = 0; docIDX < joinR->maxDoc(); docIDX++) {
      wcout << L"  docID=" << docIDX << L" doc=" << joinR->document(docIDX)
            << L" deleted?="
            << (liveDocs != nullptr && liveDocs->get(docIDX) == false) << endl;
    }
    shared_ptr<PostingsEnum> parents = MultiFields::getTermDocsEnum(
        joinR, L"isParent", make_shared<BytesRef>(L"x"));
    wcout << L"parent docIDs:" << endl;
    while (parents->nextDoc() != PostingsEnum::NO_MORE_DOCS) {
      wcout << L"  " << parents->docID() << endl;
    }
  }

  shared_ptr<IndexSearcher> *const s = newSearcher(r, false);

  shared_ptr<IndexSearcher> *const joinS = newSearcher(joinR);

  shared_ptr<BitSetProducer> *const parentsFilter =
      make_shared<QueryBitSetProducer>(
          make_shared<TermQuery>(make_shared<Term>(L"isParent", L"x")));
  CheckJoinIndex::check(joinS->getIndexReader(), parentsFilter);

  constexpr int iters = 200 * RANDOM_MULTIPLIER;

  for (int iter = 0; iter < iters; iter++) {
    if (VERBOSE) {
      wcout << L"TEST: iter=" << (1 << iter) << L" of " << iters << endl;
    }

    shared_ptr<Query> childQuery;
    if (random()->nextInt(3) == 2) {
      constexpr int childFieldID = random()->nextInt(childFields.size());
      childQuery = make_shared<TermQuery>(make_shared<Term>(
          L"child" + to_wstring(childFieldID),
          childFields[childFieldID]
                     [random()->nextInt(childFields[childFieldID].size())]));
    } else if (random()->nextInt(3) == 2) {
      shared_ptr<BooleanQuery::Builder> bq =
          make_shared<BooleanQuery::Builder>();
      constexpr int numClauses = TestUtil::nextInt(random(), 2, 4);
      bool didMust = false;
      for (int clauseIDX = 0; clauseIDX < numClauses; clauseIDX++) {
        shared_ptr<Query> clause;
        BooleanClause::Occur occur;
        if (!didMust && random()->nextBoolean()) {
          occur = random()->nextBoolean() ? BooleanClause::Occur::MUST
                                          : BooleanClause::Occur::MUST_NOT;
          clause = make_shared<TermQuery>(randomChildTerm(childFields[0]));
          didMust = true;
        } else {
          occur = BooleanClause::Occur::SHOULD;
          constexpr int childFieldID =
              TestUtil::nextInt(random(), 1, childFields.size() - 1);
          clause = make_shared<TermQuery>(
              make_shared<Term>(L"child" + to_wstring(childFieldID),
                                childFields[childFieldID][random()->nextInt(
                                    childFields[childFieldID].size())]));
        }
        bq->add(clause, occur);
      }
      childQuery = bq->build();
    } else {
      shared_ptr<BooleanQuery::Builder> bq =
          make_shared<BooleanQuery::Builder>();

      bq->add(make_shared<TermQuery>(randomChildTerm(childFields[0])),
              BooleanClause::Occur::MUST);
      constexpr int childFieldID =
          TestUtil::nextInt(random(), 1, childFields.size() - 1);
      bq->add(make_shared<TermQuery>(
                  make_shared<Term>(L"child" + to_wstring(childFieldID),
                                    childFields[childFieldID][random()->nextInt(
                                        childFields[childFieldID].size())])),
              random()->nextBoolean() ? BooleanClause::Occur::MUST
                                      : BooleanClause::Occur::MUST_NOT);
      childQuery = bq->build();
    }
    if (random()->nextBoolean()) {
      childQuery = make_shared<RandomApproximationQuery>(childQuery, random());
    }

    constexpr ScoreMode agg =
        ScoreMode::values()[random()->nextInt(ScoreMode::values()->length)];
    shared_ptr<ToParentBlockJoinQuery> *const childJoinQuery =
        make_shared<ToParentBlockJoinQuery>(childQuery, parentsFilter, agg);

    // To run against the block-join index:
    shared_ptr<Query> *const parentJoinQuery;

    // Same query as parentJoinQuery, but to run against
    // the fully denormalized index (so we can compare
    // results):
    shared_ptr<Query> *const parentQuery;

    if (random()->nextBoolean()) {
      parentQuery = childQuery;
      parentJoinQuery = childJoinQuery;
    } else {
      // AND parent field w/ child field
      shared_ptr<BooleanQuery::Builder> *const bq =
          make_shared<BooleanQuery::Builder>();
      shared_ptr<Term> *const parentTerm = randomParentTerm(parentFields[0]);
      if (random()->nextBoolean()) {
        bq->add(childJoinQuery, BooleanClause::Occur::MUST);
        bq->add(make_shared<TermQuery>(parentTerm), BooleanClause::Occur::MUST);
      } else {
        bq->add(make_shared<TermQuery>(parentTerm), BooleanClause::Occur::MUST);
        bq->add(childJoinQuery, BooleanClause::Occur::MUST);
      }

      shared_ptr<BooleanQuery::Builder> *const bq2 =
          make_shared<BooleanQuery::Builder>();
      if (random()->nextBoolean()) {
        bq2->add(childQuery, BooleanClause::Occur::MUST);
        bq2->add(make_shared<TermQuery>(parentTerm),
                 BooleanClause::Occur::MUST);
      } else {
        bq2->add(make_shared<TermQuery>(parentTerm),
                 BooleanClause::Occur::MUST);
        bq2->add(childQuery, BooleanClause::Occur::MUST);
      }
      parentJoinQuery = bq->build();
      parentQuery = bq2->build();
    }

    shared_ptr<Sort> *const parentSort =
        getRandomSort(L"parent", parentFields.size());
    shared_ptr<Sort> *const childSort =
        getRandomSort(L"child", childFields.size());

    if (VERBOSE) {
      wcout << L"\nTEST: query=" << parentQuery << L" joinQuery="
            << parentJoinQuery << L" parentSort=" << parentSort
            << L" childSort=" << childSort << endl;
    }

    // Merge both sorts:
    const deque<std::shared_ptr<SortField>> sortFields =
        deque<std::shared_ptr<SortField>>(
            Arrays::asList(parentSort->getSort()));
    sortFields.addAll(Arrays::asList(childSort->getSort()));
    shared_ptr<Sort> *const parentAndChildSort =
        make_shared<Sort>(sortFields.toArray(
            std::deque<std::shared_ptr<SortField>>(sortFields.size())));

    shared_ptr<TopDocs> *const results =
        s->search(parentQuery, r->numDocs(), parentAndChildSort);

    if (VERBOSE) {
      wcout << L"\nTEST: normal index gets " << results->totalHits
            << L" hits; sort=" << parentAndChildSort << endl;
      std::deque<std::shared_ptr<ScoreDoc>> hits = results->scoreDocs;
      for (int hitIDX = 0; hitIDX < hits.size(); hitIDX++) {
        shared_ptr<Document> *const doc = s->doc(hits[hitIDX]->doc);
        // System.out.println("  score=" + hits[hitIDX].score + " parentID=" +
        // doc.get("parentID") + " childID=" + doc.get("childID") + " (docID=" +
        // hits[hitIDX].doc + ")");
        wcout << L"  parentID=" << doc[L"parentID"] << L" childID="
              << doc[L"childID"] << L" (docID=" << hits[hitIDX]->doc << L")"
              << endl;
        shared_ptr<FieldDoc> fd =
            std::static_pointer_cast<FieldDoc>(hits[hitIDX]);
        if (fd->fields.size() > 0) {
          wcout << L"    " << fd->fields.size() << L" sort values: ";
          for (auto o : fd->fields) {
            if (std::dynamic_pointer_cast<BytesRef>(o) != nullptr) {
              wcout << (std::static_pointer_cast<BytesRef>(o))->utf8ToString()
                    << L" ";
            } else {
              wcout << o << L" ";
            }
          }
          wcout << endl;
        }
      }
    }

    shared_ptr<TopDocs> joinedResults =
        joinS->search(parentJoinQuery, numParentDocs);
    shared_ptr<SortedMap<int, std::shared_ptr<TopDocs>>> joinResults =
        map_obj<int, std::shared_ptr<TopDocs>>();
    for (auto parentHit : joinedResults->scoreDocs) {
      shared_ptr<ParentChildrenBlockJoinQuery> childrenQuery =
          make_shared<ParentChildrenBlockJoinQuery>(parentsFilter, childQuery,
                                                    parentHit->doc);
      shared_ptr<TopDocs> childTopDocs =
          joinS->search(childrenQuery, maxNumChildrenPerParent, childSort);
      shared_ptr<Document> *const parentDoc = joinS->doc(parentHit->doc);
      joinResults->put(static_cast<Integer>(parentDoc[L"parentID"]),
                       childTopDocs);
    }

    constexpr int hitsPerGroup = TestUtil::nextInt(random(), 1, 20);
    // final int hitsPerGroup = 100;

    if (VERBOSE) {
      wcout << L"\nTEST: block join index gets "
            << (joinResults == nullptr ? 0 : joinResults->size())
            << L" groups; hitsPerGroup=" << hitsPerGroup << endl;
      if (joinResults != nullptr) {
        for (auto entry : joinResults) {
          wcout << L"  group parentID=" << entry.first << L" (docID="
                << entry.first << L")" << endl;
          for (shared_ptr<ScoreDoc> childHit : entry.second::scoreDocs) {
            shared_ptr<Document> *const doc = joinS->doc(childHit->doc);
            //              System.out.println("    score=" + childHit.score + "
            //              childID=" + doc.get("childID") + " (docID=" +
            //              childHit.doc + ")");
            wcout << L"    childID=" << doc[L"childID"] << L" child0="
                  << doc[L"child0"] << L" (docID=" << childHit->doc << L")"
                  << endl;
          }
        }
      }
    }

    if (results->totalHits == 0) {
      TestUtil::assertEquals(0, joinResults->size());
    } else {
      compareHits(r, joinR, results, joinResults);
      shared_ptr<TopDocs> b = joinS->search(childJoinQuery, 10);
      for (auto hit : b->scoreDocs) {
        shared_ptr<Explanation> explanation =
            joinS->explain(childJoinQuery, hit->doc);
        shared_ptr<Document> document = joinS->doc(hit->doc - 1);
        int childId = static_cast<Integer>(document[L"childID"]);
        // System.out.println("  hit docID=" + hit.doc + " childId=" + childId +
        // " parentId=" + document.get("parentID"));
        assertTrue(explanation->isMatch());
        assertEquals(hit->score, explanation->getValue(), 0.0f);
        shared_ptr<Matcher> m =
            Pattern::compile(L"Score based on ([0-9]+) child docs in range "
                             L"from ([0-9]+) to ([0-9]+), best match:")
                .matcher(explanation->getDescription());
        assertTrue(L"Block Join description not matches", m->matches());
        assertTrue(L"Matched children not positive",
                   static_cast<Integer>(m->group(1)) > 0);
        assertEquals(L"Wrong child range start", hit->doc - 1 - childId,
                     static_cast<Integer>(m->group(2)));
        assertEquals(L"Wrong child range end", hit->doc - 1,
                     static_cast<Integer>(m->group(3)));
        shared_ptr<Explanation> childWeightExplanation =
            explanation->getDetails()[0];
        if (L"sum of:" == childWeightExplanation->getDescription()) {
          childWeightExplanation = childWeightExplanation->getDetails()[0];
        }
        assertTrue(
            L"Wrong child weight description",
            StringHelper::startsWith(childWeightExplanation->getDescription(),
                                     L"weight(child"));
      }
    }

    // Test joining in the opposite direction (parent to
    // child):

    // Get random query against parent documents:
    shared_ptr<Query> *const parentQuery2;
    if (random()->nextInt(3) == 2) {
      constexpr int fieldID = random()->nextInt(parentFields.size());
      parentQuery2 = make_shared<TermQuery>(make_shared<Term>(
          L"parent" + to_wstring(fieldID),
          parentFields[fieldID]
                      [random()->nextInt(parentFields[fieldID].size())]));
    } else if (random()->nextInt(3) == 2) {
      shared_ptr<BooleanQuery::Builder> bq =
          make_shared<BooleanQuery::Builder>();
      constexpr int numClauses = TestUtil::nextInt(random(), 2, 4);
      bool didMust = false;
      for (int clauseIDX = 0; clauseIDX < numClauses; clauseIDX++) {
        shared_ptr<Query> clause;
        BooleanClause::Occur occur;
        if (!didMust && random()->nextBoolean()) {
          occur = random()->nextBoolean() ? BooleanClause::Occur::MUST
                                          : BooleanClause::Occur::MUST_NOT;
          clause = make_shared<TermQuery>(randomParentTerm(parentFields[0]));
          didMust = true;
        } else {
          occur = BooleanClause::Occur::SHOULD;
          constexpr int fieldID =
              TestUtil::nextInt(random(), 1, parentFields.size() - 1);
          clause = make_shared<TermQuery>(make_shared<Term>(
              L"parent" + to_wstring(fieldID),
              parentFields[fieldID]
                          [random()->nextInt(parentFields[fieldID].size())]));
        }
        bq->add(clause, occur);
      }
      parentQuery2 = bq->build();
    } else {
      shared_ptr<BooleanQuery::Builder> bq =
          make_shared<BooleanQuery::Builder>();

      bq->add(make_shared<TermQuery>(randomParentTerm(parentFields[0])),
              BooleanClause::Occur::MUST);
      constexpr int fieldID =
          TestUtil::nextInt(random(), 1, parentFields.size() - 1);
      bq->add(make_shared<TermQuery>(
                  make_shared<Term>(L"parent" + to_wstring(fieldID),
                                    parentFields[fieldID][random()->nextInt(
                                        parentFields[fieldID].size())])),
              random()->nextBoolean() ? BooleanClause::Occur::MUST
                                      : BooleanClause::Occur::MUST_NOT);
      parentQuery2 = bq->build();
    }

    if (VERBOSE) {
      wcout << L"\nTEST: top down: parentQuery2=" << parentQuery2 << endl;
    }

    // Maps parent query to child docs:
    shared_ptr<ToChildBlockJoinQuery> *const parentJoinQuery2 =
        make_shared<ToChildBlockJoinQuery>(parentQuery2, parentsFilter);

    // To run against the block-join index:
    shared_ptr<Query> childJoinQuery2;

    // Same query as parentJoinQuery, but to run against
    // the fully denormalized index (so we can compare
    // results):
    shared_ptr<Query> childQuery2;

    if (random()->nextBoolean()) {
      childQuery2 = parentQuery2;
      childJoinQuery2 = parentJoinQuery2;
    } else {
      shared_ptr<Term> *const childTerm = randomChildTerm(childFields[0]);
      if (random()->nextBoolean()) { // filtered case
        childJoinQuery2 = parentJoinQuery2;
        childJoinQuery2 =
            (make_shared<BooleanQuery::Builder>())
                ->add(childJoinQuery2, Occur::MUST)
                ->add(make_shared<TermQuery>(childTerm), Occur::FILTER)
                ->build();
      } else {
        // AND child field w/ parent query:
        shared_ptr<BooleanQuery::Builder> *const bq =
            make_shared<BooleanQuery::Builder>();
        if (random()->nextBoolean()) {
          bq->add(parentJoinQuery2, BooleanClause::Occur::MUST);
          bq->add(make_shared<TermQuery>(childTerm),
                  BooleanClause::Occur::MUST);
        } else {
          bq->add(make_shared<TermQuery>(childTerm),
                  BooleanClause::Occur::MUST);
          bq->add(parentJoinQuery2, BooleanClause::Occur::MUST);
        }
        childJoinQuery2 = bq->build();
      }

      if (random()->nextBoolean()) { // filtered case
        childQuery2 = parentQuery2;
        childQuery2 =
            (make_shared<BooleanQuery::Builder>())
                ->add(childQuery2, Occur::MUST)
                ->add(make_shared<TermQuery>(childTerm), Occur::FILTER)
                ->build();
      } else {
        shared_ptr<BooleanQuery::Builder> *const bq2 =
            make_shared<BooleanQuery::Builder>();
        if (random()->nextBoolean()) {
          bq2->add(parentQuery2, BooleanClause::Occur::MUST);
          bq2->add(make_shared<TermQuery>(childTerm),
                   BooleanClause::Occur::MUST);
        } else {
          bq2->add(make_shared<TermQuery>(childTerm),
                   BooleanClause::Occur::MUST);
          bq2->add(parentQuery2, BooleanClause::Occur::MUST);
        }
        childQuery2 = bq2->build();
      }
    }

    shared_ptr<Sort> *const childSort2 =
        getRandomSort(L"child", childFields.size());

    // Search denormalized index:
    if (VERBOSE) {
      wcout << L"TEST: run top down query=" << childQuery2 << L" sort="
            << childSort2 << endl;
    }
    shared_ptr<TopDocs> *const results2 =
        s->search(childQuery2, r->numDocs(), childSort2);
    if (VERBOSE) {
      wcout << L"  " << results2->totalHits << L" totalHits:" << endl;
      for (auto sd : results2->scoreDocs) {
        shared_ptr<Document> *const doc = s->doc(sd->doc);
        wcout << L"  childID=" << doc[L"childID"] << L" parentID="
              << doc[L"parentID"] << L" docID=" << sd->doc << endl;
      }
    }

    // Search join index:
    if (VERBOSE) {
      wcout << L"TEST: run top down join query=" << childJoinQuery2 << L" sort="
            << childSort2 << endl;
    }
    shared_ptr<TopDocs> joinResults2 =
        joinS->search(childJoinQuery2, joinR->numDocs(), childSort2);
    if (VERBOSE) {
      wcout << L"  " << joinResults2->totalHits << L" totalHits:" << endl;
      for (auto sd : joinResults2->scoreDocs) {
        shared_ptr<Document> *const doc = joinS->doc(sd->doc);
        shared_ptr<Document> *const parentDoc =
            getParentDoc(joinR, parentsFilter, sd->doc);
        wcout << L"  childID=" << doc[L"childID"] << L" parentID="
              << parentDoc[L"parentID"] << L" docID=" << sd->doc << endl;
      }
    }

    compareChildHits(r, joinR, results2, joinResults2);
  }

  delete r;
  delete joinR;
  delete dir;
  delete joinDir;
}

void TestBlockJoin::compareChildHits(
    shared_ptr<IndexReader> r, shared_ptr<IndexReader> joinR,
    shared_ptr<TopDocs> results,
    shared_ptr<TopDocs> joinResults) 
{
  TestUtil::assertEquals(results->totalHits, joinResults->totalHits);
  TestUtil::assertEquals(results->scoreDocs.size(),
                         joinResults->scoreDocs.size());
  for (int hitCount = 0; hitCount < results->scoreDocs.size(); hitCount++) {
    shared_ptr<ScoreDoc> hit = results->scoreDocs[hitCount];
    shared_ptr<ScoreDoc> joinHit = joinResults->scoreDocs[hitCount];
    shared_ptr<Document> doc1 = r->document(hit->doc);
    shared_ptr<Document> doc2 = joinR->document(joinHit->doc);
    assertEquals(L"hit " + to_wstring(hitCount) + L" differs", doc1[L"childID"],
                 doc2[L"childID"]);
    // don't compare scores -- they are expected to differ

    assertTrue(std::dynamic_pointer_cast<FieldDoc>(hit) != nullptr);
    assertTrue(std::dynamic_pointer_cast<FieldDoc>(joinHit) != nullptr);

    shared_ptr<FieldDoc> hit0 = std::static_pointer_cast<FieldDoc>(hit);
    shared_ptr<FieldDoc> joinHit0 = std::static_pointer_cast<FieldDoc>(joinHit);
    assertArrayEquals(hit0->fields, joinHit0->fields);
  }
}

void TestBlockJoin::compareHits(shared_ptr<IndexReader> r,
                                shared_ptr<IndexReader> joinR,
                                shared_ptr<TopDocs> controlHits,
                                unordered_map<int, std::shared_ptr<TopDocs>>
                                    &joinResults) 
{
  int currentParentID = -1;
  int childHitSlot = 0;
  shared_ptr<TopDocs> childHits =
      make_shared<TopDocs>(0, std::deque<std::shared_ptr<ScoreDoc>>(0), 0.0f);
  for (auto controlHit : controlHits->scoreDocs) {
    shared_ptr<Document> controlDoc = r->document(controlHit->doc);
    int parentID = static_cast<Integer>(controlDoc[L"parentID"]);
    if (parentID != currentParentID) {
      TestUtil::assertEquals(childHitSlot, childHits->scoreDocs.size());
      currentParentID = parentID;
      childHitSlot = 0;
      childHits = joinResults[parentID];
    }

    wstring controlChildID = controlDoc[L"childID"];
    shared_ptr<Document> childDoc =
        joinR->document(childHits->scoreDocs[childHitSlot++]->doc);
    wstring childID = childDoc[L"childID"];
    TestUtil::assertEquals(controlChildID, childID);
  }
}

void TestBlockJoin::testMultiChildTypes() 
{

  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  const deque<std::shared_ptr<Document>> docs =
      deque<std::shared_ptr<Document>>();

  docs.push_back(makeJob(L"java", 2007));
  docs.push_back(makeJob(L"python", 2010));
  docs.push_back(makeQualification(L"maths", 1999));
  docs.push_back(makeResume(L"Lisa", L"United Kingdom"));
  w->addDocuments(docs);

  shared_ptr<IndexReader> r = w->getReader();
  delete w;
  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  // Create a filter that defines "parent" documents in the index - in this case
  // resumes
  shared_ptr<BitSetProducer> parentsFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"docType", L"resume")));
  CheckJoinIndex::check(s->getIndexReader(), parentsFilter);

  // Define child document criteria (finds an example of relevant work
  // experience)
  shared_ptr<BooleanQuery::Builder> childJobQuery =
      make_shared<BooleanQuery::Builder>();
  childJobQuery->add(make_shared<BooleanClause>(
      make_shared<TermQuery>(make_shared<Term>(L"skill", L"java")),
      Occur::MUST));
  childJobQuery->add(make_shared<BooleanClause>(
      IntPoint::newRangeQuery(L"year", 2006, 2011), Occur::MUST));

  shared_ptr<BooleanQuery::Builder> childQualificationQuery =
      make_shared<BooleanQuery::Builder>();
  childQualificationQuery->add(make_shared<BooleanClause>(
      make_shared<TermQuery>(make_shared<Term>(L"qualification", L"maths")),
      Occur::MUST));
  childQualificationQuery->add(make_shared<BooleanClause>(
      IntPoint::newRangeQuery(L"year", 1980, 2000), Occur::MUST));

  // Define parent document criteria (find a resident in the UK)
  shared_ptr<Query> parentQuery =
      make_shared<TermQuery>(make_shared<Term>(L"country", L"United Kingdom"));

  // Wrap the child document query to 'join' any matches
  // up to corresponding parent:
  shared_ptr<ToParentBlockJoinQuery> childJobJoinQuery =
      make_shared<ToParentBlockJoinQuery>(childJobQuery->build(), parentsFilter,
                                          ScoreMode::Avg);
  shared_ptr<ToParentBlockJoinQuery> childQualificationJoinQuery =
      make_shared<ToParentBlockJoinQuery>(childQualificationQuery->build(),
                                          parentsFilter, ScoreMode::Avg);

  // Combine the parent and nested child queries into a single query for a
  // candidate
  shared_ptr<BooleanQuery::Builder> fullQuery =
      make_shared<BooleanQuery::Builder>();
  fullQuery->add(make_shared<BooleanClause>(parentQuery, Occur::MUST));
  fullQuery->add(make_shared<BooleanClause>(childJobJoinQuery, Occur::MUST));
  fullQuery->add(
      make_shared<BooleanClause>(childQualificationJoinQuery, Occur::MUST));

  shared_ptr<TopDocs> *const topDocs = s->search(fullQuery->build(), 10);
  TestUtil::assertEquals(1, topDocs->totalHits);
  shared_ptr<Document> parentDoc = s->doc(topDocs->scoreDocs[0]->doc);
  TestUtil::assertEquals(L"Lisa", parentDoc[L"name"]);

  shared_ptr<ParentChildrenBlockJoinQuery> childrenQuery =
      make_shared<ParentChildrenBlockJoinQuery>(
          parentsFilter, childJobQuery->build(), topDocs->scoreDocs[0]->doc);
  shared_ptr<TopDocs> matchingChildren = s->search(childrenQuery, 1);
  TestUtil::assertEquals(1, matchingChildren->totalHits);
  TestUtil::assertEquals(L"java",
                         s->doc(matchingChildren->scoreDocs[0]->doc)[L"skill"]);

  childrenQuery = make_shared<ParentChildrenBlockJoinQuery>(
      parentsFilter, childQualificationQuery->build(),
      topDocs->scoreDocs[0]->doc);
  matchingChildren = s->search(childrenQuery, 1);
  TestUtil::assertEquals(1, matchingChildren->totalHits);
  TestUtil::assertEquals(
      L"maths", s->doc(matchingChildren->scoreDocs[0]->doc)[L"qualification"]);

  delete r;
  delete dir;
}

void TestBlockJoin::testAdvanceSingleParentSingleChild() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> childDoc = make_shared<Document>();
  childDoc->push_back(newStringField(L"child", L"1", Field::Store::NO));
  shared_ptr<Document> parentDoc = make_shared<Document>();
  parentDoc->push_back(newStringField(L"parent", L"1", Field::Store::NO));
  w->addDocuments(Arrays::asList(childDoc, parentDoc));
  shared_ptr<IndexReader> r = w->getReader();
  delete w;
  shared_ptr<IndexSearcher> s = newSearcher(r);
  shared_ptr<Query> tq =
      make_shared<TermQuery>(make_shared<Term>(L"child", L"1"));
  shared_ptr<BitSetProducer> parentFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"parent", L"1")));
  CheckJoinIndex::check(s->getIndexReader(), parentFilter);

  shared_ptr<ToParentBlockJoinQuery> q =
      make_shared<ToParentBlockJoinQuery>(tq, parentFilter, ScoreMode::Avg);
  shared_ptr<Weight> weight = s->createWeight(s->rewrite(q), true, 1);
  shared_ptr<Scorer> sc = weight->scorer(s->getIndexReader()->leaves()[0]);
  TestUtil::assertEquals(1, sc->begin().advance(1));
  delete r;
  delete dir;
}

void TestBlockJoin::testAdvanceSingleParentNoChild() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(make_shared<LogDocMergePolicy>()));
  shared_ptr<Document> parentDoc = make_shared<Document>();
  parentDoc->push_back(newStringField(L"parent", L"1", Field::Store::NO));
  parentDoc->push_back(newStringField(L"isparent", L"yes", Field::Store::NO));
  w->addDocuments(Arrays::asList(parentDoc));

  // Add another doc so scorer is not null
  parentDoc = make_shared<Document>();
  parentDoc->push_back(newStringField(L"parent", L"2", Field::Store::NO));
  parentDoc->push_back(newStringField(L"isparent", L"yes", Field::Store::NO));
  shared_ptr<Document> childDoc = make_shared<Document>();
  childDoc->push_back(newStringField(L"child", L"2", Field::Store::NO));
  w->addDocuments(Arrays::asList(childDoc, parentDoc));

  // Need single seg:
  w->forceMerge(1);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;
  shared_ptr<IndexSearcher> s = newSearcher(r);
  shared_ptr<Query> tq =
      make_shared<TermQuery>(make_shared<Term>(L"child", L"2"));
  shared_ptr<BitSetProducer> parentFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"isparent", L"yes")));
  CheckJoinIndex::check(s->getIndexReader(), parentFilter);

  shared_ptr<ToParentBlockJoinQuery> q =
      make_shared<ToParentBlockJoinQuery>(tq, parentFilter, ScoreMode::Avg);
  shared_ptr<Weight> weight = s->createWeight(s->rewrite(q), true, 1);
  shared_ptr<Scorer> sc = weight->scorer(s->getIndexReader()->leaves()[0]);
  TestUtil::assertEquals(2, sc->begin().advance(0));
  delete r;
  delete dir;
}

void TestBlockJoin::testChildQueryNeverMatches() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  shared_ptr<Document> parent = make_shared<Document>();
  parent->push_back(make_shared<StoredField>(L"parentID", L"0"));
  parent->push_back(make_shared<SortedDocValuesField>(
      L"parentID", make_shared<BytesRef>(L"0")));
  parent->push_back(newTextField(L"parentText", L"text", Field::Store::NO));
  parent->push_back(newStringField(L"isParent", L"yes", Field::Store::NO));

  deque<std::shared_ptr<Document>> docs = deque<std::shared_ptr<Document>>();

  shared_ptr<Document> child = make_shared<Document>();
  docs.push_back(child);
  child->push_back(make_shared<StoredField>(L"childID", L"0"));
  child->push_back(newTextField(L"childText", L"text", Field::Store::NO));

  // parent last:
  docs.push_back(parent);
  w->addDocuments(docs);

  docs.clear();

  parent = make_shared<Document>();
  parent->push_back(newTextField(L"parentText", L"text", Field::Store::NO));
  parent->push_back(newStringField(L"isParent", L"yes", Field::Store::NO));
  parent->push_back(make_shared<StoredField>(L"parentID", L"1"));
  parent->push_back(make_shared<SortedDocValuesField>(
      L"parentID", make_shared<BytesRef>(L"1")));

  // parent last:
  docs.push_back(parent);
  w->addDocuments(docs);

  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> searcher = newSearcher(r);

  // never matches:
  shared_ptr<Query> childQuery =
      make_shared<TermQuery>(make_shared<Term>(L"childText", L"bogus"));
  shared_ptr<BitSetProducer> parentsFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"isParent", L"yes")));
  CheckJoinIndex::check(r, parentsFilter);
  shared_ptr<ToParentBlockJoinQuery> childJoinQuery =
      make_shared<ToParentBlockJoinQuery>(childQuery, parentsFilter,
                                          ScoreMode::Avg);

  shared_ptr<Weight> weight = searcher->createWeight(
      searcher->rewrite(childJoinQuery), random()->nextBoolean(), 1);
  shared_ptr<Scorer> scorer =
      weight->scorer(searcher->getIndexReader()->leaves()[0]);
  assertNull(scorer);

  // never matches and produces a null scorer
  childQuery = make_shared<TermQuery>(make_shared<Term>(L"bogus", L"bogus"));
  childJoinQuery = make_shared<ToParentBlockJoinQuery>(
      childQuery, parentsFilter, ScoreMode::Avg);

  weight = searcher->createWeight(searcher->rewrite(childJoinQuery),
                                  random()->nextBoolean(), 1);
  scorer = weight->scorer(searcher->getIndexReader()->leaves()[0]);
  assertNull(scorer);

  delete r;
  delete d;
}

void TestBlockJoin::testAdvanceSingleDeletedParentNoChild() 
{

  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  // First doc with 1 children
  shared_ptr<Document> parentDoc = make_shared<Document>();
  parentDoc->push_back(newStringField(L"parent", L"1", Field::Store::NO));
  parentDoc->push_back(newStringField(L"isparent", L"yes", Field::Store::NO));
  shared_ptr<Document> childDoc = make_shared<Document>();
  childDoc->push_back(newStringField(L"child", L"1", Field::Store::NO));
  w->addDocuments(Arrays::asList(childDoc, parentDoc));

  parentDoc = make_shared<Document>();
  parentDoc->push_back(newStringField(L"parent", L"2", Field::Store::NO));
  parentDoc->push_back(newStringField(L"isparent", L"yes", Field::Store::NO));
  w->addDocuments(Arrays::asList(parentDoc));

  w->deleteDocuments(make_shared<Term>(L"parent", L"2"));

  parentDoc = make_shared<Document>();
  parentDoc->push_back(newStringField(L"parent", L"2", Field::Store::NO));
  parentDoc->push_back(newStringField(L"isparent", L"yes", Field::Store::NO));
  childDoc = make_shared<Document>();
  childDoc->push_back(newStringField(L"child", L"2", Field::Store::NO));
  w->addDocuments(Arrays::asList(childDoc, parentDoc));

  shared_ptr<IndexReader> r = w->getReader();
  delete w;
  shared_ptr<IndexSearcher> s = newSearcher(r);

  // Create a filter that defines "parent" documents in the index - in this case
  // resumes
  shared_ptr<BitSetProducer> parentsFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"isparent", L"yes")));
  CheckJoinIndex::check(r, parentsFilter);

  shared_ptr<Query> parentQuery =
      make_shared<TermQuery>(make_shared<Term>(L"parent", L"2"));

  shared_ptr<ToChildBlockJoinQuery> parentJoinQuery =
      make_shared<ToChildBlockJoinQuery>(parentQuery, parentsFilter);
  shared_ptr<TopDocs> topdocs = s->search(parentJoinQuery, 3);
  TestUtil::assertEquals(1, topdocs->totalHits);

  delete r;
  delete dir;
}

void TestBlockJoin::testIntersectionWithRandomApproximation() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  constexpr int numBlocks = atLeast(100);
  for (int i = 0; i < numBlocks; ++i) {
    deque<std::shared_ptr<Document>> docs =
        deque<std::shared_ptr<Document>>();
    constexpr int numChildren = random()->nextInt(3);
    for (int j = 0; j < numChildren; ++j) {
      shared_ptr<Document> child = make_shared<Document>();
      child->push_back(make_shared<StringField>(
          L"foo_child", random()->nextBoolean() ? L"bar" : L"baz",
          Field::Store::NO));
      docs.push_back(child);
    }
    shared_ptr<Document> parent = make_shared<Document>();
    parent->push_back(
        make_shared<StringField>(L"parent", L"true", Field::Store::NO));
    parent->push_back(make_shared<StringField>(
        L"foo_parent", random()->nextBoolean() ? L"bar" : L"baz",
        Field::Store::NO));
    docs.push_back(parent);
    w->addDocuments(docs);
  }
  shared_ptr<IndexReader> *const reader = w->getReader();
  shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
  searcher->setQueryCache(nullptr); // to have real advance() calls

  shared_ptr<BitSetProducer> *const parentsFilter =
      make_shared<QueryBitSetProducer>(
          make_shared<TermQuery>(make_shared<Term>(L"parent", L"true")));
  shared_ptr<Query> *const toChild = make_shared<ToChildBlockJoinQuery>(
      make_shared<TermQuery>(make_shared<Term>(L"foo_parent", L"bar")),
      parentsFilter);
  shared_ptr<Query> *const childQuery =
      make_shared<TermQuery>(make_shared<Term>(L"foo_child", L"baz"));

  shared_ptr<BooleanQuery> bq1 = (make_shared<BooleanQuery::Builder>())
                                     ->add(toChild, Occur::MUST)
                                     ->add(childQuery, Occur::MUST)
                                     ->build();
  shared_ptr<BooleanQuery> bq2 =
      (make_shared<BooleanQuery::Builder>())
          ->add(toChild, Occur::MUST)
          ->add(make_shared<RandomApproximationQuery>(childQuery, random()),
                Occur::MUST)
          ->build();

  TestUtil::assertEquals(searcher->count(bq1), searcher->count(bq2));

  delete searcher->getIndexReader();
  delete w;
  delete dir;
}

void TestBlockJoin::testParentScoringBug() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  const deque<std::shared_ptr<Document>> docs =
      deque<std::shared_ptr<Document>>();
  docs.push_back(makeJob(L"java", 2007));
  docs.push_back(makeJob(L"python", 2010));
  docs.push_back(makeResume(L"Lisa", L"United Kingdom"));
  w->addDocuments(docs);

  docs.clear();
  docs.push_back(makeJob(L"java", 2006));
  docs.push_back(makeJob(L"ruby", 2005));
  docs.push_back(makeResume(L"Frank", L"United States"));
  w->addDocuments(docs);
  w->deleteDocuments(make_shared<Term>(
      L"skill", L"java")); // delete the first child of every parent

  shared_ptr<IndexReader> r = w->getReader();
  delete w;
  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  // Create a filter that defines "parent" documents in the index - in this case
  // resumes
  shared_ptr<BitSetProducer> parentsFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"docType", L"resume")));
  shared_ptr<Query> parentQuery =
      make_shared<PrefixQuery>(make_shared<Term>(L"country", L"United"));

  shared_ptr<ToChildBlockJoinQuery> toChildQuery =
      make_shared<ToChildBlockJoinQuery>(parentQuery, parentsFilter);

  shared_ptr<TopDocs> hits = s->search(toChildQuery, 10);
  TestUtil::assertEquals(hits->scoreDocs.size(), 2);
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    if (hits->scoreDocs[i]->score == 0.0) {
      fail(L"Failed to calculate score for hit #" + to_wstring(i));
    }
  }

  delete r;
  delete dir;
}

void TestBlockJoin::testToChildBlockJoinQueryExplain() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  const deque<std::shared_ptr<Document>> docs =
      deque<std::shared_ptr<Document>>();
  docs.push_back(makeJob(L"java", 2007));
  docs.push_back(makeJob(L"python", 2010));
  docs.push_back(makeResume(L"Lisa", L"United Kingdom"));
  w->addDocuments(docs);

  docs.clear();
  docs.push_back(makeJob(L"java", 2006));
  docs.push_back(makeJob(L"ruby", 2005));
  docs.push_back(makeResume(L"Frank", L"United States"));
  w->addDocuments(docs);
  w->deleteDocuments(make_shared<Term>(
      L"skill", L"java")); // delete the first child of every parent

  shared_ptr<IndexReader> r = w->getReader();
  delete w;
  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  // Create a filter that defines "parent" documents in the index - in this case
  // resumes
  shared_ptr<BitSetProducer> parentsFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"docType", L"resume")));
  shared_ptr<Query> parentQuery =
      make_shared<PrefixQuery>(make_shared<Term>(L"country", L"United"));

  shared_ptr<ToChildBlockJoinQuery> toChildQuery =
      make_shared<ToChildBlockJoinQuery>(parentQuery, parentsFilter);

  shared_ptr<TopDocs> hits = s->search(toChildQuery, 10);
  TestUtil::assertEquals(hits->scoreDocs.size(), 2);
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    assertEquals(hits->scoreDocs[i]->score,
                 s->explain(toChildQuery, hits->scoreDocs[i]->doc)->getValue(),
                 0.01);
  }

  delete r;
  delete dir;
}

void TestBlockJoin::testToChildInitialAdvanceParentButNoKids() throw(
    runtime_error)
{

  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  // degenerate case: first doc has no children
  w->addDocument(makeResume(L"first", L"nokids"));
  w->addDocuments(
      Arrays::asList(makeJob(L"job", 42), makeResume(L"second", L"haskid")));

  // single segment
  w->forceMerge(1);

  shared_ptr<IndexReader> *const r = w->getReader();
  shared_ptr<IndexSearcher> *const s = newSearcher(r, false);
  delete w;

  shared_ptr<BitSetProducer> parentFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"docType", L"resume")));
  shared_ptr<Query> parentQuery =
      make_shared<TermQuery>(make_shared<Term>(L"docType", L"resume"));

  shared_ptr<ToChildBlockJoinQuery> parentJoinQuery =
      make_shared<ToChildBlockJoinQuery>(parentQuery, parentFilter);

  shared_ptr<Weight> weight =
      s->createWeight(s->rewrite(parentJoinQuery), random()->nextBoolean(), 1);
  shared_ptr<Scorer> advancingScorer =
      weight->scorer(s->getIndexReader()->leaves()[0]);
  shared_ptr<Scorer> nextDocScorer =
      weight->scorer(s->getIndexReader()->leaves()[0]);

  constexpr int firstKid = nextDocScorer->begin().nextDoc();
  assertTrue(L"firstKid not found", DocIdSetIterator::NO_MORE_DOCS != firstKid);
  TestUtil::assertEquals(firstKid, advancingScorer->begin().advance(0));

  delete r;
  delete dir;
}

void TestBlockJoin::testMultiChildQueriesOfDiffParentLevels() throw(
    runtime_error)
{

  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  // randomly generate resume->jobs[]->qualifications[]
  constexpr int numResumes = atLeast(100);
  for (int r = 0; r < numResumes; r++) {
    const deque<std::shared_ptr<Document>> docs =
        deque<std::shared_ptr<Document>>();

    constexpr int rv = TestUtil::nextInt(random(), 1, 10);
    constexpr int numJobs = atLeast(10);
    for (int j = 0; j < numJobs; j++) {
      constexpr int jv = TestUtil::nextInt(
          random(), -10, -1); // neg so no overlap with q (both used for "year")

      constexpr int numQualifications = atLeast(10);
      for (int q = 0; q < numQualifications; q++) {
        docs.push_back(makeQualification(L"q" + to_wstring(q) + L"_rv" +
                                             to_wstring(rv) + L"_jv" +
                                             to_wstring(jv),
                                         q));
      }
      docs.push_back(makeJob(L"j" + to_wstring(j), jv));
    }
    docs.push_back(makeResume(L"r" + to_wstring(r), L"rv" + to_wstring(rv)));
    w->addDocuments(docs);
  }

  shared_ptr<IndexReader> *const r = w->getReader();
  shared_ptr<IndexSearcher> *const s = newSearcher(r, false);
  delete w;

  shared_ptr<BitSetProducer> resumeFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"docType", L"resume")));
  // anything with a skill is a job
  shared_ptr<BitSetProducer> jobFilter = make_shared<QueryBitSetProducer>(
      make_shared<PrefixQuery>(make_shared<Term>(L"skill", L"")));

  constexpr int numQueryIters = atLeast(1);
  for (int i = 0; i < numQueryIters; i++) {
    constexpr int qjv = TestUtil::nextInt(random(), -10, -1);
    constexpr int qrv = TestUtil::nextInt(random(), 1, 10);

    shared_ptr<Query> resumeQuery = make_shared<ToChildBlockJoinQuery>(
        make_shared<TermQuery>(
            make_shared<Term>(L"country", L"rv" + to_wstring(qrv))),
        resumeFilter);

    shared_ptr<Query> jobQuery = make_shared<ToChildBlockJoinQuery>(
        IntPoint::newRangeQuery(L"year", qjv, qjv), jobFilter);

    shared_ptr<BooleanQuery::Builder> fullQuery =
        make_shared<BooleanQuery::Builder>();
    fullQuery->add(make_shared<BooleanClause>(jobQuery, Occur::MUST));
    fullQuery->add(make_shared<BooleanClause>(resumeQuery, Occur::MUST));

    shared_ptr<TopDocs> hits =
        s->search(fullQuery->build(),
                  100); // NOTE: totally possible that we'll get no matches

    for (auto sd : hits->scoreDocs) {
      // since we're looking for children of jobs, all results must be
      // qualifications
      wstring q = r->document(sd->doc)[L"qualification"];
      assertNotNull(to_wstring(sd->doc) + L" has no qualification", q);
      assertTrue(q + L" MUST contain jv" + to_wstring(qjv),
                 q.find(L"jv" + to_wstring(qjv)) != wstring::npos);
      assertTrue(q + L" MUST contain rv" + to_wstring(qrv),
                 q.find(L"rv" + to_wstring(qrv)) != wstring::npos);
    }
  }

  delete r;
  delete dir;
}

void TestBlockJoin::testScoreMode() 
{
  shared_ptr<Similarity> sim =
      make_shared<SimilarityBaseAnonymousInnerClass>(shared_from_this());
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir, newIndexWriterConfig()->setSimilarity(sim));
  w->addDocuments(Arrays::asList(
      Collections::singleton(
          newTextField(L"foo", L"bar bar", Field::Store::NO)),
      Collections::singleton(newTextField(L"foo", L"bar", Field::Store::NO)),
      Collections::emptyList(),
      Collections::singleton(newStringField(
          L"type", make_shared<BytesRef>(L"parent"), Field::Store::NO))));
  shared_ptr<DirectoryReader> reader = w->getReader();
  delete w;
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  searcher->setSimilarity(sim);
  shared_ptr<BitSetProducer> parents = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"type", L"parent")));
  for (ScoreMode scoreMode : ScoreMode::values()) {
    shared_ptr<Query> query = make_shared<ToParentBlockJoinQuery>(
        make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")), parents,
        scoreMode);
    shared_ptr<TopDocs> topDocs = searcher->search(query, 10);
    TestUtil::assertEquals(1, topDocs->totalHits);
    TestUtil::assertEquals(3, topDocs->scoreDocs[0]->doc);
    float expectedScore;
    switch (scoreMode) {
    case org::apache::lucene::search::join::ScoreMode::Avg:
      expectedScore = 1.5f;
      break;
    case org::apache::lucene::search::join::ScoreMode::Max:
      expectedScore = 2.0f;
      break;
    case org::apache::lucene::search::join::ScoreMode::Min:
      expectedScore = 1.0f;
      break;
    case org::apache::lucene::search::join::ScoreMode::None:
      expectedScore = 0.0f;
      break;
    case org::apache::lucene::search::join::ScoreMode::Total:
      expectedScore = 3.0f;
      break;
    default:
      throw make_shared<AssertionError>();
    }
    assertEquals(expectedScore, topDocs->scoreDocs[0]->score, 0.0f);
  }
  reader->close();
  delete dir;
}

TestBlockJoin::SimilarityBaseAnonymousInnerClass::
    SimilarityBaseAnonymousInnerClass(shared_ptr<TestBlockJoin> outerInstance)
{
  this->outerInstance = outerInstance;
}

wstring TestBlockJoin::SimilarityBaseAnonymousInnerClass::toString()
{
  return L"TestSim";
}

float TestBlockJoin::SimilarityBaseAnonymousInnerClass::score(
    shared_ptr<BasicStats> stats, float freq, float docLen)
{
  return freq;
}
} // namespace org::apache::lucene::search::join