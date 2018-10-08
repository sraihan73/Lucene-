using namespace std;

#include "TestRegexpRandom2.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using FilteredTermsEnum = org::apache::lucene::index::FilteredTermsEnum;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;
using AutomatonTestUtil =
    org::apache::lucene::util::automaton::AutomatonTestUtil;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;

void TestRegexpRandom2::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  fieldName = random()->nextBoolean()
                  ? L"field"
                  : L""; // sometimes use an empty string as field name
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(
          make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false))
          ->setMaxBufferedDocs(TestUtil::nextInt(random(), 50, 1000)));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field = newStringField(fieldName, L"", Field::Store::NO);
  doc->push_back(field);
  shared_ptr<Field> dvField =
      make_shared<SortedDocValuesField>(fieldName, make_shared<BytesRef>());
  doc->push_back(dvField);
  deque<wstring> terms = deque<wstring>();
  int num = atLeast(200);
  for (int i = 0; i < num; i++) {
    wstring s = TestUtil::randomUnicodeString(random());
    field->setStringValue(s);
    dvField->setBytesValue(make_shared<BytesRef>(s));
    terms.push_back(s);
    writer->addDocument(doc);
  }

  if (VERBOSE) {
    // utf16 order
    sort(terms.begin(), terms.end());
    wcout << L"UTF16 order:" << endl;
    for (auto s : terms) {
      wcout << L"  " << UnicodeUtil::toHexString(s) << endl;
    }
  }

  reader = writer->getReader();
  searcher1 = newSearcher(reader);
  searcher2 = newSearcher(reader);
  delete writer;
}

void TestRegexpRandom2::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

TestRegexpRandom2::DumbRegexpQuery::DumbRegexpQuery(shared_ptr<Term> term,
                                                    int flags)
    : MultiTermQuery(term->field()), automaton(re::toAutomaton())
{
  shared_ptr<RegExp> re = make_shared<RegExp>(term->text(), flags);
}

shared_ptr<TermsEnum> TestRegexpRandom2::DumbRegexpQuery::getTermsEnum(
    shared_ptr<Terms> terms,
    shared_ptr<AttributeSource> atts) 
{
  return make_shared<SimpleAutomatonTermsEnum>(shared_from_this(),
                                               terms->begin());
}

TestRegexpRandom2::DumbRegexpQuery::SimpleAutomatonTermsEnum::
    SimpleAutomatonTermsEnum(
        shared_ptr<TestRegexpRandom2::DumbRegexpQuery> outerInstance,
        shared_ptr<TermsEnum> tenum)
    : org::apache::lucene::index::FilteredTermsEnum(tenum),
      outerInstance(outerInstance)
{
  setInitialSeekTerm(make_shared<BytesRef>(L""));
}

AcceptStatus
TestRegexpRandom2::DumbRegexpQuery::SimpleAutomatonTermsEnum::accept(
    shared_ptr<BytesRef> term) 
{
  utf16->copyUTF8Bytes(term->bytes, term->offset, term->length);
  return runAutomaton->run(utf16->chars(), 0, utf16->length())
             ? AcceptStatus::YES
             : AcceptStatus::NO;
}

wstring TestRegexpRandom2::DumbRegexpQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return field.toString() + automaton->toString();
}

bool TestRegexpRandom2::DumbRegexpQuery::equals(any obj)
{
  if (MultiTermQuery::equals(obj) == false) {
    return false;
  }
  shared_ptr<DumbRegexpQuery> *const that =
      any_cast<std::shared_ptr<DumbRegexpQuery>>(obj);
  return automaton->equals(that->automaton);
}

void TestRegexpRandom2::testRegexps() 
{
  int num = atLeast(1000);
  for (int i = 0; i < num; i++) {
    wstring reg = AutomatonTestUtil::randomRegexp(random());
    if (VERBOSE) {
      wcout << L"TEST: regexp='" << reg << L"'" << endl;
    }
    assertSame(reg);
  }
}

void TestRegexpRandom2::assertSame(const wstring &regexp) 
{
  shared_ptr<RegexpQuery> smart = make_shared<RegexpQuery>(
      make_shared<Term>(fieldName, regexp), RegExp::NONE);
  shared_ptr<DumbRegexpQuery> dumb = make_shared<DumbRegexpQuery>(
      make_shared<Term>(fieldName, regexp), RegExp::NONE);

  shared_ptr<TopDocs> smartDocs = searcher1->search(smart, 25);
  shared_ptr<TopDocs> dumbDocs = searcher2->search(dumb, 25);

  CheckHits::checkEqual(smart, smartDocs->scoreDocs, dumbDocs->scoreDocs);
}
} // namespace org::apache::lucene::search