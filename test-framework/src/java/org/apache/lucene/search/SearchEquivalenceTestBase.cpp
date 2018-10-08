using namespace std;

#include "SearchEquivalenceTestBase.h"

namespace org::apache::lucene::search
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using TestUtil = org::apache::lucene::util::TestUtil;
using Automata = org::apache::lucene::util::automaton::Automata;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<IndexSearcher> s1, SearchEquivalenceTestBase::s2;
shared_ptr<org::apache::lucene::store::Directory>
    SearchEquivalenceTestBase::directory;
shared_ptr<org::apache::lucene::index::IndexReader>
    SearchEquivalenceTestBase::reader;
shared_ptr<org::apache::lucene::analysis::Analyzer>
    SearchEquivalenceTestBase::analyzer;
wstring SearchEquivalenceTestBase::stopword;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void SearchEquivalenceTestBase::beforeClass() 
{
  shared_ptr<Random> random = SearchEquivalenceTestBase::random();
  directory = newDirectory();
  stopword = L"" + StringHelper::toString(randomChar());
  shared_ptr<CharacterRunAutomaton> stopset =
      make_shared<CharacterRunAutomaton>(Automata::makeString(stopword));
  analyzer = make_shared<MockAnalyzer>(random, MockTokenizer::WHITESPACE, false,
                                       stopset);
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random, directory, analyzer);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> id = make_shared<StringField>(L"id", L"", Field::Store::NO);
  shared_ptr<Field> field =
      make_shared<TextField>(L"field", L"", Field::Store::NO);
  doc->push_back(id);
  doc->push_back(field);

  // index some docs
  int numDocs = TEST_NIGHTLY ? atLeast(1000) : atLeast(100);
  for (int i = 0; i < numDocs; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    id->setStringValue(Integer::toString(i));
    field->setStringValue(randomFieldContents());
    iw->addDocument(doc);
  }

  // delete some docs
  int numDeletes = numDocs / 20;
  for (int i = 0; i < numDeletes; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<Term> toDelete =
        make_shared<Term>(L"id", Integer::toString(random->nextInt(numDocs)));
    if (random->nextBoolean()) {
      iw->deleteDocuments(toDelete);
    } else {
      iw->deleteDocuments(make_shared<TermQuery>(toDelete));
    }
  }

  reader = iw->getReader();
  s1 = newSearcher(reader);
  s2 = newSearcher(reader);
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void SearchEquivalenceTestBase::afterClass() 
{
  delete reader;
  delete directory;
  delete analyzer;
  reader.reset();
  directory.reset();
  analyzer.reset();
  s1 = s2 = nullptr;
}

wstring SearchEquivalenceTestBase::randomFieldContents()
{
  // TODO: zipf-like distribution
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  int numTerms = random()->nextInt(15);
  for (int i = 0; i < numTerms; i++) {
    if (sb->length() > 0) {
      sb->append(L' '); // whitespace
    }
    sb->append(randomChar());
  }
  return sb->toString();
}

wchar_t SearchEquivalenceTestBase::randomChar()
{
  return static_cast<wchar_t>(TestUtil::nextInt(random(), L'a', L'z'));
}

shared_ptr<Term> SearchEquivalenceTestBase::randomTerm()
{
  return make_shared<Term>(L"field",
                           L"" + StringHelper::toString(randomChar()));
}

shared_ptr<Query> SearchEquivalenceTestBase::randomFilter()
{
  shared_ptr<Query> *const query;
  if (random()->nextBoolean()) {
    query = TermRangeQuery::newStringRange(
        L"field", L"a", L"" + StringHelper::toString(randomChar()), true, true);
  } else {
    // use a query with a two-phase approximation
    shared_ptr<PhraseQuery> phrase = make_shared<PhraseQuery>(
        100, L"field", L"" + StringHelper::toString(randomChar()),
        L"" + StringHelper::toString(randomChar()));
    query = phrase;
  }
  return query;
}

void SearchEquivalenceTestBase::assertSameSet(
    shared_ptr<Query> q1, shared_ptr<Query> q2) 
{
  assertSubsetOf(q1, q2);
  assertSubsetOf(q2, q1);
}

void SearchEquivalenceTestBase::assertSubsetOf(
    shared_ptr<Query> q1, shared_ptr<Query> q2) 
{
  // test without a filter
  assertSubsetOf(q1, q2, nullptr);

  // test with some filters (this will sometimes cause advance'ing enough to
  // test it)
  int numFilters = TEST_NIGHTLY ? atLeast(10) : atLeast(3);
  for (int i = 0; i < numFilters; i++) {
    shared_ptr<Query> filter = randomFilter();
    // incorporate the filter in different ways.
    assertSubsetOf(q1, q2, filter);
    assertSubsetOf(filteredQuery(q1, filter), filteredQuery(q2, filter),
                   nullptr);
  }
}

void SearchEquivalenceTestBase::assertSubsetOf(
    shared_ptr<Query> q1, shared_ptr<Query> q2,
    shared_ptr<Query> filter) 
{
  QueryUtils::check(q1);
  QueryUtils::check(q2);

  if (filter != nullptr) {
    q1 = (make_shared<BooleanQuery::Builder>())
             ->add(q1, Occur::MUST)
             ->add(filter, Occur::FILTER)
             ->build();
    q2 = (make_shared<BooleanQuery::Builder>())
             ->add(q2, Occur::MUST)
             ->add(filter, Occur::FILTER)
             ->build();
  }
  // we test both INDEXORDER and RELEVANCE because we want to test
  // needsScores=true/false
  for (auto sort :
       std::deque<std::shared_ptr<Sort>>{Sort::INDEXORDER, Sort::RELEVANCE}) {
    // not efficient, but simple!
    shared_ptr<TopDocs> td1 = s1->search(q1, reader->maxDoc(), sort);
    shared_ptr<TopDocs> td2 = s2->search(q2, reader->maxDoc(), sort);
    assertTrue(L"too many hits: " + to_wstring(td1->totalHits) + L" > " +
                   to_wstring(td2->totalHits),
               td1->totalHits <= td2->totalHits);

    // fill the superset into a bitset
    shared_ptr<BitSet> bitset = make_shared<BitSet>();
    for (int i = 0; i < td2->scoreDocs.size(); i++) {
      bitset->set(td2->scoreDocs[i]->doc);
    }

    // check in the subset, that every bit was set by the super
    for (int i = 0; i < td1->scoreDocs.size(); i++) {
      assertTrue(bitset->get(td1->scoreDocs[i]->doc));
    }
  }
}

void SearchEquivalenceTestBase::assertSameScores(
    shared_ptr<Query> q1, shared_ptr<Query> q2) 
{
  assertSameSet(q1, q2);

  assertSameScores(q1, q2, nullptr);
  // also test with some filters to test advancing
  int numFilters = TEST_NIGHTLY ? atLeast(10) : atLeast(3);
  for (int i = 0; i < numFilters; i++) {
    shared_ptr<Query> filter = randomFilter();
    // incorporate the filter in different ways.
    assertSameScores(q1, q2, filter);
    assertSameScores(filteredQuery(q1, filter), filteredQuery(q2, filter),
                     nullptr);
  }
}

void SearchEquivalenceTestBase::assertSameScores(
    shared_ptr<Query> q1, shared_ptr<Query> q2,
    shared_ptr<Query> filter) 
{
  // not efficient, but simple!
  if (filter != nullptr) {
    q1 = (make_shared<BooleanQuery::Builder>())
             ->add(q1, Occur::MUST)
             ->add(filter, Occur::FILTER)
             ->build();
    q2 = (make_shared<BooleanQuery::Builder>())
             ->add(q2, Occur::MUST)
             ->add(filter, Occur::FILTER)
             ->build();
  }
  shared_ptr<TopDocs> td1 = s1->search(q1, reader->maxDoc());
  shared_ptr<TopDocs> td2 = s2->search(q2, reader->maxDoc());
  TestUtil::assertEquals(td1->totalHits, td2->totalHits);
  for (int i = 0; i < td1->scoreDocs.size(); ++i) {
    TestUtil::assertEquals(td1->scoreDocs[i]->doc, td2->scoreDocs[i]->doc);
    assertEquals(td1->scoreDocs[i]->score, td2->scoreDocs[i]->score, 10e-5);
  }
}

shared_ptr<Query>
SearchEquivalenceTestBase::filteredQuery(shared_ptr<Query> query,
                                         shared_ptr<Query> filter)
{
  return (make_shared<BooleanQuery::Builder>())
      ->add(query, Occur::MUST)
      ->add(filter, Occur::FILTER)
      ->build();
}
} // namespace org::apache::lucene::search