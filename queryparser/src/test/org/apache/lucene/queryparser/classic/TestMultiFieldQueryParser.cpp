using namespace std;

#include "TestMultiFieldQueryParser.h"

namespace org::apache::lucene::queryparser::classic
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockSynonymFilter = org::apache::lucene::analysis::MockSynonymFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using RegexpQuery = org::apache::lucene::search::RegexpQuery;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestMultiFieldQueryParser::testStopwordsParsing() 
{
  assertStopQueryEquals(L"one", L"b:one t:one");
  assertStopQueryEquals(L"one stop", L"b:one t:one");
  assertStopQueryEquals(L"one (stop)", L"b:one t:one");
  assertStopQueryEquals(L"one ((stop))", L"b:one t:one");
  assertStopQueryEquals(L"stop", L"");
  assertStopQueryEquals(L"(stop)", L"");
  assertStopQueryEquals(L"((stop))", L"");
}

void TestMultiFieldQueryParser::assertStopQueryEquals(
    const wstring &qtxt, const wstring &expectedRes) 
{
  std::deque<wstring> fields = {L"b", L"t"};
  std::deque<BooleanClause::Occur> occur = {BooleanClause::Occur::SHOULD,
                                             BooleanClause::Occur::SHOULD};
  shared_ptr<TestQueryParser::QPTestAnalyzer> a =
      make_shared<TestQueryParser::QPTestAnalyzer>();
  shared_ptr<MultiFieldQueryParser> mfqp =
      make_shared<MultiFieldQueryParser>(fields, a);

  shared_ptr<Query> q = mfqp->parse(qtxt);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(expectedRes, q->toString());

  q = MultiFieldQueryParser::parse(qtxt, fields, occur, a);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(expectedRes, q->toString());
}

void TestMultiFieldQueryParser::testSimple() 
{
  std::deque<wstring> fields = {L"b", L"t"};
  shared_ptr<MultiFieldQueryParser> mfqp = make_shared<MultiFieldQueryParser>(
      fields, make_shared<MockAnalyzer>(random()));

  shared_ptr<Query> q = mfqp->parse(L"one");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b:one t:one", q->toString());

  q = mfqp->parse(L"one two");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one t:one) (b:two t:two)", q->toString());

  q = mfqp->parse(L"+one +two");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(b:one t:one) +(b:two t:two)", q->toString());

  q = mfqp->parse(L"+one -two -three");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(b:one t:one) -(b:two t:two) -(b:three t:three)",
               q->toString());

  q = mfqp->parse(L"one^2 two");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one t:one)^2.0 (b:two t:two)", q->toString());

  q = mfqp->parse(L"one~ two");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one~2 t:one~2) (b:two t:two)", q->toString());

  q = mfqp->parse(L"one~0.8 two^2");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one~0 t:one~0) (b:two t:two)^2.0", q->toString());

  q = mfqp->parse(L"one* two*");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one* t:one*) (b:two* t:two*)", q->toString());

  q = mfqp->parse(L"[a TO c] two");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:[a TO c] t:[a TO c]) (b:two t:two)", q->toString());

  q = mfqp->parse(L"w?ldcard");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b:w?ldcard t:w?ldcard", q->toString());

  q = mfqp->parse(L"\"foo bar\"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b:\"foo bar\" t:\"foo bar\"", q->toString());

  q = mfqp->parse(L"\"aa bb cc\" \"dd ee\"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:\"aa bb cc\" t:\"aa bb cc\") (b:\"dd ee\" t:\"dd ee\")",
               q->toString());

  q = mfqp->parse(L"\"foo bar\"~4");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b:\"foo bar\"~4 t:\"foo bar\"~4", q->toString());

  // LUCENE-1213: MultiFieldQueryParser was ignoring slop when phrase had a
  // field.
  q = mfqp->parse(L"b:\"foo bar\"~4");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b:\"foo bar\"~4", q->toString());

  // make sure that terms which have a field are not touched:
  q = mfqp->parse(L"one f:two");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one t:one) f:two", q->toString());

  // AND mode:
  mfqp->setDefaultOperator(QueryParserBase::AND_OPERATOR);
  q = mfqp->parse(L"one two");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(b:one t:one) +(b:two t:two)", q->toString());
  q = mfqp->parse(L"\"aa bb cc\" \"dd ee\"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(b:\"aa bb cc\" t:\"aa bb cc\") +(b:\"dd ee\" t:\"dd ee\")",
               q->toString());
}

void TestMultiFieldQueryParser::testBoostsSimple() 
{
  unordered_map<wstring, float> boosts = unordered_map<wstring, float>();
  boosts.emplace(L"b", static_cast<Float>(5));
  boosts.emplace(L"t", static_cast<Float>(10));
  std::deque<wstring> fields = {L"b", L"t"};
  shared_ptr<MultiFieldQueryParser> mfqp = make_shared<MultiFieldQueryParser>(
      fields, make_shared<MockAnalyzer>(random()), boosts);

  // Check for simple
  shared_ptr<Query> q = mfqp->parse(L"one");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one)^5.0 (t:one)^10.0", q->toString());

  // Check for AND
  q = mfqp->parse(L"one AND two");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+((b:one)^5.0 (t:one)^10.0) +((b:two)^5.0 (t:two)^10.0)",
               q->toString());

  // Check for OR
  q = mfqp->parse(L"one OR two");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"((b:one)^5.0 (t:one)^10.0) ((b:two)^5.0 (t:two)^10.0)",
               q->toString());

  // Check for AND and a field
  q = mfqp->parse(L"one AND two AND foo:test");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"+((b:one)^5.0 (t:one)^10.0) +((b:two)^5.0 (t:two)^10.0) +foo:test",
      q->toString());

  q = mfqp->parse(L"one^3 AND two^4");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"+((b:one)^5.0 (t:one)^10.0)^3.0 +((b:two)^5.0 (t:two)^10.0)^4.0",
      q->toString());
}

void TestMultiFieldQueryParser::testStaticMethod1() 
{
  std::deque<wstring> fields = {L"b", L"t"};
  std::deque<wstring> queries = {L"one", L"two"};
  shared_ptr<Query> q = MultiFieldQueryParser::parse(
      queries, fields, make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b:one t:two", q->toString());

  std::deque<wstring> queries2 = {L"+one", L"+two"};
  q = MultiFieldQueryParser::parse(queries2, fields,
                                   make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(+b:one) (+t:two)", q->toString());

  std::deque<wstring> queries3 = {L"one", L"+two"};
  q = MultiFieldQueryParser::parse(queries3, fields,
                                   make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"b:one (+t:two)", q->toString());

  std::deque<wstring> queries4 = {L"one +more", L"+two"};
  q = MultiFieldQueryParser::parse(queries4, fields,
                                   make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one +b:more) (+t:two)", q->toString());

  std::deque<wstring> queries5 = {L"blah"};
  // expected exception, array length differs
  expectThrows(invalid_argument::typeid, [&]() {
    MultiFieldQueryParser::parse(queries5, fields,
                                 make_shared<MockAnalyzer>(random()));
  });

  // check also with stop words for this static form (qtxts[], fields[]).
  shared_ptr<TestQueryParser::QPTestAnalyzer> stopA =
      make_shared<TestQueryParser::QPTestAnalyzer>();

  std::deque<wstring> queries6 = {L"((+stop))", L"+((stop))"};
  q = MultiFieldQueryParser::parse(queries6, fields, stopA);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"", q->toString());

  std::deque<wstring> queries7 = {L"one ((+stop)) +more", L"+((stop)) +two"};
  q = MultiFieldQueryParser::parse(queries7, fields, stopA);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:one +b:more) (+t:two)", q->toString());
}

void TestMultiFieldQueryParser::testStaticMethod2() 
{
  std::deque<wstring> fields = {L"b", L"t"};
  std::deque<BooleanClause::Occur> flags = {BooleanClause::Occur::MUST,
                                             BooleanClause::Occur::MUST_NOT};
  shared_ptr<Query> q = MultiFieldQueryParser::parse(
      L"one", fields, flags, make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+b:one -t:one", q->toString());

  q = MultiFieldQueryParser::parse(L"one two", fields, flags,
                                   make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(b:one b:two) -(t:one t:two)", q->toString());

  // expected exception, array length differs
  expectThrows(invalid_argument::typeid, [&]() {
    std::deque<BooleanClause::Occur> flags2 = {BooleanClause::Occur::MUST};
    MultiFieldQueryParser::parse(L"blah", fields, flags2,
                                 make_shared<MockAnalyzer>(random()));
  });
}

void TestMultiFieldQueryParser::testStaticMethod2Old() 
{
  std::deque<wstring> fields = {L"b", L"t"};
  // int[] flags = {MultiFieldQueryParser.REQUIRED_FIELD,
  // MultiFieldQueryParser.PROHIBITED_FIELD};
  std::deque<BooleanClause::Occur> flags = {BooleanClause::Occur::MUST,
                                             BooleanClause::Occur::MUST_NOT};

  shared_ptr<Query> q = MultiFieldQueryParser::parse(
      L"one", fields, flags,
      make_shared<MockAnalyzer>(
          random())); //, fields, flags, new MockAnalyzer(random));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+b:one -t:one", q->toString());

  q = MultiFieldQueryParser::parse(L"one two", fields, flags,
                                   make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(b:one b:two) -(t:one t:two)", q->toString());

  // expected exception, array length differs
  expectThrows(invalid_argument::typeid, [&]() {
    std::deque<BooleanClause::Occur> flags2 = {BooleanClause::Occur::MUST};
    MultiFieldQueryParser::parse(L"blah", fields, flags2,
                                 make_shared<MockAnalyzer>(random()));
  });
}

void TestMultiFieldQueryParser::testStaticMethod3() 
{
  std::deque<wstring> queries = {L"one", L"two", L"three"};
  std::deque<wstring> fields = {L"f1", L"f2", L"f3"};
  std::deque<BooleanClause::Occur> flags = {BooleanClause::Occur::MUST,
                                             BooleanClause::Occur::MUST_NOT,
                                             BooleanClause::Occur::SHOULD};
  shared_ptr<Query> q = MultiFieldQueryParser::parse(
      queries, fields, flags, make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+f1:one -f2:two f3:three", q->toString());

  // expected exception, array length differs
  expectThrows(invalid_argument::typeid, [&]() {
    std::deque<BooleanClause::Occur> flags2 = {BooleanClause::Occur::MUST};
    MultiFieldQueryParser::parse(queries, fields, flags2,
                                 make_shared<MockAnalyzer>(random()));
  });
}

void TestMultiFieldQueryParser::testStaticMethod3Old() 
{
  std::deque<wstring> queries = {L"one", L"two"};
  std::deque<wstring> fields = {L"b", L"t"};
  std::deque<BooleanClause::Occur> flags = {BooleanClause::Occur::MUST,
                                             BooleanClause::Occur::MUST_NOT};
  shared_ptr<Query> q = MultiFieldQueryParser::parse(
      queries, fields, flags, make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+b:one -t:two", q->toString());

  // expected exception, array length differs
  expectThrows(invalid_argument::typeid, [&]() {
    std::deque<BooleanClause::Occur> flags2 = {BooleanClause::Occur::MUST};
    MultiFieldQueryParser::parse(queries, fields, flags2,
                                 make_shared<MockAnalyzer>(random()));
  });
}

void TestMultiFieldQueryParser::testAnalyzerReturningNull() throw(
    ParseException)
{
  std::deque<wstring> fields = {L"f1", L"f2", L"f3"};
  shared_ptr<MultiFieldQueryParser> parser = make_shared<MultiFieldQueryParser>(
      fields, make_shared<AnalyzerReturningNull>());
  shared_ptr<Query> q = parser->parse(L"bla AND blo");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(f2:bla f3:bla) +(f2:blo f3:blo)", q->toString());
  // the following queries are not affected as their terms are not analyzed
  // anyway:
  q = parser->parse(L"bla*");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"f1:bla* f2:bla* f3:bla*", q->toString());
  q = parser->parse(L"bla~");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"f1:bla~2 f2:bla~2 f3:bla~2", q->toString());
  q = parser->parse(L"[a TO c]");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"f1:[a TO c] f2:[a TO c] f3:[a TO c]", q->toString());
}

void TestMultiFieldQueryParser::testStopWordSearching() 
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

  shared_ptr<MultiFieldQueryParser> mfqp = make_shared<MultiFieldQueryParser>(
      std::deque<wstring>{L"body"}, analyzer);
  mfqp->setDefaultOperator(QueryParser::Operator::AND);
  shared_ptr<Query> q = mfqp->parse(L"the footest");
  shared_ptr<IndexReader> ir = DirectoryReader::open(ramDir);
  shared_ptr<IndexSearcher> is = newSearcher(ir);
  std::deque<std::shared_ptr<ScoreDoc>> hits = is->search(q, 1000)->scoreDocs;
  assertEquals(1, hits.size());
  delete ir;
  delete ramDir;
}

TestMultiFieldQueryParser::AnalyzerReturningNull::AnalyzerReturningNull()
    : org::apache::lucene::analysis::Analyzer(PER_FIELD_REUSE_STRATEGY)
{
}

shared_ptr<Reader> TestMultiFieldQueryParser::AnalyzerReturningNull::initReader(
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

shared_ptr<Analyzer::TokenStreamComponents>
TestMultiFieldQueryParser::AnalyzerReturningNull::createComponents(
    const wstring &fieldName)
{
  return stdAnalyzer->createComponents(fieldName);
}

void TestMultiFieldQueryParser::testSimpleRegex() 
{
  std::deque<wstring> fields = {L"a", L"b"};
  shared_ptr<MultiFieldQueryParser> mfqp = make_shared<MultiFieldQueryParser>(
      fields, make_shared<MockAnalyzer>(random()));

  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<RegexpQuery>(make_shared<Term>(L"a", L"[a-z][123]")),
          BooleanClause::Occur::SHOULD);
  bq->add(make_shared<RegexpQuery>(make_shared<Term>(L"b", L"[a-z][123]")),
          BooleanClause::Occur::SHOULD);
  assertEquals(bq->build(), mfqp->parse(L"/[a-z][123]/"));
}

shared_ptr<Analyzer::TokenStreamComponents>
TestMultiFieldQueryParser::MockSynonymAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<MockSynonymFilter>(tokenizer));
}

void TestMultiFieldQueryParser::testSynonyms() 
{
  std::deque<wstring> fields = {L"b", L"t"};
  shared_ptr<MultiFieldQueryParser> parser = make_shared<MultiFieldQueryParser>(
      fields, make_shared<MockSynonymAnalyzer>());
  shared_ptr<Query> q = parser->parse(L"dogs");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Synonym(b:dog b:dogs) Synonym(t:dog t:dogs)", q->toString());
  q = parser->parse(L"guinea pig");
  assertFalse(parser->getSplitOnWhitespace());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"((+b:guinea +b:pig) b:cavy) ((+t:guinea +t:pig) t:cavy)",
               q->toString());
  parser->setSplitOnWhitespace(true);
  q = parser->parse(L"guinea pig");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(b:guinea t:guinea) (b:pig t:pig)", q->toString());
}
} // namespace org::apache::lucene::queryparser::classic