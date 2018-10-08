using namespace std;

#include "TestCorePlusExtensionsParser.h"

namespace org::apache::lucene::queryparser::xml
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using Query = org::apache::lucene::search::Query;

shared_ptr<CoreParser>
TestCorePlusExtensionsParser::newCoreParser(const wstring &defaultField,
                                            shared_ptr<Analyzer> analyzer)
{
  return make_shared<CorePlusExtensionsParser>(defaultField, analyzer);
}

void TestCorePlusExtensionsParser::testFuzzyLikeThisQueryXML() throw(
    runtime_error)
{
  shared_ptr<Query> q = parse(L"FuzzyLikeThisQuery.xml");
  // show rewritten fuzzyLikeThisQuery - see what is being matched on
  if (VERBOSE) {
    wcout << rewrite(q) << endl;
  }
  dumpResults(L"FuzzyLikeThis", q, 5);
}
} // namespace org::apache::lucene::queryparser::xml