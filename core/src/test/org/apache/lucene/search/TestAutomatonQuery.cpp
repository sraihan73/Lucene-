using namespace std;

#include "TestAutomatonQuery.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiFields = org::apache::lucene::index::MultiFields;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SingleTermsEnum = org::apache::lucene::index::SingleTermsEnum;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Rethrow = org::apache::lucene::util::Rethrow;
using TestUtil = org::apache::lucene::util::TestUtil;
using Automata = org::apache::lucene::util::automaton::Automata;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using AutomatonTestUtil =
    org::apache::lucene::util::automaton::AutomatonTestUtil;
using Operations = org::apache::lucene::util::automaton::Operations;
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;
const wstring TestAutomatonQuery::FN = L"field";

void TestAutomatonQuery::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> titleField =
      newTextField(L"title", L"some title", Field::Store::NO);
  shared_ptr<Field> field =
      newTextField(FN, L"this is document one 2345", Field::Store::NO);
  shared_ptr<Field> footerField =
      newTextField(L"footer", L"a footer", Field::Store::NO);
  doc->push_back(titleField);
  doc->push_back(field);
  doc->push_back(footerField);
  writer->addDocument(doc);
  field->setStringValue(L"some text from doc two a short piece 5678.91");
  writer->addDocument(doc);
  field->setStringValue(wstring(L"doc three has some different stuff") +
                        L" with numbers 1234 5678.9 and letter b");
  writer->addDocument(doc);
  reader = writer->getReader();
  searcher = newSearcher(reader);
  delete writer;
}

void TestAutomatonQuery::tearDown() 
{
  delete reader;
  delete directory;
  LuceneTestCase::tearDown();
}

shared_ptr<Term> TestAutomatonQuery::newTerm(const wstring &value)
{
  return make_shared<Term>(FN, value);
}

int64_t TestAutomatonQuery::automatonQueryNrHits(
    shared_ptr<AutomatonQuery> query) 
{
  if (VERBOSE) {
    wcout << L"TEST: run aq=" << query << endl;
  }
  return searcher->search(query, 5)->totalHits;
}

void TestAutomatonQuery::assertAutomatonHits(
    int expected, shared_ptr<Automaton> automaton) 
{
  shared_ptr<AutomatonQuery> query =
      make_shared<AutomatonQuery>(newTerm(L"bogus"), automaton);

  query->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  TestUtil::assertEquals(expected, automatonQueryNrHits(query));

  query->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_REWRITE);
  TestUtil::assertEquals(expected, automatonQueryNrHits(query));

  query->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_BOOLEAN_REWRITE);
  TestUtil::assertEquals(expected, automatonQueryNrHits(query));
}

void TestAutomatonQuery::testAutomata() 
{
  assertAutomatonHits(0, Automata::makeEmpty());
  assertAutomatonHits(0, Automata::makeEmptyString());
  assertAutomatonHits(2, Automata::makeAnyChar());
  assertAutomatonHits(3, Automata::makeAnyString());
  assertAutomatonHits(2, Automata::makeString(L"doc"));
  assertAutomatonHits(1, Automata::makeChar(L'a'));
  assertAutomatonHits(2, Automata::makeCharRange(L'a', L'b'));
  assertAutomatonHits(2, Automata::makeDecimalInterval(1233, 2346, 0));
  assertAutomatonHits(1, Automata::makeDecimalInterval(0, 2000, 0));
  assertAutomatonHits(2, Operations::union_(Automata::makeChar(L'a'),
                                            Automata::makeChar(L'b')));
  assertAutomatonHits(0, Operations::intersection(Automata::makeChar(L'a'),
                                                  Automata::makeChar(L'b')));
  assertAutomatonHits(
      1, Operations::minus(Automata::makeCharRange(L'a', L'b'),
                           Automata::makeChar(L'a'),
                           AutomatonTestUtil::DEFAULT_MAX_DETERMINIZED_STATES));
}

void TestAutomatonQuery::testNFA() 
{
  // accept this or three, the union is an NFA (two transitions for 't' from
  // initial state)
  shared_ptr<Automaton> nfa = Operations::union_(
      Automata::makeString(L"this"), Automata::makeString(L"three"));
  assertAutomatonHits(2, nfa);
}

void TestAutomatonQuery::testEquals()
{
  shared_ptr<AutomatonQuery> a1 = make_shared<AutomatonQuery>(
      newTerm(L"foobar"), Automata::makeString(L"foobar"));
  // reference to a1
  shared_ptr<AutomatonQuery> a2 = a1;
  // same as a1 (accepts the same language, same term)
  shared_ptr<AutomatonQuery> a3 = make_shared<AutomatonQuery>(
      newTerm(L"foobar"),
      Operations::concatenate(Automata::makeString(L"foo"),
                              Automata::makeString(L"bar")));
  // different than a1 (same term, but different language)
  shared_ptr<AutomatonQuery> a4 = make_shared<AutomatonQuery>(
      newTerm(L"foobar"), Automata::makeString(L"different"));
  // different than a1 (different term, same language)
  shared_ptr<AutomatonQuery> a5 = make_shared<AutomatonQuery>(
      newTerm(L"blah"), Automata::makeString(L"foobar"));

  TestUtil::assertEquals(a1->hashCode(), a2->hashCode());
  TestUtil::assertEquals(a1, a2);

  TestUtil::assertEquals(a1->hashCode(), a3->hashCode());
  TestUtil::assertEquals(a1, a3);

  // different class
  shared_ptr<AutomatonQuery> w1 =
      make_shared<WildcardQuery>(newTerm(L"foobar"));
  // different class
  shared_ptr<AutomatonQuery> w2 = make_shared<RegexpQuery>(newTerm(L"foobar"));

  assertFalse(a1->equals(w1));
  assertFalse(a1->equals(w2));
  assertFalse(w1->equals(w2));
  assertFalse(a1->equals(a4));
  assertFalse(a1->equals(a5));
  assertFalse(a1->equals(nullptr));
}

void TestAutomatonQuery::testRewriteSingleTerm() 
{
  shared_ptr<AutomatonQuery> aq = make_shared<AutomatonQuery>(
      newTerm(L"bogus"), Automata::makeString(L"piece"));
  shared_ptr<Terms> terms =
      MultiFields::getTerms(searcher->getIndexReader(), FN);
  assertTrue(std::dynamic_pointer_cast<SingleTermsEnum>(
                 aq->getTermsEnum(terms)) != nullptr);
  TestUtil::assertEquals(1, automatonQueryNrHits(aq));
}

void TestAutomatonQuery::testRewritePrefix() 
{
  shared_ptr<Automaton> pfx = Automata::makeString(L"do");
  shared_ptr<Automaton> prefixAutomaton =
      Operations::concatenate(pfx, Automata::makeAnyString());
  shared_ptr<AutomatonQuery> aq =
      make_shared<AutomatonQuery>(newTerm(L"bogus"), prefixAutomaton);
  TestUtil::assertEquals(3, automatonQueryNrHits(aq));
}

void TestAutomatonQuery::testEmptyOptimization() 
{
  shared_ptr<AutomatonQuery> aq =
      make_shared<AutomatonQuery>(newTerm(L"bogus"), Automata::makeEmpty());
  // not yet available: assertTrue(aq.getEnum(searcher.getIndexReader())
  // instanceof EmptyTermEnum);
  shared_ptr<Terms> terms =
      MultiFields::getTerms(searcher->getIndexReader(), FN);
  assertSame(TermsEnum::EMPTY, aq->getTermsEnum(terms));
  TestUtil::assertEquals(0, automatonQueryNrHits(aq));
}

void TestAutomatonQuery::testHashCodeWithThreads() 
{
  std::deque<std::shared_ptr<AutomatonQuery>> queries(1000);
  for (int i = 0; i < queries.size(); i++) {
    queries[i] = make_shared<AutomatonQuery>(
        make_shared<Term>(L"bogus", L"bogus"),
        AutomatonTestUtil::randomAutomaton(random()),
        numeric_limits<int>::max());
  }
  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  int numThreads = TestUtil::nextInt(random(), 2, 5);
  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  for (int threadID = 0; threadID < numThreads; threadID++) {
    shared_ptr<Thread> thread = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), queries, startingGun);
    threads[threadID] = thread;
    thread->start();
  }
  startingGun->countDown();
  for (auto thread : threads) {
    thread->join();
  }
}

TestAutomatonQuery::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestAutomatonQuery> outerInstance,
    deque<std::shared_ptr<org::apache::lucene::search::AutomatonQuery>>
        &queries,
    shared_ptr<CountDownLatch> startingGun)
{
  this->outerInstance = outerInstance;
  this->queries = queries;
  this->startingGun = startingGun;
}

void TestAutomatonQuery::ThreadAnonymousInnerClass::run()
{
  try {
    startingGun->await();
    for (int i = 0; i < queries.size(); i++) {
      queries[i]->hashCode();
    }
  } catch (const runtime_error &e) {
    Rethrow::rethrow(e);
  }
}

void TestAutomatonQuery::testBiggishAutomaton()
{
  deque<std::shared_ptr<BytesRef>> terms = deque<std::shared_ptr<BytesRef>>();
  while (terms.size() < 3000) {
    terms.push_back(
        make_shared<BytesRef>(TestUtil::randomUnicodeString(random())));
  }
  sort(terms.begin(), terms.end());
  make_shared<AutomatonQuery>(make_shared<Term>(L"foo", L"bar"),
                              Automata::makeStringUnion(terms),
                              numeric_limits<int>::max());
}
} // namespace org::apache::lucene::search