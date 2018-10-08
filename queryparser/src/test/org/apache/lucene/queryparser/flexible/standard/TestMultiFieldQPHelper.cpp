using namespace std;

#include "TestMultiFieldQPHelper.h"

namespace org::apache::lucene::queryparser::flexible::standard
{
using namespace org::apache::lucene::analysis;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using StandardQueryConfigHandler = org::apache::lucene::queryparser::flexible::
    standard::config::StandardQueryConfigHandler;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestMultiFieldQPHelper::testStopwordsParsing() 
{
  assertStopQueryEquals(L"one", L"b:one t:one");
  assertStopQueryEquals(L"one stop", L"b:one t:one");
  assertStopQueryEquals(L"one (stop)", L"b:one t:one");
  assertStopQueryEquals(L"one ((stop))", L"b:one t:one");
  assertStopQueryIsMatchNoDocsQuery(L"stop");
  assertStopQueryIsMatchNoDocsQuery(L"(stop)");
  assertStopQueryIsMatchNoDocsQuery(L"((stop))");
}

void TestMultiFieldQPHelper::assertStopQueryIsMatchNoDocsQuery(
    const wstring &qtxt) 
{
  std::deque<wstring> fields = {L"b", L"t"};
  std::deque<Occur> occur = {Occur::SHOULD, Occur::SHOULD};
  shared_ptr<TestQPHelper::QPTestAnalyzer> a =
      make_shared<TestQPHelper::QPTestAnalyzer>();
  shared_ptr<StandardQueryParser> mfqp = make_shared<StandardQueryParser>();
  mfqp->setMultiFields(fields);
  mfqp->setAnalyzer(a);

  shared_ptr<Query> q = mfqp->parse(qtxt, L"");
  assertTrue(std::dynamic_pointer_cast<MatchNoDocsQuery>(q) != nullptr);
}

void TestMultiFieldQPHelper::assertStopQueryEquals(
    const wstring &qtxt, const wstring &expectedRes) 
{
  std::deque<wstring> fields = {L"b", L"t"};
  std::deque<Occur> occur = {Occur::SHOULD, Occur::SHOULD};
  shared_ptr<TestQPHelper::QPTestAnalyzer> a =
      make_shared<TestQPHelper::QPTestAnalyzer>();
  shared_ptr<StandardQueryParser> mfqp = make_shared<StandardQueryParser>();
  mfqp->setMultiFields(fields);
  mfqp->setAnalyzer(a);

  shared_ptr<Query> q = mfqp->parse(qtxt, L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(expectedRes, StringHelper::trim(q->toString()));

  q = QueryParserUtil::parse(qtxt, fields, occur, a);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(expectedRes, StringHelper::trim(q->toString()));
}

void TestMultiFieldQPHelper::testSimple() 
{
  std::deque<wstring> fields = {L"b", L"t"};
  shared_ptr<StandardQueryParser> mfqp = make_shared<StandardQueryParser>();
  mfqp->setMultiFields(fields);
  mfqp->setAnalyzer(make_shared<MockAnalyzer>(random()));

  shared_ptr<Query> q = mfqp->parse(L"one", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b:one t:one", q->toString());

  q = mfqp->parse(L"one two", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one t:one) (b:two t:two)", q->toString());

  q = mfqp->parse(L"+one +two", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(b:one t:one) +(b:two t:two)", q->toString());

  q = mfqp->parse(L"+one -two -three", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(b:one t:one) -(b:two t:two) -(b:three t:three)",
               q->toString());

  q = mfqp->parse(L"one^2 two", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one t:one)^2.0 (b:two t:two)", q->toString());

  q = mfqp->parse(L"one~ two", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one~2 t:one~2) (b:two t:two)", q->toString());

  q = mfqp->parse(L"one~0.8 two^2", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one~0 t:one~0) (b:two t:two)^2.0", q->toString());

  q = mfqp->parse(L"one* two*", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one* t:one*) (b:two* t:two*)", q->toString());

  q = mfqp->parse(L"[a TO c] two", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:[a TO c] t:[a TO c]) (b:two t:two)", q->toString());

  q = mfqp->parse(L"w?ldcard", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b:w?ldcard t:w?ldcard", q->toString());

  q = mfqp->parse(L"\"foo bar\"", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b:\"foo bar\" t:\"foo bar\"", q->toString());

  q = mfqp->parse(L"\"aa bb cc\" \"dd ee\"", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:\"aa bb cc\" t:\"aa bb cc\") (b:\"dd ee\" t:\"dd ee\")",
               q->toString());

  q = mfqp->parse(L"\"foo bar\"~4", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b:\"foo bar\"~4 t:\"foo bar\"~4", q->toString());

  // LUCENE-1213: QueryParser was ignoring slop when phrase
  // had a field.
  q = mfqp->parse(L"b:\"foo bar\"~4", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b:\"foo bar\"~4", q->toString());

  // make sure that terms which have a field are not touched:
  q = mfqp->parse(L"one f:two", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one t:one) f:two", q->toString());

  // AND mode:
  mfqp->setDefaultOperator(StandardQueryConfigHandler::Operator::AND);
  q = mfqp->parse(L"one two", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(b:one t:one) +(b:two t:two)", q->toString());
  q = mfqp->parse(L"\"aa bb cc\" \"dd ee\"", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(b:\"aa bb cc\" t:\"aa bb cc\") +(b:\"dd ee\" t:\"dd ee\")",
               q->toString());
}

void TestMultiFieldQPHelper::testBoostsSimple() 
{
  unordered_map<wstring, float> boosts = unordered_map<wstring, float>();
  boosts.emplace(L"b", static_cast<Float>(5));
  boosts.emplace(L"t", static_cast<Float>(10));
  std::deque<wstring> fields = {L"b", L"t"};
  shared_ptr<StandardQueryParser> mfqp = make_shared<StandardQueryParser>();
  mfqp->setMultiFields(fields);
  mfqp->setFieldsBoost(boosts);
  mfqp->setAnalyzer(make_shared<MockAnalyzer>(random()));

  // Check for simple
  shared_ptr<Query> q = mfqp->parse(L"one", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one)^5.0 (t:one)^10.0", q->toString());

  // Check for AND
  q = mfqp->parse(L"one AND two", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+((b:one)^5.0 (t:one)^10.0) +((b:two)^5.0 (t:two)^10.0)",
               q->toString());

  // Check for OR
  q = mfqp->parse(L"one OR two", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"((b:one)^5.0 (t:one)^10.0) ((b:two)^5.0 (t:two)^10.0)",
               q->toString());

  // Check for AND and a field
  q = mfqp->parse(L"one AND two AND foo:test", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"+((b:one)^5.0 (t:one)^10.0) +((b:two)^5.0 (t:two)^10.0) +foo:test",
      q->toString());

  q = mfqp->parse(L"one^3 AND two^4", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"+((b:one)^5.0 (t:one)^10.0)^3.0 +((b:two)^5.0 (t:two)^10.0)^4.0",
      q->toString());
}

void TestMultiFieldQPHelper::testStaticMethod1() 
{
  std::deque<wstring> fields = {L"b", L"t"};
  std::deque<wstring> queries = {L"one", L"two"};
  shared_ptr<Query> q = QueryParserUtil::parse(
      queries, fields, make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b:one t:two", q->toString());

  std::deque<wstring> queries2 = {L"+one", L"+two"};
  q = QueryParserUtil::parse(queries2, fields,
                             make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b:one t:two", q->toString());

  std::deque<wstring> queries3 = {L"one", L"+two"};
  q = QueryParserUtil::parse(queries3, fields,
                             make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b:one t:two", q->toString());

  std::deque<wstring> queries4 = {L"one +more", L"+two"};
  q = QueryParserUtil::parse(queries4, fields,
                             make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one +b:more) t:two", q->toString());

  std::deque<wstring> queries5 = {L"blah"};
  // expected exception, array length differs
  expectThrows(invalid_argument::typeid, [&]() {
    QueryParserUtil::parse(queries5, fields,
                           make_shared<MockAnalyzer>(random()));
  });

  // check also with stop words for this static form (qtxts[], fields[]).
  shared_ptr<TestQPHelper::QPTestAnalyzer> stopA =
      make_shared<TestQPHelper::QPTestAnalyzer>();

  std::deque<wstring> queries6 = {L"((+stop))", L"+((stop))"};
  q = QueryParserUtil::parse(queries6, fields, stopA);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"MatchNoDocsQuery(\"\") MatchNoDocsQuery(\"\")", q->toString());
  // assertEquals(" ", q.toString());

  std::deque<wstring> queries7 = {L"one ((+stop)) +more", L"+((stop)) +two"};
  q = QueryParserUtil::parse(queries7, fields, stopA);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one +b:more) (+t:two)", q->toString());
}

void TestMultiFieldQPHelper::testStaticMethod2() 
{
  std::deque<wstring> fields = {L"b", L"t"};
  std::deque<Occur> flags = {Occur::MUST, Occur::MUST_NOT};
  shared_ptr<Query> q = QueryParserUtil::parse(
      L"one", fields, flags, make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+b:one -t:one", q->toString());

  q = QueryParserUtil::parse(L"one two", fields, flags,
                             make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(b:one b:two) -(t:one t:two)", q->toString());

  // expected exception, array length differs
  expectThrows(invalid_argument::typeid, [&]() {
    std::deque<BooleanClause::Occur> flags2 = {BooleanClause::Occur::MUST};
    QueryParserUtil::parse(L"blah", fields, flags2,
                           make_shared<MockAnalyzer>(random()));
  });
}

void TestMultiFieldQPHelper::testStaticMethod2Old() 
{
  std::deque<wstring> fields = {L"b", L"t"};
  std::deque<Occur> flags = {Occur::MUST, Occur::MUST_NOT};
  shared_ptr<StandardQueryParser> parser = make_shared<StandardQueryParser>();
  parser->setMultiFields(fields);
  parser->setAnalyzer(make_shared<MockAnalyzer>(random()));

  shared_ptr<Query> q = QueryParserUtil::parse(
      L"one", fields, flags,
      make_shared<MockAnalyzer>(random())); // , fields, flags, new
  // MockAnalyzer());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+b:one -t:one", q->toString());

  q = QueryParserUtil::parse(L"one two", fields, flags,
                             make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(b:one b:two) -(t:one t:two)", q->toString());

  // expected exception, array length differs
  expectThrows(invalid_argument::typeid, [&]() {
    std::deque<BooleanClause::Occur> flags2 = {BooleanClause::Occur::MUST};
    QueryParserUtil::parse(L"blah", fields, flags2,
                           make_shared<MockAnalyzer>(random()));
  });
}

void TestMultiFieldQPHelper::testStaticMethod3() 
{
  std::deque<wstring> queries = {L"one", L"two", L"three"};
  std::deque<wstring> fields = {L"f1", L"f2", L"f3"};
  std::deque<Occur> flags = {Occur::MUST, Occur::MUST_NOT, Occur::SHOULD};
  shared_ptr<Query> q = QueryParserUtil::parse(
      queries, fields, flags, make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+f1:one -f2:two f3:three", q->toString());

  // expected exception, array length differs
  expectThrows(invalid_argument::typeid, [&]() {
    std::deque<BooleanClause::Occur> flags2 = {BooleanClause::Occur::MUST};
    QueryParserUtil::parse(queries, fields, flags2,
                           make_shared<MockAnalyzer>(random()));
  });
}

void TestMultiFieldQPHelper::testStaticMethod3Old() 
{
  std::deque<wstring> queries = {L"one", L"two"};
  std::deque<wstring> fields = {L"b", L"t"};
  std::deque<Occur> flags = {Occur::MUST, Occur::MUST_NOT};
  shared_ptr<Query> q = QueryParserUtil::parse(
      queries, fields, flags, make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+b:one -t:two", q->toString());

  // expected exception, array length differs
  expectThrows(invalid_argument::typeid, [&]() {
    std::deque<BooleanClause::Occur> flags2 = {BooleanClause::Occur::MUST};
    QueryParserUtil::parse(queries, fields, flags2,
                           make_shared<MockAnalyzer>(random()));
  });
}

void TestMultiFieldQPHelper::testAnalyzerReturningNull() throw(
    QueryNodeException)
{
  std::deque<wstring> fields = {L"f1", L"f2", L"f3"};
  shared_ptr<StandardQueryParser> parser = make_shared<StandardQueryParser>();
  parser->setMultiFields(fields);
  parser->setAnalyzer(make_shared<AnalyzerReturningNull>());

  shared_ptr<Query> q = parser->parse(L"bla AND blo", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(f2:bla f3:bla) +(f2:blo f3:blo)", q->toString());
  // the following queries are not affected as their terms are not
  // analyzed anyway:
  q = parser->parse(L"bla*", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"f1:bla* f2:bla* f3:bla*", q->toString());
  q = parser->parse(L"bla~", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"f1:bla~2 f2:bla~2 f3:bla~2", q->toString());
  q = parser->parse(L"[a TO c]", L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"f1:[a TO c] f2:[a TO c] f3:[a TO c]", q->toString());
}

void TestMultiFieldQPHelper::testStopWordSearching() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<Directory> ramDir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(ramDir, newIndexWriterConfig(analyzer));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"body", L"blah the footest blah", Field::Store::NO));
  iw->addDocument(doc);
  delete iw;

  shared_ptr<StandardQueryParser> mfqp = make_shared<StandardQueryParser>();

  mfqp->setMultiFields(std::deque<wstring>{L"body"});
  mfqp->setAnalyzer(analyzer);
  mfqp->setDefaultOperator(StandardQueryConfigHandler::Operator::AND);
  shared_ptr<Query> q = mfqp->parse(L"the footest", L"");
  shared_ptr<IndexReader> ir = DirectoryReader::open(ramDir);
  shared_ptr<IndexSearcher> is = newSearcher(ir);
  std::deque<std::shared_ptr<ScoreDoc>> hits = is->search(q, 1000)->scoreDocs;
  assertEquals(1, hits.size());
  delete ir;
  delete ramDir;
}

TestMultiFieldQPHelper::AnalyzerReturningNull::AnalyzerReturningNull()
    : Analyzer(PER_FIELD_REUSE_STRATEGY)
{
}

shared_ptr<Reader> TestMultiFieldQPHelper::AnalyzerReturningNull::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  if (L"f1" == fieldName) {
    // we don't use the reader, so close it:
    IOUtils::closeWhileHandlingException({reader});
    // return empty reader, so MockTokenizer returns no tokens:
    return make_shared<StringReader>(L"");
  } else {
    return Analyzer::initReader(fieldName, reader);
  }
}

shared_ptr<TokenStreamComponents>
TestMultiFieldQPHelper::AnalyzerReturningNull::createComponents(
    const wstring &fieldName)
{
  return stdAnalyzer->createComponents(fieldName);
}
} // namespace org::apache::lucene::queryparser::flexible::standard