using namespace std;

#include "SimpleQQParser.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/BooleanClause.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../../../queryparser/src/java/org/apache/lucene/queryparser/classic/ParseException.h"
#include "../../../../../../../../../queryparser/src/java/org/apache/lucene/queryparser/classic/QueryParser.h"
#include "../../../../../../../../../queryparser/src/java/org/apache/lucene/queryparser/classic/QueryParserBase.h"
#include "../QualityQuery.h"

namespace org::apache::lucene::benchmark::quality::utils
{
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
using QualityQuery = org::apache::lucene::benchmark::quality::QualityQuery;
using QualityQueryParser =
    org::apache::lucene::benchmark::quality::QualityQueryParser;
using ParseException =
    org::apache::lucene::queryparser::classic::ParseException;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using QueryParserBase =
    org::apache::lucene::queryparser::classic::QueryParserBase;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;

SimpleQQParser::SimpleQQParser(std::deque<wstring> &qqNames,
                               const wstring &indexField)
{
  this->qqNames = qqNames;
  this->indexField = indexField;
}

SimpleQQParser::SimpleQQParser(const wstring &qqName, const wstring &indexField)
    : SimpleQQParser(new std::wstring[]{qqName}, indexField)
{
}

shared_ptr<Query>
SimpleQQParser::parse(shared_ptr<QualityQuery> qq) 
{
  shared_ptr<QueryParser> qp = queryParser->get();
  if (qp == nullptr) {
    qp = make_shared<QueryParser>(indexField, make_shared<StandardAnalyzer>());
    queryParser->set(qp);
  }
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  for (int i = 0; i < qqNames.size(); i++) {
    bq->add(qp->parse(QueryParserBase::escape(qq->getValue(qqNames[i]))),
            BooleanClause::Occur::SHOULD);
  }

  return bq->build();
}
} // namespace org::apache::lucene::benchmark::quality::utils