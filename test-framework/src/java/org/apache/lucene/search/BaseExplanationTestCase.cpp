using namespace std;

#include "BaseExplanationTestCase.h"

namespace org::apache::lucene::search
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::junit::BeforeClass;
//    import static org.apache.lucene.search.spans.SpanTestUtil.*;
shared_ptr<IndexSearcher> BaseExplanationTestCase::searcher;
shared_ptr<org::apache::lucene::index::IndexReader>
    BaseExplanationTestCase::reader;
shared_ptr<org::apache::lucene::store::Directory>
    BaseExplanationTestCase::directory;
shared_ptr<org::apache::lucene::analysis::Analyzer>
    BaseExplanationTestCase::analyzer;
const wstring BaseExplanationTestCase::KEY = L"KEY";
const wstring BaseExplanationTestCase::FIELD = L"field";
const wstring BaseExplanationTestCase::ALTFIELD = L"alt";

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClassTestExplanations()
// throws Exception
void BaseExplanationTestCase::afterClassTestExplanations() 
{
  searcher.reset();
  delete reader;
  reader.reset();
  delete directory;
  directory.reset();
  delete analyzer;
  analyzer.reset();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClassTestExplanations()
// throws Exception
void BaseExplanationTestCase::beforeClassTestExplanations() 
{
  directory = newDirectory();
  analyzer = make_shared<MockAnalyzer>(random());
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.RandomIndexWriter
  // writer = new org.apache.lucene.index.RandomIndexWriter(random(), directory,
  // newIndexWriterConfig(analyzer).setMergePolicy(newLogMergePolicy())))
  {
    org::apache::lucene::index::RandomIndexWriter writer =
        org::apache::lucene::index::RandomIndexWriter(
            random(), directory,
            newIndexWriterConfig(analyzer)->setMergePolicy(
                newLogMergePolicy()));
    for (int i = 0; i < docFields.size(); i++) {
      writer->addDocument(createDoc(i));
    }
    reader = writer->getReader();
    searcher = newSearcher(reader);
  }
}

shared_ptr<Document> BaseExplanationTestCase::createDoc(int index)
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newStringField(KEY, L"" + to_wstring(index), Field::Store::NO));
  doc->push_back(make_shared<SortedDocValuesField>(
      KEY, make_shared<BytesRef>(L"" + to_wstring(index))));
  shared_ptr<Field> f = newTextField(FIELD, docFields[index], Field::Store::NO);
  doc->push_back(f);
  doc->push_back(newTextField(ALTFIELD, docFields[index], Field::Store::NO));
  return doc;
}

std::deque<wstring> const BaseExplanationTestCase::docFields = {
    L"w1 w2 w3 w4 w5", L"w1 w3 w2 w3 zz", L"w1 xx w2 yy w3",
    L"w1 w3 xx w2 yy w3 zz"};

void BaseExplanationTestCase::qtest(
    shared_ptr<Query> q, std::deque<int> &expDocNrs) 
{
  if (random()->nextBoolean()) {
    shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
    bq->add(q, BooleanClause::Occur::SHOULD);
    bq->add(make_shared<TermQuery>(make_shared<Term>(L"NEVER", L"MATCH")),
            BooleanClause::Occur::SHOULD);
    q = bq->build();
  }
  CheckHits::checkHitCollector(random(), q, FIELD, searcher, expDocNrs);
}

void BaseExplanationTestCase::bqtest(
    shared_ptr<Query> q, std::deque<int> &expDocNrs) 
{
  qtest(reqB(q), expDocNrs);
  qtest(optB(q), expDocNrs);
}

shared_ptr<Query>
BaseExplanationTestCase::matchTheseItems(std::deque<int> &terms)
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  for (auto term : terms) {
    query->add(make_shared<BooleanClause>(
        make_shared<TermQuery>(make_shared<Term>(KEY, L"" + to_wstring(term))),
        BooleanClause::Occur::SHOULD));
  }
  return query->build();
}

std::deque<std::shared_ptr<Term>>
BaseExplanationTestCase::ta(std::deque<wstring> &s)
{
  std::deque<std::shared_ptr<Term>> t(s.size());
  for (int i = 0; i < s.size(); i++) {
    t[i] = make_shared<Term>(FIELD, s[i]);
  }
  return t;
}

shared_ptr<SpanQuery> BaseExplanationTestCase::st(const wstring &s)
{
  return SpanTestUtil::spanTermQuery(FIELD, s);
}

shared_ptr<SpanQuery> BaseExplanationTestCase::snot(shared_ptr<SpanQuery> i,
                                                    shared_ptr<SpanQuery> e)
{
  return SpanTestUtil::spanNotQuery(i, e);
}

shared_ptr<SpanQuery> BaseExplanationTestCase::sor(const wstring &s,
                                                   const wstring &e)
{
  return spanOrQuery(FIELD, s, e);
}

shared_ptr<SpanQuery> BaseExplanationTestCase::sor(shared_ptr<SpanQuery> s,
                                                   shared_ptr<SpanQuery> e)
{
  return SpanTestUtil::spanOrQuery({s, e});
}

shared_ptr<SpanQuery> BaseExplanationTestCase::sor(const wstring &s,
                                                   const wstring &m,
                                                   const wstring &e)
{
  return spanOrQuery(FIELD, s, m, e);
}

shared_ptr<SpanQuery> BaseExplanationTestCase::sor(shared_ptr<SpanQuery> s,
                                                   shared_ptr<SpanQuery> m,
                                                   shared_ptr<SpanQuery> e)
{
  return spanOrQuery(s, m, e);
}

shared_ptr<SpanQuery> BaseExplanationTestCase::snear(const wstring &s,
                                                     const wstring &e, int slop,
                                                     bool inOrder)
{
  return snear(st(s), st(e), slop, inOrder);
}

shared_ptr<SpanQuery> BaseExplanationTestCase::snear(shared_ptr<SpanQuery> s,
                                                     shared_ptr<SpanQuery> e,
                                                     int slop, bool inOrder)
{
  if (inOrder) {
    return SpanTestUtil::spanNearOrderedQuery(slop, {s, e});
  } else {
    return SpanTestUtil::spanNearUnorderedQuery(slop, {s, e});
  }
}

shared_ptr<SpanQuery> BaseExplanationTestCase::snear(const wstring &s,
                                                     const wstring &m,
                                                     const wstring &e, int slop,
                                                     bool inOrder)
{
  return snear(st(s), st(m), st(e), slop, inOrder);
}

shared_ptr<SpanQuery> BaseExplanationTestCase::snear(shared_ptr<SpanQuery> s,
                                                     shared_ptr<SpanQuery> m,
                                                     shared_ptr<SpanQuery> e,
                                                     int slop, bool inOrder)
{
  if (inOrder) {
    return spanNearOrderedQuery(slop, s, m, e);
  } else {
    return spanNearUnorderedQuery(slop, s, m, e);
  }
}

shared_ptr<SpanQuery> BaseExplanationTestCase::sf(const wstring &s, int b)
{
  return SpanTestUtil::spanFirstQuery(st(s), b);
}

shared_ptr<Query>
BaseExplanationTestCase::optB(shared_ptr<Query> q) 
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(q, BooleanClause::Occur::SHOULD);
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"NEVER", L"MATCH")),
          BooleanClause::Occur::MUST_NOT);
  return bq->build();
}

shared_ptr<Query>
BaseExplanationTestCase::reqB(shared_ptr<Query> q) 
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(q, BooleanClause::Occur::MUST);
  bq->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
          BooleanClause::Occur::SHOULD);
  return bq->build();
}
} // namespace org::apache::lucene::search