using namespace std;

#include "TestMultiTermQueryRewrites.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FilteredTermsEnum = org::apache::lucene::index::FilteredTermsEnum;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiReader = org::apache::lucene::index::MultiReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::store::Directory> dir, sdir1,
    TestMultiTermQueryRewrites::sdir2;
shared_ptr<org::apache::lucene::index::IndexReader> reader, multiReader,
    TestMultiTermQueryRewrites::multiReaderDupls;
shared_ptr<IndexSearcher> searcher, multiSearcher,
    TestMultiTermQueryRewrites::multiSearcherDupls;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestMultiTermQueryRewrites::beforeClass() 
{
  dir = newDirectory();
  sdir1 = newDirectory();
  sdir2 = newDirectory();
  shared_ptr<RandomIndexWriter> *const writer = make_shared<RandomIndexWriter>(
      random(), dir, make_shared<MockAnalyzer>(random()));
  shared_ptr<RandomIndexWriter> *const swriter1 =
      make_shared<RandomIndexWriter>(random(), sdir1,
                                     make_shared<MockAnalyzer>(random()));
  shared_ptr<RandomIndexWriter> *const swriter2 =
      make_shared<RandomIndexWriter>(random(), sdir2,
                                     make_shared<MockAnalyzer>(random()));

  for (int i = 0; i < 10; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(
        newStringField(L"data", Integer::toString(i), Field::Store::NO));
    writer->addDocument(doc);
    ((i % 2 == 0) ? swriter1 : swriter2)->addDocument(doc);
  }
  writer->forceMerge(1);
  swriter1->forceMerge(1);
  swriter2->forceMerge(1);
  delete writer;
  delete swriter1;
  delete swriter2;

  reader = DirectoryReader::open(dir);
  searcher = newSearcher(reader);

  multiReader = make_shared<MultiReader>(
      std::deque<std::shared_ptr<IndexReader>>{DirectoryReader::open(sdir1),
                                                DirectoryReader::open(sdir2)},
      true);
  multiSearcher = newSearcher(multiReader);

  multiReaderDupls = make_shared<MultiReader>(
      std::deque<std::shared_ptr<IndexReader>>{DirectoryReader::open(sdir1),
                                                DirectoryReader::open(dir)},
      true);
  multiSearcherDupls = newSearcher(multiReaderDupls);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestMultiTermQueryRewrites::afterClass() 
{
  delete reader;
  delete multiReader;
  delete multiReaderDupls;
  delete dir;
  delete sdir1;
  delete sdir2;
  reader = multiReader = multiReaderDupls = nullptr;
  searcher = multiSearcher = multiSearcherDupls = nullptr;
  dir = sdir1 = sdir2 = nullptr;
}

shared_ptr<Query>
TestMultiTermQueryRewrites::extractInnerQuery(shared_ptr<Query> q)
{
  if (std::dynamic_pointer_cast<ConstantScoreQuery>(q) != nullptr) {
    // wrapped as ConstantScoreQuery
    q = (std::static_pointer_cast<ConstantScoreQuery>(q))->getQuery();
  }
  return q;
}

shared_ptr<Term> TestMultiTermQueryRewrites::extractTerm(shared_ptr<Query> q)
{
  q = extractInnerQuery(q);
  return (std::static_pointer_cast<TermQuery>(q))->getTerm();
}

void TestMultiTermQueryRewrites::checkBooleanQueryOrder(shared_ptr<Query> q)
{
  q = extractInnerQuery(q);
  shared_ptr<BooleanQuery> *const bq =
      std::static_pointer_cast<BooleanQuery>(q);
  shared_ptr<Term> last = nullptr, act;
  for (auto clause : bq->clauses()) {
    act = extractTerm(clause->getQuery());
    if (last != nullptr) {
      assertTrue(L"sort order of terms in BQ violated",
                 last->compareTo(act) < 0);
    }
    last = act;
  }
}

void TestMultiTermQueryRewrites::checkDuplicateTerms(
    shared_ptr<MultiTermQuery::RewriteMethod> method) 
{
  shared_ptr<MultiTermQuery> *const mtq =
      TermRangeQuery::newStringRange(L"data", L"2", L"7", true, true);
  mtq->setRewriteMethod(method);
  shared_ptr<Query> *const q1 = searcher->rewrite(mtq);
  shared_ptr<Query> *const q2 = multiSearcher->rewrite(mtq);
  shared_ptr<Query> *const q3 = multiSearcherDupls->rewrite(mtq);
  if (VERBOSE) {
    wcout << endl;
    wcout << L"single segment: " << q1 << endl;
    wcout << L"multi segment: " << q2 << endl;
    wcout << L"multi segment with duplicates: " << q3 << endl;
  }
  assertEquals(L"The multi-segment case must produce same rewritten query", q1,
               q2);
  assertEquals(L"The multi-segment case with duplicates must produce same "
               L"rewritten query",
               q1, q3);
  checkBooleanQueryOrder(q1);
  checkBooleanQueryOrder(q2);
  checkBooleanQueryOrder(q3);
}

void TestMultiTermQueryRewrites::testRewritesWithDuplicateTerms() throw(
    runtime_error)
{
  checkDuplicateTerms(MultiTermQuery::SCORING_BOOLEAN_REWRITE);

  checkDuplicateTerms(MultiTermQuery::CONSTANT_SCORE_BOOLEAN_REWRITE);

  // use a large PQ here to only test duplicate terms and dont mix up when all
  // scores are equal
  checkDuplicateTerms(
      make_shared<MultiTermQuery::TopTermsScoringBooleanQueryRewrite>(1024));
  checkDuplicateTerms(
      make_shared<MultiTermQuery::TopTermsBoostOnlyBooleanQueryRewrite>(1024));
}

void TestMultiTermQueryRewrites::checkBooleanQueryBoosts(
    shared_ptr<BooleanQuery> bq)
{
  for (auto clause : bq->clauses()) {
    shared_ptr<BoostQuery> *const boostQ =
        std::static_pointer_cast<BoostQuery>(clause->getQuery());
    shared_ptr<TermQuery> *const mtq =
        std::static_pointer_cast<TermQuery>(boostQ->getQuery());
    assertEquals(L"Parallel sorting of boosts in rewrite mode broken",
                 stof(mtq->getTerm()->text()), boostQ->getBoost(), 0);
  }
}

void TestMultiTermQueryRewrites::checkBoosts(
    shared_ptr<MultiTermQuery::RewriteMethod> method) 
{
  shared_ptr<MultiTermQuery> *const mtq =
      make_shared<MultiTermQueryAnonymousInnerClass>(shared_from_this());
  mtq->setRewriteMethod(method);
  shared_ptr<Query> *const q1 = searcher->rewrite(mtq);
  shared_ptr<Query> *const q2 = multiSearcher->rewrite(mtq);
  shared_ptr<Query> *const q3 = multiSearcherDupls->rewrite(mtq);
  if (VERBOSE) {
    wcout << endl;
    wcout << L"single segment: " << q1 << endl;
    wcout << L"multi segment: " << q2 << endl;
    wcout << L"multi segment with duplicates: " << q3 << endl;
  }
  assertEquals(L"The multi-segment case must produce same rewritten query", q1,
               q2);
  assertEquals(L"The multi-segment case with duplicates must produce same "
               L"rewritten query",
               q1, q3);
  if (std::dynamic_pointer_cast<MatchNoDocsQuery>(q1) != nullptr) {
    assertTrue(std::dynamic_pointer_cast<MatchNoDocsQuery>(q1) != nullptr);
    assertTrue(std::dynamic_pointer_cast<MatchNoDocsQuery>(q3) != nullptr);
  } else {
    checkBooleanQueryBoosts(std::static_pointer_cast<BooleanQuery>(q1));
    checkBooleanQueryBoosts(std::static_pointer_cast<BooleanQuery>(q2));
    checkBooleanQueryBoosts(std::static_pointer_cast<BooleanQuery>(q3));
    assert(false);
  }
}

TestMultiTermQueryRewrites::MultiTermQueryAnonymousInnerClass::
    MultiTermQueryAnonymousInnerClass(
        shared_ptr<TestMultiTermQueryRewrites> outerInstance)
    : MultiTermQuery(L"data")
{
  this->outerInstance = outerInstance;
}

shared_ptr<TermsEnum>
TestMultiTermQueryRewrites::MultiTermQueryAnonymousInnerClass::getTermsEnum(
    shared_ptr<Terms> terms,
    shared_ptr<AttributeSource> atts) 
{
  return make_shared<FilteredTermsEnumAnonymousInnerClass>(shared_from_this(),
                                                           terms->begin());
}

TestMultiTermQueryRewrites::MultiTermQueryAnonymousInnerClass::
    FilteredTermsEnumAnonymousInnerClass::FilteredTermsEnumAnonymousInnerClass(
        shared_ptr<MultiTermQueryAnonymousInnerClass> outerInstance,
        shared_ptr<TermsEnum> iterator)
    : org::apache::lucene::index::FilteredTermsEnum(iterator)
{
  this->outerInstance = outerInstance;
  boostAtt = attributes().addAttribute(BoostAttribute::typeid);
}

AcceptStatus TestMultiTermQueryRewrites::MultiTermQueryAnonymousInnerClass::
    FilteredTermsEnumAnonymousInnerClass::accept(shared_ptr<BytesRef> term)
{
  boostAtt::setBoost(stof(term->utf8ToString()));
  if (term->length == 0) {
    return AcceptStatus::NO;
  }
  wchar_t c = static_cast<wchar_t>(term->bytes[term->offset] & 0xff);
  if (c >= L'2') {
    if (c <= L'7') {
      return AcceptStatus::YES;
    } else {
      return AcceptStatus::END;
    }
  } else {
    return AcceptStatus::NO;
  }
}

// C++ TODO: There is no native C++ equivalent to 'toString':
wstring
TestMultiTermQueryRewrites::MultiTermQueryAnonymousInnerClass::Term::toString(
    const wstring &field)
{
  return L"dummy";
}

void TestMultiTermQueryRewrites::testBoosts() 
{
  checkBoosts(MultiTermQuery::SCORING_BOOLEAN_REWRITE);

  // use a large PQ here to only test boosts and dont mix up when all scores are
  // equal
  checkBoosts(
      make_shared<MultiTermQuery::TopTermsScoringBooleanQueryRewrite>(1024));
}

void TestMultiTermQueryRewrites::checkMaxClauseLimitation(
    shared_ptr<MultiTermQuery::RewriteMethod> method) 
{
  int savedMaxClauseCount = BooleanQuery::getMaxClauseCount();
  BooleanQuery::setMaxClauseCount(3);

  shared_ptr<MultiTermQuery> *const mtq =
      TermRangeQuery::newStringRange(L"data", L"2", L"7", true, true);
  mtq->setRewriteMethod(method);
  try {
    shared_ptr<BooleanQuery::TooManyClauses> expected =
        expectThrows(BooleanQuery::TooManyClauses::typeid,
                     [&]() { multiSearcherDupls->rewrite(mtq); });
    //  Maybe remove this assert in later versions, when internal API changes:
    assertEquals(L"Should throw BooleanQuery.TooManyClauses with a stacktrace "
                 L"containing checkMaxClauseCount()",
                 L"checkMaxClauseCount",
                 expected->getStackTrace()[0].getMethodName());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    BooleanQuery::setMaxClauseCount(savedMaxClauseCount);
  }
}

void TestMultiTermQueryRewrites::checkNoMaxClauseLimitation(
    shared_ptr<MultiTermQuery::RewriteMethod> method) 
{
  int savedMaxClauseCount = BooleanQuery::getMaxClauseCount();
  BooleanQuery::setMaxClauseCount(3);

  shared_ptr<MultiTermQuery> *const mtq =
      TermRangeQuery::newStringRange(L"data", L"2", L"7", true, true);
  mtq->setRewriteMethod(method);
  try {
    multiSearcherDupls->rewrite(mtq);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    BooleanQuery::setMaxClauseCount(savedMaxClauseCount);
  }
}

void TestMultiTermQueryRewrites::testMaxClauseLimitations() 
{
  checkMaxClauseLimitation(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  checkMaxClauseLimitation(MultiTermQuery::CONSTANT_SCORE_BOOLEAN_REWRITE);

  checkNoMaxClauseLimitation(MultiTermQuery::CONSTANT_SCORE_REWRITE);
  checkNoMaxClauseLimitation(
      make_shared<MultiTermQuery::TopTermsScoringBooleanQueryRewrite>(1024));
  checkNoMaxClauseLimitation(
      make_shared<MultiTermQuery::TopTermsBoostOnlyBooleanQueryRewrite>(1024));
}
} // namespace org::apache::lucene::search