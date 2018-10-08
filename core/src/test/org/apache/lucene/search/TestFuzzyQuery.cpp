using namespace std;

#include "TestFuzzyQuery.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiReader = org::apache::lucene::index::MultiReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRef = org::apache::lucene::util::IntsRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using LevenshteinAutomata =
    org::apache::lucene::util::automaton::LevenshteinAutomata;

void TestFuzzyQuery::testBasicPrefix() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  addDoc(L"abc", writer);
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  delete writer;

  shared_ptr<FuzzyQuery> query = make_shared<FuzzyQuery>(
      make_shared<Term>(L"field", L"abc"), FuzzyQuery::defaultMaxEdits, 1);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(1, hits.size());
  delete reader;
  delete directory;
}

void TestFuzzyQuery::testFuzziness() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  addDoc(L"aaaaa", writer);
  addDoc(L"aaaab", writer);
  addDoc(L"aaabb", writer);
  addDoc(L"aabbb", writer);
  addDoc(L"abbbb", writer);
  addDoc(L"bbbbb", writer);
  addDoc(L"ddddd", writer);

  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  delete writer;

  shared_ptr<FuzzyQuery> query = make_shared<FuzzyQuery>(
      make_shared<Term>(L"field", L"aaaaa"), FuzzyQuery::defaultMaxEdits, 0);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(3, hits.size());

  // same with prefix
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"aaaaa"),
                                  FuzzyQuery::defaultMaxEdits, 1);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(3, hits.size());
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"aaaaa"),
                                  FuzzyQuery::defaultMaxEdits, 2);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(3, hits.size());
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"aaaaa"),
                                  FuzzyQuery::defaultMaxEdits, 3);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(3, hits.size());
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"aaaaa"),
                                  FuzzyQuery::defaultMaxEdits, 4);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(2, hits.size());
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"aaaaa"),
                                  FuzzyQuery::defaultMaxEdits, 5);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(1, hits.size());
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"aaaaa"),
                                  FuzzyQuery::defaultMaxEdits, 6);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(1, hits.size());

  // test scoring
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"bbbbb"),
                                  FuzzyQuery::defaultMaxEdits, 0);
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"3 documents should match", 3, hits.size());
  deque<wstring> order = Arrays::asList(L"bbbbb", L"abbbb", L"aabbb");
  for (int i = 0; i < hits.size(); i++) {
    const wstring term = searcher->doc(hits[i]->doc)[L"field"];
    // System.out.println(hits[i].score);
    TestUtil::assertEquals(order[i], term);
  }

  // test pq size by supplying maxExpansions=2
  // This query would normally return 3 documents, because 3 terms match (see
  // above):
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"bbbbb"),
                                  FuzzyQuery::defaultMaxEdits, 0, 2, false);
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"only 2 documents should match", 2, hits.size());
  order = Arrays::asList(L"bbbbb", L"abbbb");
  for (int i = 0; i < hits.size(); i++) {
    const wstring term = searcher->doc(hits[i]->doc)[L"field"];
    // System.out.println(hits[i].score);
    TestUtil::assertEquals(order[i], term);
  }

  // not similar enough:
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"xxxxx"),
                                  FuzzyQuery::defaultMaxEdits, 0);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(0, hits.size());
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"aaccc"),
                                  FuzzyQuery::defaultMaxEdits,
                                  0); // edit distance to "aaaaa" = 3
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(0, hits.size());

  // query identical to a word in the index:
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"aaaaa"),
                                  FuzzyQuery::defaultMaxEdits, 0);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(3, hits.size());
  TestUtil::assertEquals(searcher->doc(hits[0]->doc)[L"field"], (L"aaaaa"));
  // default allows for up to two edits:
  TestUtil::assertEquals(searcher->doc(hits[1]->doc)[L"field"], (L"aaaab"));
  TestUtil::assertEquals(searcher->doc(hits[2]->doc)[L"field"], (L"aaabb"));

  // query similar to a word in the index:
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"aaaac"),
                                  FuzzyQuery::defaultMaxEdits, 0);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(3, hits.size());
  TestUtil::assertEquals(searcher->doc(hits[0]->doc)[L"field"], (L"aaaaa"));
  TestUtil::assertEquals(searcher->doc(hits[1]->doc)[L"field"], (L"aaaab"));
  TestUtil::assertEquals(searcher->doc(hits[2]->doc)[L"field"], (L"aaabb"));

  // now with prefix
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"aaaac"),
                                  FuzzyQuery::defaultMaxEdits, 1);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(3, hits.size());
  TestUtil::assertEquals(searcher->doc(hits[0]->doc)[L"field"], (L"aaaaa"));
  TestUtil::assertEquals(searcher->doc(hits[1]->doc)[L"field"], (L"aaaab"));
  TestUtil::assertEquals(searcher->doc(hits[2]->doc)[L"field"], (L"aaabb"));
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"aaaac"),
                                  FuzzyQuery::defaultMaxEdits, 2);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(3, hits.size());
  TestUtil::assertEquals(searcher->doc(hits[0]->doc)[L"field"], (L"aaaaa"));
  TestUtil::assertEquals(searcher->doc(hits[1]->doc)[L"field"], (L"aaaab"));
  TestUtil::assertEquals(searcher->doc(hits[2]->doc)[L"field"], (L"aaabb"));
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"aaaac"),
                                  FuzzyQuery::defaultMaxEdits, 3);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(3, hits.size());
  TestUtil::assertEquals(searcher->doc(hits[0]->doc)[L"field"], (L"aaaaa"));
  TestUtil::assertEquals(searcher->doc(hits[1]->doc)[L"field"], (L"aaaab"));
  TestUtil::assertEquals(searcher->doc(hits[2]->doc)[L"field"], (L"aaabb"));
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"aaaac"),
                                  FuzzyQuery::defaultMaxEdits, 4);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(2, hits.size());
  TestUtil::assertEquals(searcher->doc(hits[0]->doc)[L"field"], (L"aaaaa"));
  TestUtil::assertEquals(searcher->doc(hits[1]->doc)[L"field"], (L"aaaab"));
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"aaaac"),
                                  FuzzyQuery::defaultMaxEdits, 5);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(0, hits.size());

  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"ddddX"),
                                  FuzzyQuery::defaultMaxEdits, 0);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(1, hits.size());
  TestUtil::assertEquals(searcher->doc(hits[0]->doc)[L"field"], (L"ddddd"));

  // now with prefix
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"ddddX"),
                                  FuzzyQuery::defaultMaxEdits, 1);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(1, hits.size());
  TestUtil::assertEquals(searcher->doc(hits[0]->doc)[L"field"], (L"ddddd"));
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"ddddX"),
                                  FuzzyQuery::defaultMaxEdits, 2);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(1, hits.size());
  TestUtil::assertEquals(searcher->doc(hits[0]->doc)[L"field"], (L"ddddd"));
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"ddddX"),
                                  FuzzyQuery::defaultMaxEdits, 3);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(1, hits.size());
  TestUtil::assertEquals(searcher->doc(hits[0]->doc)[L"field"], (L"ddddd"));
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"ddddX"),
                                  FuzzyQuery::defaultMaxEdits, 4);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(1, hits.size());
  TestUtil::assertEquals(searcher->doc(hits[0]->doc)[L"field"], (L"ddddd"));
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"ddddX"),
                                  FuzzyQuery::defaultMaxEdits, 5);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(0, hits.size());

  // different field = no match:
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"anotherfield", L"ddddX"),
                                  FuzzyQuery::defaultMaxEdits, 0);
  hits = searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(0, hits.size());

  delete reader;
  delete directory;
}

void TestFuzzyQuery::test2() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false));
  addDoc(L"LANGE", writer);
  addDoc(L"LUETH", writer);
  addDoc(L"PIRSING", writer);
  addDoc(L"RIEGEL", writer);
  addDoc(L"TRZECZIAK", writer);
  addDoc(L"WALKER", writer);
  addDoc(L"WBR", writer);
  addDoc(L"WE", writer);
  addDoc(L"WEB", writer);
  addDoc(L"WEBE", writer);
  addDoc(L"WEBER", writer);
  addDoc(L"WEBERE", writer);
  addDoc(L"WEBREE", writer);
  addDoc(L"WEBEREI", writer);
  addDoc(L"WBRE", writer);
  addDoc(L"WITTKOPF", writer);
  addDoc(L"WOJNAROWSKI", writer);
  addDoc(L"WRICKE", writer);

  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  delete writer;

  shared_ptr<FuzzyQuery> query =
      make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"WEBER"), 2, 1);
  // query.setRewriteMethod(FuzzyQuery.SCORING_BOOLEAN_QUERY_REWRITE);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(8, hits.size());

  delete reader;
  delete directory;
}

void TestFuzzyQuery::testSingleQueryExactMatchScoresHighest() throw(
    runtime_error)
{
  // See issue LUCENE-329 - IDF shouldn't wreck similarity ranking
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  addDoc(L"smith", writer);
  addDoc(L"smith", writer);
  addDoc(L"smith", writer);
  addDoc(L"smith", writer);
  addDoc(L"smith", writer);
  addDoc(L"smith", writer);
  addDoc(L"smythe", writer);
  addDoc(L"smdssasd", writer);

  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  searcher->setSimilarity(
      make_shared<ClassicSimilarity>()); // avoid randomisation of similarity
                                         // algo by test framework
  delete writer;
  std::deque<wstring> searchTerms = {L"smith", L"smythe", L"smdssasd"};
  for (auto searchTerm : searchTerms) {
    shared_ptr<FuzzyQuery> query =
        make_shared<FuzzyQuery>(make_shared<Term>(L"field", searchTerm), 2, 1);
    std::deque<std::shared_ptr<ScoreDoc>> hits =
        searcher->search(query, 1000)->scoreDocs;
    shared_ptr<Document> bestDoc = searcher->doc(hits[0]->doc);
    assertTrue(hits.size() > 0);
    wstring topMatch = bestDoc[L"field"];
    TestUtil::assertEquals(searchTerm, topMatch);
    if (hits.size() > 1) {
      shared_ptr<Document> worstDoc = searcher->doc(hits[hits.size() - 1]->doc);
      wstring worstMatch = worstDoc[L"field"];
      assertNotSame(searchTerm, worstMatch);
    }
  }
  delete reader;
  delete directory;
}

void TestFuzzyQuery::testMultipleQueriesIdfWorks() 
{
  // With issue LUCENE-329 - it could be argued a
  // MultiTermQuery.TopTermsBoostOnlyBooleanQueryRewrite is the solution as it
  // disables IDF. However - IDF is still useful as in this case where there are
  // multiple FuzzyQueries.
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);

  addDoc(L"michael smith", writer);
  addDoc(L"michael lucero", writer);
  addDoc(L"doug cutting", writer);
  addDoc(L"doug cuttin", writer);
  addDoc(L"michael wardle", writer);
  addDoc(L"micheal vegas", writer);
  addDoc(L"michael lydon", writer);

  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  searcher->setSimilarity(
      make_shared<ClassicSimilarity>()); // avoid randomisation of similarity
                                         // algo by test framework

  delete writer;

  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  wstring commonSearchTerm = L"michael";
  shared_ptr<FuzzyQuery> commonQuery = make_shared<FuzzyQuery>(
      make_shared<Term>(L"field", commonSearchTerm), 2, 1);
  query->add(commonQuery, Occur::SHOULD);

  wstring rareSearchTerm = L"cutting";
  shared_ptr<FuzzyQuery> rareQuery = make_shared<FuzzyQuery>(
      make_shared<Term>(L"field", rareSearchTerm), 2, 1);
  query->add(rareQuery, Occur::SHOULD);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query->build(), 1000)->scoreDocs;

  // Matches on the rare surname should be worth more than matches on the common
  // forename
  TestUtil::assertEquals(7, hits.size());
  shared_ptr<Document> bestDoc = searcher->doc(hits[0]->doc);
  wstring topMatch = bestDoc[L"field"];
  assertTrue(topMatch.find(rareSearchTerm) != wstring::npos);

  shared_ptr<Document> runnerUpDoc = searcher->doc(hits[1]->doc);
  wstring runnerUpMatch = runnerUpDoc[L"field"];
  assertTrue(runnerUpMatch.find(L"cuttin") != wstring::npos);

  shared_ptr<Document> worstDoc = searcher->doc(hits[hits.size() - 1]->doc);
  wstring worstMatch = worstDoc[L"field"];
  assertTrue(worstMatch.find(L"micheal") !=
             wstring::npos); // misspelling of common name

  delete reader;
  delete directory;
}

void TestFuzzyQuery::testTieBreaker() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  addDoc(L"a123456", writer);
  addDoc(L"c123456", writer);
  addDoc(L"d123456", writer);
  addDoc(L"e123456", writer);

  shared_ptr<Directory> directory2 = newDirectory();
  shared_ptr<RandomIndexWriter> writer2 =
      make_shared<RandomIndexWriter>(random(), directory2);
  addDoc(L"a123456", writer2);
  addDoc(L"b123456", writer2);
  addDoc(L"b123456", writer2);
  addDoc(L"b123456", writer2);
  addDoc(L"c123456", writer2);
  addDoc(L"f123456", writer2);

  shared_ptr<IndexReader> ir1 = writer->getReader();
  shared_ptr<IndexReader> ir2 = writer2->getReader();

  shared_ptr<MultiReader> mr = make_shared<MultiReader>(ir1, ir2);
  shared_ptr<IndexSearcher> searcher = newSearcher(mr);
  shared_ptr<FuzzyQuery> fq = make_shared<FuzzyQuery>(
      make_shared<Term>(L"field", L"z123456"), 1, 0, 2, false);
  shared_ptr<TopDocs> docs = searcher->search(fq, 2);
  TestUtil::assertEquals(5, docs->totalHits); // 5 docs, from the a and b's
  mr->close();
  delete ir1;
  delete ir2;
  delete writer;
  delete writer2;
  delete directory;
  delete directory2;
}

void TestFuzzyQuery::testBoostOnlyRewrite() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  addDoc(L"Lucene", writer);
  addDoc(L"Lucene", writer);
  addDoc(L"Lucenne", writer);

  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  delete writer;

  shared_ptr<FuzzyQuery> query =
      make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"lucene"));
  query->setRewriteMethod(
      make_shared<MultiTermQuery::TopTermsBoostOnlyBooleanQueryRewrite>(50));
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(3, hits.size());
  // normally, 'Lucenne' would be the first result as IDF will skew the score.
  TestUtil::assertEquals(L"Lucene", reader->document(hits[0]->doc)[L"field"]);
  TestUtil::assertEquals(L"Lucene", reader->document(hits[1]->doc)[L"field"]);
  TestUtil::assertEquals(L"Lucenne", reader->document(hits[2]->doc)[L"field"]);
  delete reader;
  delete directory;
}

void TestFuzzyQuery::testGiga() 
{

  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<Directory> index = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), index);

  addDoc(L"Lucene in Action", w);
  addDoc(L"Lucene for Dummies", w);

  // addDoc("Giga", w);
  addDoc(L"Giga byte", w);

  addDoc(L"ManagingGigabytesManagingGigabyte", w);
  addDoc(L"ManagingGigabytesManagingGigabytes", w);

  addDoc(L"The Art of Computer Science", w);
  addDoc(L"J. K. Rowling", w);
  addDoc(L"JK Rowling", w);
  addDoc(L"Joanne K Roling", w);
  addDoc(L"Bruce Willis", w);
  addDoc(L"Willis bruce", w);
  addDoc(L"Brute willis", w);
  addDoc(L"B. willis", w);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<Query> q =
      make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"giga"), 0);

  // 3. search
  shared_ptr<IndexSearcher> searcher = newSearcher(r);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(q, 10)->scoreDocs;
  TestUtil::assertEquals(1, hits.size());
  TestUtil::assertEquals(L"Giga byte", searcher->doc(hits[0]->doc)[L"field"]);
  delete r;
  delete index;
}

void TestFuzzyQuery::testDistanceAsEditsSearching() 
{
  shared_ptr<Directory> index = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), index);
  addDoc(L"foobar", w);
  addDoc(L"test", w);
  addDoc(L"working", w);
  shared_ptr<IndexReader> reader = w->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  delete w;

  shared_ptr<FuzzyQuery> q =
      make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"fouba"), 2);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(q, 10)->scoreDocs;
  TestUtil::assertEquals(1, hits.size());
  TestUtil::assertEquals(L"foobar", searcher->doc(hits[0]->doc)[L"field"]);

  q = make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"foubara"), 2);
  hits = searcher->search(q, 10)->scoreDocs;
  TestUtil::assertEquals(1, hits.size());
  TestUtil::assertEquals(L"foobar", searcher->doc(hits[0]->doc)[L"field"]);

  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"t"), 3);
  });

  delete reader;
  delete index;
}

void TestFuzzyQuery::testValidation()
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"foo"), -1, 0, 1,
                            false);
  });
  assertTrue(expected.what()->contains(L"maxEdits"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"foo"),
                            LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE + 1,
                            0, 1, false);
  });
  assertTrue(expected.what()->contains(L"maxEdits must be between"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"foo"), 1, -1, 1,
                            false);
  });
  assertTrue(expected.what()->contains(L"prefixLength cannot be negative"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"foo"), 1, 0, -1,
                            false);
  });
  assertTrue(expected.what()->contains(L"maxExpansions must be positive"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"foo"), 1, 0, -1,
                            false);
  });
  assertTrue(expected.what()->contains(L"maxExpansions must be positive"));
}

void TestFuzzyQuery::addDoc(
    const wstring &text,
    shared_ptr<RandomIndexWriter> writer) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", text, Field::Store::YES));
  writer->addDocument(doc);
}

wstring TestFuzzyQuery::randomSimpleString(int digits)
{
  int termLength = TestUtil::nextInt(random(), 1, 8);
  std::deque<wchar_t> chars(termLength);
  for (int i = 0; i < termLength; i++) {
    chars[i] = static_cast<wchar_t>(L'a' + random()->nextInt(digits));
  }
  return wstring(chars);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) public void
// testRandom() throws Exception
void TestFuzzyQuery::testRandom() 
{
  int digits = TestUtil::nextInt(random(), 2, 3);
  // underestimated total number of unique terms that randomSimpleString
  // maybe generate, it assumes all terms have a length of 7
  int vocabularySize = digits << 7;
  int numTerms = min(atLeast(100), vocabularySize);
  shared_ptr<Set<wstring>> terms = unordered_set<wstring>();
  while (terms->size() < numTerms) {
    terms->add(randomSimpleString(digits));
  }

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  for (auto term : terms) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"field", term, Field::Store::YES));
    w->addDocument(doc);
  }
  shared_ptr<DirectoryReader> r = w->getReader();
  // System.out.println("TEST: reader=" + r);
  shared_ptr<IndexSearcher> s = newSearcher(r);
  int iters = atLeast(1000);
  for (int iter = 0; iter < iters; iter++) {
    wstring queryTerm = randomSimpleString(digits);
    int prefixLength = random()->nextInt(queryTerm.length());
    wstring queryPrefix = queryTerm.substr(0, prefixLength);

    // we don't look at scores here:
    std::deque<deque<std::shared_ptr<TermAndScore>>> expected =
        std::deque<deque>(3);
    for (int ed = 0; ed < 3; ed++) {
      expected[ed] = deque<std::shared_ptr<TermAndScore>>();
    }
    for (auto term : terms) {
      if (term.startsWith(queryPrefix) == false) {
        continue;
      }
      int ed = getDistance(term, queryTerm);
      float score =
          1.0f - static_cast<float>(ed) /
                     static_cast<float>(min(queryTerm.length(), term.length()));
      while (ed < 3) {
        expected[ed].push_back(make_shared<TermAndScore>(term, score));
        ed++;
      }
    }

    for (int ed = 0; ed < 3; ed++) {
      sort(expected[ed].begin(), expected[ed].end());
      int queueSize = TestUtil::nextInt(random(), 1, terms->size());
      /*
      System.out.println("\nTEST: query=" + queryTerm + " ed=" + ed + "
      queueSize=" + queueSize + " vs expected match size=" + expected[ed].size()
      + " prefixLength=" + prefixLength); for(TermAndScore ent : expected[ed]) {
        System.out.println("  " + ent);
      }
      */
      shared_ptr<FuzzyQuery> query =
          make_shared<FuzzyQuery>(make_shared<Term>(L"field", queryTerm), ed,
                                  prefixLength, queueSize, true);
      shared_ptr<TopDocs> hits = s->search(query, terms->size());
      shared_ptr<Set<wstring>> actual = unordered_set<wstring>();
      for (auto hit : hits->scoreDocs) {
        shared_ptr<Document> doc = s->doc(hit->doc);
        actual->add(doc[L"field"]);
        // System.out.println("   actual: " + doc.get("field") + " score=" +
        // hit.score);
      }
      shared_ptr<Set<wstring>> expectedTop = unordered_set<wstring>();
      int limit = min(queueSize, expected[ed].size());
      for (int i = 0; i < limit; i++) {
        expectedTop->add(expected[ed][i]->term);
      }

      if (actual->equals(expectedTop) == false) {
        shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
        sb->append(L"FAILED: query=" + queryTerm + L" ed=" + to_wstring(ed) +
                   L" queueSize=" + to_wstring(queueSize) +
                   L" vs expected match size=" + expected[ed].size() +
                   L" prefixLength=" + to_wstring(prefixLength) + L"\n");

        bool first = true;
        for (auto term : actual) {
          if (expectedTop->contains(term) == false) {
            if (first) {
              sb->append(L"  these matched but shouldn't:\n");
              first = false;
            }
            sb->append(L"    " + term + L"\n");
          }
        }
        first = true;
        for (auto term : expectedTop) {
          if (actual->contains(term) == false) {
            if (first) {
              sb->append(L"  these did not match but should:\n");
              first = false;
            }
            sb->append(L"    " + term + L"\n");
          }
        }
        throw make_shared<AssertionError>(sb->toString());
      }
    }
  }

  IOUtils::close({r, w, dir});
}

TestFuzzyQuery::TermAndScore::TermAndScore(const wstring &term, float score)
    : term(term), score(score)
{
}

int TestFuzzyQuery::TermAndScore::compareTo(shared_ptr<TermAndScore> other)
{
  // higher score sorts first, and if scores are tied, lower term sorts first
  if (score > other->score) {
    return -1;
  } else if (score < other->score) {
    return 1;
  } else {
    return term.compare(other->term);
  }
}

wstring TestFuzzyQuery::TermAndScore::toString()
{
  return term + L" score=" + to_wstring(score);
}

int TestFuzzyQuery::getDistance(const wstring &target, const wstring &other)
{
  shared_ptr<IntsRef> targetPoints;
  shared_ptr<IntsRef> otherPoints;
  int n;
  std::deque<std::deque<int>> d; // cost array

  // NOTE: if we cared, we could 3*m space instead of m*n space, similar to
  // what LevenshteinDistance does, except cycling thru a ring of three
  // horizontal cost arrays... but this comparator is never actually used by
  // DirectSpellChecker, it's only used for merging results from multiple shards
  // in "distributed spellcheck", and it's inefficient in other ways too...

  // cheaper to do this up front once
  targetPoints = toIntsRef(target);
  otherPoints = toIntsRef(other);
  n = targetPoints->length;
  constexpr int m = otherPoints->length;
  // C++ NOTE: The following call to the 'RectangularVectors' helper class
  // reproduces the rectangular array initialization that is automatic in Java:
  // ORIGINAL LINE: d = new int[n+1][m+1];
  d = RectangularVectors::ReturnRectangularIntVector(n + 1, m + 1);

  if (n == 0 || m == 0) {
    if (n == m) {
      return 0;
    } else {
      return max(n, m);
    }
  }

  // indexes into strings s and t
  int i; // iterates through s
  int j; // iterates through t

  int t_j; // jth character of t

  int cost; // cost

  for (i = 0; i <= n; i++) {
    d[i][0] = i;
  }

  for (j = 0; j <= m; j++) {
    d[0][j] = j;
  }

  for (j = 1; j <= m; j++) {
    t_j = otherPoints->ints[j - 1];

    for (i = 1; i <= n; i++) {
      cost = targetPoints->ints[i - 1] == t_j ? 0 : 1;
      // minimum of cell to the left+1, to the top+1, diagonally left and up
      // +cost
      d[i][j] =
          min(min(d[i - 1][j] + 1, d[i][j - 1] + 1), d[i - 1][j - 1] + cost);
      // transposition
      if (i > 1 && j > 1 &&
          targetPoints->ints[i - 1] == otherPoints->ints[j - 2] &&
          targetPoints->ints[i - 2] == otherPoints->ints[j - 1]) {
        d[i][j] = min(d[i][j], d[i - 2][j - 2] + cost);
      }
    }
  }

  return d[n][m];
}

shared_ptr<IntsRef> TestFuzzyQuery::toIntsRef(const wstring &s)
{
  shared_ptr<IntsRef> ref = make_shared<IntsRef>(s.length()); // worst case
  int utf16Len = s.length();
  for (int i = 0, cp = 0; i < utf16Len; i += Character::charCount(cp)) {
    cp = ref->ints[ref->length++] = Character::codePointAt(s, i);
  }
  return ref;
}
} // namespace org::apache::lucene::search