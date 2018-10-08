using namespace std;

#include "ExtensionStub.h"

namespace org::apache::lucene::queryparser::ext
{
using Term = org::apache::lucene::index::Term;
using ParseException =
    org::apache::lucene::queryparser::classic::ParseException;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;

shared_ptr<Query> ExtensionStub::parse(
    shared_ptr<ExtensionQuery> components) 
{
  return make_shared<TermQuery>(make_shared<Term>(
      components->getField(), components->getRawQueryString()));
}
} // namespace org::apache::lucene::queryparser::ext