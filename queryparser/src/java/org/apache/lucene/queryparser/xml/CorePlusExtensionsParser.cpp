using namespace std;

#include "CorePlusExtensionsParser.h"

namespace org::apache::lucene::queryparser::xml
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using FuzzyLikeThisQueryBuilder =
    org::apache::lucene::queryparser::xml::builders::FuzzyLikeThisQueryBuilder;

CorePlusExtensionsParser::CorePlusExtensionsParser(
    shared_ptr<Analyzer> analyzer, shared_ptr<QueryParser> parser)
    : CorePlusExtensionsParser(nullptr, analyzer, parser)
{
}

CorePlusExtensionsParser::CorePlusExtensionsParser(
    const wstring &defaultField, shared_ptr<Analyzer> analyzer)
    : CorePlusExtensionsParser(defaultField, analyzer, nullptr)
{
}

CorePlusExtensionsParser::CorePlusExtensionsParser(
    const wstring &defaultField, shared_ptr<Analyzer> analyzer,
    shared_ptr<QueryParser> parser)
    : CorePlusQueriesParser(defaultField, analyzer, parser)
{
  queryFactory->addBuilder(L"FuzzyLikeThisQuery",
                           make_shared<FuzzyLikeThisQueryBuilder>(analyzer));
}
} // namespace org::apache::lucene::queryparser::xml