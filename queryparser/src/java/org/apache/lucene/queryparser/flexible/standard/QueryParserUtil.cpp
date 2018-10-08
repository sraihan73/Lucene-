using namespace std;

#include "QueryParserUtil.h"

namespace org::apache::lucene::queryparser::flexible::standard
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;

shared_ptr<Query>
QueryParserUtil::parse(std::deque<wstring> &queries,
                       std::deque<wstring> &fields,
                       shared_ptr<Analyzer> analyzer) 
{
  if (queries.size() != fields.size()) {
    throw invalid_argument(L"queries.length != fields.length");
  }
  shared_ptr<BooleanQuery::Builder> bQuery =
      make_shared<BooleanQuery::Builder>();

  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
  qp->setAnalyzer(analyzer);

  for (int i = 0; i < fields.size(); i++) {
    shared_ptr<Query> q = qp->parse(queries[i], fields[i]);

    if (q != nullptr) { // q never null, just being defensive
      bQuery->add(q, BooleanClause::Occur::SHOULD);
    }
  }
  return bQuery->build();
}

shared_ptr<Query>
QueryParserUtil::parse(const wstring &query, std::deque<wstring> &fields,
                       std::deque<BooleanClause::Occur> &flags,
                       shared_ptr<Analyzer> analyzer) 
{
  if (fields.size() != flags.size()) {
    throw invalid_argument(L"fields.length != flags.length");
  }
  shared_ptr<BooleanQuery::Builder> bQuery =
      make_shared<BooleanQuery::Builder>();

  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
  qp->setAnalyzer(analyzer);

  for (int i = 0; i < fields.size(); i++) {
    shared_ptr<Query> q = qp->parse(query, fields[i]);

    if (q != nullptr) { // q never null, just being defensive
      bQuery->add(q, flags[i]);
    }
  }
  return bQuery->build();
}

shared_ptr<Query>
QueryParserUtil::parse(std::deque<wstring> &queries,
                       std::deque<wstring> &fields,
                       std::deque<BooleanClause::Occur> &flags,
                       shared_ptr<Analyzer> analyzer) 
{
  if (!(queries.size() == fields.size() && queries.size() == flags.size())) {
    throw invalid_argument(
        L"queries, fields, and flags array have have different length");
  }
  shared_ptr<BooleanQuery::Builder> bQuery =
      make_shared<BooleanQuery::Builder>();

  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
  qp->setAnalyzer(analyzer);

  for (int i = 0; i < fields.size(); i++) {
    shared_ptr<Query> q = qp->parse(queries[i], fields[i]);

    if (q != nullptr) { // q never null, just being defensive
      bQuery->add(q, flags[i]);
    }
  }
  return bQuery->build();
}

wstring QueryParserUtil::escape(const wstring &s)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < s.length(); i++) {
    wchar_t c = s[i];
    // These characters are part of the query syntax and must be escaped
    if (c == L'\\' || c == L'+' || c == L'-' || c == L'!' || c == L'(' ||
        c == L')' || c == L':' || c == L'^' || c == L'[' || c == L']' ||
        c == L'\"' || c == L'{' || c == L'}' || c == L'~' || c == L'*' ||
        c == L'?' || c == L'|' || c == L'&' || c == L'/') {
      sb->append(L'\\');
    }
    sb->append(c);
  }
  return sb->toString();
}
} // namespace org::apache::lucene::queryparser::flexible::standard