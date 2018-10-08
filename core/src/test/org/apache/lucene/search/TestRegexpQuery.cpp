using namespace std;

#include "TestRegexpQuery.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Automata = org::apache::lucene::util::automaton::Automata;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using AutomatonProvider =
    org::apache::lucene::util::automaton::AutomatonProvider;
using Operations = org::apache::lucene::util::automaton::Operations;
using RegExp = org::apache::lucene::util::automaton::RegExp;
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;
const wstring TestRegexpQuery::FN = L"field";

void TestRegexpQuery::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(
      FN, L"the quick brown fox jumps over the lazy ??? dog 493432 49344",
      Field::Store::NO));
  writer->addDocument(doc);
  reader = writer->getReader();
  delete writer;
  searcher = newSearcher(reader);
}

void TestRegexpQuery::tearDown() 
{
  delete reader;
  delete directory;
  LuceneTestCase::tearDown();
}

shared_ptr<Term> TestRegexpQuery::newTerm(const wstring &value)
{
  return make_shared<Term>(FN, value);
}

int64_t
TestRegexpQuery::regexQueryNrHits(const wstring &regex) 
{
  shared_ptr<RegexpQuery> query = make_shared<RegexpQuery>(newTerm(regex));
  return searcher->search(query, 5)->totalHits;
}

void TestRegexpQuery::testRegex1() 
{
  assertEquals(1, regexQueryNrHits(L"q.[aeiou]c.*"));
}

void TestRegexpQuery::testRegex2() 
{
  assertEquals(0, regexQueryNrHits(L".[aeiou]c.*"));
}

void TestRegexpQuery::testRegex3() 
{
  assertEquals(0, regexQueryNrHits(L"q.[aeiou]c"));
}

void TestRegexpQuery::testNumericRange() 
{
  assertEquals(1, regexQueryNrHits(L"<420000-600000>"));
  assertEquals(0, regexQueryNrHits(L"<493433-600000>"));
}

void TestRegexpQuery::testRegexComplement() 
{
  assertEquals(1, regexQueryNrHits(L"4934~[3]"));
  // not the empty lang, i.e. match all docs
  assertEquals(1, regexQueryNrHits(L"~#"));
}

void TestRegexpQuery::testCustomProvider() 
{
  shared_ptr<AutomatonProvider> myProvider =
      make_shared<AutomatonProviderAnonymousInnerClass>(shared_from_this());
  shared_ptr<RegexpQuery> query = make_shared<RegexpQuery>(
      newTerm(L"<quickBrown>"), RegExp::ALL, myProvider,
      Operations::DEFAULT_MAX_DETERMINIZED_STATES);
  assertEquals(1, searcher->search(query, 5)->totalHits);
}

TestRegexpQuery::AutomatonProviderAnonymousInnerClass::
    AutomatonProviderAnonymousInnerClass(
        shared_ptr<TestRegexpQuery> outerInstance)
{
  this->outerInstance = outerInstance;
  quickBrownAutomaton = Operations::union_(Arrays::asList(
      Automata::makeString(L"quick"), Automata::makeString(L"brown"),
      Automata::makeString(L"bob")));
}

shared_ptr<Automaton>
TestRegexpQuery::AutomatonProviderAnonymousInnerClass::getAutomaton(
    const wstring &name)
{
  if (name == L"quickBrown") {
    return quickBrownAutomaton;
  } else {
    return nullptr;
  }
}

void TestRegexpQuery::testBacktracking() 
{
  assertEquals(1, regexQueryNrHits(L"4934[314]"));
}
} // namespace org::apache::lucene::search