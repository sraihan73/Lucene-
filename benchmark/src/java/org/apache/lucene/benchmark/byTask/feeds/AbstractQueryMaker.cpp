using namespace std;

#include "AbstractQueryMaker.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../utils/Config.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using Query = org::apache::lucene::search::Query;

void AbstractQueryMaker::resetInputs() 
{
  qnum = 0;
  // re-initialize since properties by round may have changed.
  setConfig(config);
}

void AbstractQueryMaker::setConfig(shared_ptr<Config> config) throw(
    runtime_error)
{
  this->config = config;
  queries = prepareQueries();
}

wstring AbstractQueryMaker::printQueries()
{
  wstring newline = System::getProperty(L"line.separator");
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  if (queries.size() > 0) {
    for (int i = 0; i < queries.size(); i++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      sb->append(to_wstring(i) + L". " +
                 queries[i]->getClass().getSimpleName() + L" - " +
                 queries[i]->toString());
      sb->append(newline);
    }
  }
  return sb->toString();
}

shared_ptr<Query> AbstractQueryMaker::makeQuery() 
{
  return queries[nextQnum()];
}

// C++ WARNING: The following method was originally marked 'synchronized':
int AbstractQueryMaker::nextQnum()
{
  int res = qnum;
  qnum = (qnum + 1) % queries.size();
  return res;
}

shared_ptr<Query> AbstractQueryMaker::makeQuery(int size) 
{
  throw runtime_error(shared_from_this() +
                      L".makeQuery(int size) is not supported!");
}
} // namespace org::apache::lucene::benchmark::byTask::feeds