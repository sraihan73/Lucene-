using namespace std;

#include "TestExtendableQueryParser.h"

namespace org::apache::lucene::queryparser::ext
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using ParseException =
    org::apache::lucene::queryparser::classic::ParseException;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using QueryParserBase =
    org::apache::lucene::queryparser::classic::QueryParserBase;
using TestQueryParser =
    org::apache::lucene::queryparser::classic::TestQueryParser;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
std::deque<wchar_t> TestExtendableQueryParser::DELIMITERS = {
    Extensions::DEFAULT_EXTENSION_FIELD_DELIMITER, L'-', L'|'};

shared_ptr<QueryParser> TestExtendableQueryParser::getParser(
    shared_ptr<Analyzer> a) 
{
  return getParser(a, nullptr);
}

shared_ptr<QueryParser> TestExtendableQueryParser::getParser(
    shared_ptr<Analyzer> a,
    shared_ptr<Extensions> extensions) 
{
  if (a == nullptr) {
    a = make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  }
  shared_ptr<QueryParser> qp =
      extensions == nullptr
          ? make_shared<ExtendableQueryParser>(getDefaultField(), a)
          : make_shared<ExtendableQueryParser>(getDefaultField(), a,
                                               extensions);
  qp->setDefaultOperator(QueryParserBase::OR_OPERATOR);
  qp->setSplitOnWhitespace(splitOnWhitespace);
  return qp;
}

void TestExtendableQueryParser::testUnescapedExtDelimiter() 
{
  shared_ptr<Extensions> ext = newExtensions(L':');
  ext->add(L"testExt", make_shared<ExtensionStub>());
  shared_ptr<ExtendableQueryParser> parser =
      std::static_pointer_cast<ExtendableQueryParser>(getParser(nullptr, ext));
  expectThrows(ParseException::typeid,
               [&]() { parser->parse(L"aField:testExt:\"foo \\& bar\""); });
}

void TestExtendableQueryParser::testExtFieldUnqoted() 
{
  for (int i = 0; i < DELIMITERS.size(); i++) {
    shared_ptr<Extensions> ext = newExtensions(DELIMITERS[i]);
    ext->add(L"testExt", make_shared<ExtensionStub>());
    shared_ptr<ExtendableQueryParser> parser =
        std::static_pointer_cast<ExtendableQueryParser>(
            getParser(nullptr, ext));
    wstring field = ext->buildExtensionField(L"testExt", L"aField");
    shared_ptr<Query> query =
        parser->parse(wstring::format(Locale::ROOT, L"%s:foo bar", field));
    assertTrue(L"expected instance of BooleanQuery but was " +
                   query->getClass(),
               std::dynamic_pointer_cast<BooleanQuery>(query) != nullptr);
    shared_ptr<BooleanQuery> bquery =
        std::static_pointer_cast<BooleanQuery>(query);
    std::deque<std::shared_ptr<BooleanClause>> clauses =
        bquery->clauses().toArray(
            std::deque<std::shared_ptr<BooleanClause>>(0));
    assertEquals(2, clauses.size());
    shared_ptr<BooleanClause> booleanClause = clauses[0];
    query = booleanClause->getQuery();
    assertTrue(L"expected instance of TermQuery but was " + query->getClass(),
               std::dynamic_pointer_cast<TermQuery>(query) != nullptr);
    shared_ptr<TermQuery> tquery = std::static_pointer_cast<TermQuery>(query);
    assertEquals(L"aField", tquery->getTerm()->field());
    assertEquals(L"foo", tquery->getTerm()->text());

    booleanClause = clauses[1];
    query = booleanClause->getQuery();
    assertTrue(L"expected instance of TermQuery but was " + query->getClass(),
               std::dynamic_pointer_cast<TermQuery>(query) != nullptr);
    tquery = std::static_pointer_cast<TermQuery>(query);
    assertEquals(getDefaultField(), tquery->getTerm()->field());
    assertEquals(L"bar", tquery->getTerm()->text());
  }
}

void TestExtendableQueryParser::testExtDefaultField() 
{
  for (int i = 0; i < DELIMITERS.size(); i++) {
    shared_ptr<Extensions> ext = newExtensions(DELIMITERS[i]);
    ext->add(L"testExt", make_shared<ExtensionStub>());
    shared_ptr<ExtendableQueryParser> parser =
        std::static_pointer_cast<ExtendableQueryParser>(
            getParser(nullptr, ext));
    wstring field = ext->buildExtensionField(L"testExt");
    shared_ptr<Query> parse = parser->parse(
        wstring::format(Locale::ROOT, L"%s:\"foo \\& bar\"", field));
    assertTrue(L"expected instance of TermQuery but was " + parse->getClass(),
               std::dynamic_pointer_cast<TermQuery>(parse) != nullptr);
    shared_ptr<TermQuery> tquery = std::static_pointer_cast<TermQuery>(parse);
    assertEquals(getDefaultField(), tquery->getTerm()->field());
    assertEquals(L"foo & bar", tquery->getTerm()->text());
  }
}

shared_ptr<Extensions>
TestExtendableQueryParser::newExtensions(wchar_t delimiter)
{
  return make_shared<Extensions>(delimiter);
}

void TestExtendableQueryParser::testExtField() 
{
  for (int i = 0; i < DELIMITERS.size(); i++) {
    shared_ptr<Extensions> ext = newExtensions(DELIMITERS[i]);
    ext->add(L"testExt", make_shared<ExtensionStub>());
    shared_ptr<ExtendableQueryParser> parser =
        std::static_pointer_cast<ExtendableQueryParser>(
            getParser(nullptr, ext));
    wstring field = ext->buildExtensionField(L"testExt", L"afield");
    shared_ptr<Query> parse = parser->parse(
        wstring::format(Locale::ROOT, L"%s:\"foo \\& bar\"", field));
    assertTrue(L"expected instance of TermQuery but was " + parse->getClass(),
               std::dynamic_pointer_cast<TermQuery>(parse) != nullptr);
    shared_ptr<TermQuery> tquery = std::static_pointer_cast<TermQuery>(parse);
    assertEquals(L"afield", tquery->getTerm()->field());
    assertEquals(L"foo & bar", tquery->getTerm()->text());
  }
}
} // namespace org::apache::lucene::queryparser::ext