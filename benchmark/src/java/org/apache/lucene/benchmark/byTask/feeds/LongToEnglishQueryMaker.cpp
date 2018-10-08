using namespace std;

#include "LongToEnglishQueryMaker.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../../../queryparser/src/java/org/apache/lucene/queryparser/classic/QueryParser.h"
#include "../tasks/NewAnalyzerTask.h"
#include "../utils/Config.h"
#include "DocMaker.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
using NewAnalyzerTask =
    org::apache::lucene::benchmark::byTask::tasks::NewAnalyzerTask;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using Query = org::apache::lucene::search::Query;
using com::ibm::icu::text::RuleBasedNumberFormat;

shared_ptr<Query>
LongToEnglishQueryMaker::makeQuery(int size) 
{
  throw make_shared<UnsupportedOperationException>();
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Query> LongToEnglishQueryMaker::makeQuery() 
{
  return parser->parse(L"" + rnbf->format(getNextCounter()) + L"");
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t LongToEnglishQueryMaker::getNextCounter()
{
  if (counter == numeric_limits<int64_t>::max()) {
    counter = numeric_limits<int64_t>::min() + 10;
  }
  return counter++;
}

void LongToEnglishQueryMaker::setConfig(shared_ptr<Config> config) throw(
    runtime_error)
{
  shared_ptr<Analyzer> anlzr = NewAnalyzerTask::createAnalyzer(
      config->get(L"analyzer", StandardAnalyzer::typeid->getName()));
  parser = make_shared<QueryParser>(DocMaker::BODY_FIELD, anlzr);
}

void LongToEnglishQueryMaker::resetInputs()
{
  counter = numeric_limits<int64_t>::min() + 10;
}

wstring LongToEnglishQueryMaker::printQueries()
{
  return L"LongToEnglish: [" + numeric_limits<int64_t>::min() + L" TO " +
         to_wstring(counter) + L"]";
}
} // namespace org::apache::lucene::benchmark::byTask::feeds