using namespace std;

#include "CorePlusQueriesParser.h"

namespace org::apache::lucene::queryparser::xml
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using LikeThisQueryBuilder =
    org::apache::lucene::queryparser::xml::builders::LikeThisQueryBuilder;
using BoostingQueryBuilder =
    org::apache::lucene::queryparser::xml::builders::BoostingQueryBuilder;

CorePlusQueriesParser::CorePlusQueriesParser(shared_ptr<Analyzer> analyzer,
                                             shared_ptr<QueryParser> parser)
    : CorePlusQueriesParser(nullptr, analyzer, parser)
{
}

CorePlusQueriesParser::CorePlusQueriesParser(const wstring &defaultField,
                                             shared_ptr<Analyzer> analyzer)
    : CorePlusQueriesParser(defaultField, analyzer, nullptr)
{
}

CorePlusQueriesParser::CorePlusQueriesParser(const wstring &defaultField,
                                             shared_ptr<Analyzer> analyzer,
                                             shared_ptr<QueryParser> parser)
    : CoreParser(defaultField, analyzer, parser)
{
  std::deque<wstring> fields = {L"contents"};
  queryFactory->addBuilder(L"LikeThisQuery",
                           make_shared<LikeThisQueryBuilder>(analyzer, fields));
  queryFactory->addBuilder(L"BoostingQuery",
                           make_shared<BoostingQueryBuilder>(queryFactory));
}
} // namespace org::apache::lucene::queryparser::xml