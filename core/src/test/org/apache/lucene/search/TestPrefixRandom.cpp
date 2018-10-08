using namespace std;

#include "TestPrefixRandom.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FilteredTermsEnum = org::apache::lucene::index::FilteredTermsEnum;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestPrefixRandom::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(
          make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false))
          ->setMaxBufferedDocs(TestUtil::nextInt(random(), 50, 1000)));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field = newStringField(L"field", L"", Field::Store::NO);
  doc->push_back(field);

  int num = atLeast(1000);
  for (int i = 0; i < num; i++) {
    field->setStringValue(TestUtil::randomUnicodeString(random(), 10));
    writer->addDocument(doc);
  }
  reader = writer->getReader();
  searcher = newSearcher(reader);
  delete writer;
}

void TestPrefixRandom::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

TestPrefixRandom::DumbPrefixQuery::DumbPrefixQuery(shared_ptr<Term> term)
    : MultiTermQuery(term->field()), prefix(term->bytes())
{
}

shared_ptr<TermsEnum> TestPrefixRandom::DumbPrefixQuery::getTermsEnum(
    shared_ptr<Terms> terms,
    shared_ptr<AttributeSource> atts) 
{
  return make_shared<SimplePrefixTermsEnum>(terms->begin(), prefix);
}

TestPrefixRandom::DumbPrefixQuery::SimplePrefixTermsEnum::SimplePrefixTermsEnum(
    shared_ptr<TermsEnum> tenum, shared_ptr<BytesRef> prefix)
    : org::apache::lucene::index::FilteredTermsEnum(tenum), prefix(prefix)
{
  setInitialSeekTerm(make_shared<BytesRef>(L""));
}

AcceptStatus TestPrefixRandom::DumbPrefixQuery::SimplePrefixTermsEnum::accept(
    shared_ptr<BytesRef> term) 
{
  return StringHelper::startsWith(term, prefix) ? AcceptStatus::YES
                                                : AcceptStatus::NO;
}

wstring TestPrefixRandom::DumbPrefixQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return field.toString() + L":" + prefix->toString();
}

bool TestPrefixRandom::DumbPrefixQuery::equals(any obj)
{
  if (MultiTermQuery::equals(obj) == false) {
    return false;
  }
  shared_ptr<DumbPrefixQuery> *const that =
      any_cast<std::shared_ptr<DumbPrefixQuery>>(obj);
  return prefix->equals(that->prefix);
}

void TestPrefixRandom::testPrefixes() 
{
  int num = atLeast(100);
  for (int i = 0; i < num; i++) {
    assertSame(TestUtil::randomUnicodeString(random(), 5));
  }
}

void TestPrefixRandom::assertSame(const wstring &prefix) 
{
  shared_ptr<PrefixQuery> smart =
      make_shared<PrefixQuery>(make_shared<Term>(L"field", prefix));
  shared_ptr<DumbPrefixQuery> dumb =
      make_shared<DumbPrefixQuery>(make_shared<Term>(L"field", prefix));

  shared_ptr<TopDocs> smartDocs = searcher->search(smart, 25);
  shared_ptr<TopDocs> dumbDocs = searcher->search(dumb, 25);
  CheckHits::checkEqual(smart, smartDocs->scoreDocs, dumbDocs->scoreDocs);
}
} // namespace org::apache::lucene::search