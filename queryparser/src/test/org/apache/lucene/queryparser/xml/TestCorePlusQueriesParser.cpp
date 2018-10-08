using namespace std;

#include "TestCorePlusQueriesParser.h"

namespace org::apache::lucene::queryparser::xml
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using Query = org::apache::lucene::search::Query;

shared_ptr<CoreParser>
TestCorePlusQueriesParser::newCoreParser(const wstring &defaultField,
                                         shared_ptr<Analyzer> analyzer)
{
  return make_shared<CorePlusQueriesParser>(defaultField, analyzer);
}

void TestCorePlusQueriesParser::testLikeThisQueryXML() 
{
  shared_ptr<Query> q = parse(L"LikeThisQuery.xml");
  dumpResults(L"like this", q, 5);
}

void TestCorePlusQueriesParser::testBoostingQueryXML() 
{
  shared_ptr<Query> q = parse(L"BoostingQuery.xml");
  dumpResults(L"boosting ", q, 5);
}
} // namespace org::apache::lucene::queryparser::xml