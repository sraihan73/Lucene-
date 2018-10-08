using namespace std;

#include "ExceptionQueryTst.h"

namespace org::apache::lucene::queryparser::surround::query
{
using ParseException =
    org::apache::lucene::queryparser::surround::parser::ParseException;
using QueryParser =
    org::apache::lucene::queryparser::surround::parser::QueryParser;

ExceptionQueryTst::ExceptionQueryTst(const wstring &queryText, bool verbose)
{
  this->queryText = queryText;
  this->verbose = verbose;
}

void ExceptionQueryTst::doTest(shared_ptr<StringBuilder> failQueries)
{
  bool pass = false;
  shared_ptr<SrndQuery> lq = nullptr;
  try {
    lq = QueryParser::parse(queryText);
    if (verbose) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wcout << L"Query: " << queryText << L"\nParsed as: " << lq->toString()
            << endl;
    }
  } catch (const ParseException &e) {
    if (verbose) {
      wcout << L"Parse exception for query:\n"
            << queryText << L"\n"
            << e->what() << endl;
    }
    pass = true;
  }
  if (!pass) {
    failQueries->append(queryText);
    failQueries->append(L"\nParsed as: ");
    // C++ TODO: There is no native C++ equivalent to 'toString':
    failQueries->append(lq->toString());
    failQueries->append(L"\n");
  }
}

wstring
ExceptionQueryTst::getFailQueries(std::deque<wstring> &exceptionQueries,
                                  bool verbose)
{
  shared_ptr<StringBuilder> failQueries = make_shared<StringBuilder>();
  for (int i = 0; i < exceptionQueries.size(); i++) {
    (make_shared<ExceptionQueryTst>(exceptionQueries[i], verbose))
        ->doTest(failQueries);
  }
  return failQueries->toString();
}
} // namespace org::apache::lucene::queryparser::surround::query