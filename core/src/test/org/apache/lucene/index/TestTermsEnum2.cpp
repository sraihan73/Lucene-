using namespace std;

#include "TestTermsEnum2.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SeekStatus = org::apache::lucene::index::TermsEnum::SeekStatus;
using AutomatonQuery = org::apache::lucene::search::AutomatonQuery;
using CheckHits = org::apache::lucene::search::CheckHits;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using namespace org::apache::lucene::util::automaton;
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

void TestTermsEnum2::setUp() 
{
  LuceneTestCase::setUp();
  numIterations = atLeast(50);
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(
          make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false))
          ->setMaxBufferedDocs(TestUtil::nextInt(random(), 50, 1000)));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field = newStringField(L"field", L"", Field::Store::YES);
  doc->push_back(field);
  terms = set<>();

  int num = atLeast(200);
  for (int i = 0; i < num; i++) {
    wstring s = TestUtil::randomUnicodeString(random());
    field->setStringValue(s);
    terms->add(make_shared<BytesRef>(s));
    writer->addDocument(doc);
  }

  termsAutomaton = Automata::makeStringUnion(terms);

  reader = writer->getReader();
  searcher = newSearcher(reader);
  delete writer;
}

void TestTermsEnum2::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestTermsEnum2::testFiniteVersusInfinite() 
{

  for (int i = 0; i < numIterations; i++) {
    wstring reg = AutomatonTestUtil::randomRegexp(random());
    shared_ptr<Automaton> automaton = Operations::determinize(
        (make_shared<RegExp>(reg, RegExp::NONE))->toAutomaton(),
        DEFAULT_MAX_DETERMINIZED_STATES);
    const deque<std::shared_ptr<BytesRef>> matchedTerms =
        deque<std::shared_ptr<BytesRef>>();
    for (auto t : terms) {
      if (Operations::run(automaton, t->utf8ToString())) {
        matchedTerms.push_back(t);
      }
    }

    shared_ptr<Automaton> alternate = Automata::makeStringUnion(matchedTerms);
    // System.out.println("match " + matchedTerms.size() + " " +
    // alternate.getNumberOfStates() + " states, sigma=" +
    // alternate.getStartPoints().length);
    // AutomatonTestUtil.minimizeSimple(alternate);
    // System.out.println("minimize done");
    shared_ptr<AutomatonQuery> a1 = make_shared<AutomatonQuery>(
        make_shared<Term>(L"field", L""), automaton);
    shared_ptr<AutomatonQuery> a2 =
        make_shared<AutomatonQuery>(make_shared<Term>(L"field", L""), alternate,
                                    numeric_limits<int>::max());

    std::deque<std::shared_ptr<ScoreDoc>> origHits =
        searcher->search(a1, 25)->scoreDocs;
    std::deque<std::shared_ptr<ScoreDoc>> newHits =
        searcher->search(a2, 25)->scoreDocs;
    CheckHits::checkEqual(a1, origHits, newHits);
  }
}

void TestTermsEnum2::testSeeking() 
{
  for (int i = 0; i < numIterations; i++) {
    wstring reg = AutomatonTestUtil::randomRegexp(random());
    shared_ptr<Automaton> automaton = Operations::determinize(
        (make_shared<RegExp>(reg, RegExp::NONE))->toAutomaton(),
        DEFAULT_MAX_DETERMINIZED_STATES);
    shared_ptr<TermsEnum> te = MultiFields::getTerms(reader, L"field")->begin();
    deque<std::shared_ptr<BytesRef>> unsortedTerms =
        deque<std::shared_ptr<BytesRef>>(terms);
    Collections::shuffle(unsortedTerms, random());

    for (auto term : unsortedTerms) {
      if (Operations::run(automaton, term->utf8ToString())) {
        // term is accepted
        if (random()->nextBoolean()) {
          // seek exact
          assertTrue(te->seekExact(term));
        } else {
          // seek ceil
          TestUtil::assertEquals(SeekStatus::FOUND, te->seekCeil(term));
          TestUtil::assertEquals(term, te->term());
        }
      }
    }
  }
}

void TestTermsEnum2::testSeekingAndNexting() 
{
  for (int i = 0; i < numIterations; i++) {
    shared_ptr<TermsEnum> te = MultiFields::getTerms(reader, L"field")->begin();

    for (auto term : terms) {
      int c = random()->nextInt(3);
      if (c == 0) {
        TestUtil::assertEquals(term, te->next());
      } else if (c == 1) {
        TestUtil::assertEquals(SeekStatus::FOUND, te->seekCeil(term));
        TestUtil::assertEquals(term, te->term());
      } else {
        assertTrue(te->seekExact(term));
      }
    }
  }
}

void TestTermsEnum2::testIntersect() 
{
  for (int i = 0; i < numIterations; i++) {
    wstring reg = AutomatonTestUtil::randomRegexp(random());
    shared_ptr<Automaton> automaton =
        (make_shared<RegExp>(reg, RegExp::NONE))->toAutomaton();
    shared_ptr<CompiledAutomaton> ca = make_shared<CompiledAutomaton>(
        automaton, Operations::isFinite(automaton), false);
    shared_ptr<TermsEnum> te =
        MultiFields::getTerms(reader, L"field")->intersect(ca, nullptr);
    shared_ptr<Automaton> expected = Operations::determinize(
        Operations::intersection(termsAutomaton, automaton),
        DEFAULT_MAX_DETERMINIZED_STATES);
    set<std::shared_ptr<BytesRef>> found = set<std::shared_ptr<BytesRef>>();
    while (te->next() != nullptr) {
      found.insert(BytesRef::deepCopyOf(te->term()));
    }

    shared_ptr<Automaton> actual = Operations::determinize(
        Automata::makeStringUnion(found), DEFAULT_MAX_DETERMINIZED_STATES);
    assertTrue(Operations::sameLanguage(expected, actual));
  }
}
} // namespace org::apache::lucene::index