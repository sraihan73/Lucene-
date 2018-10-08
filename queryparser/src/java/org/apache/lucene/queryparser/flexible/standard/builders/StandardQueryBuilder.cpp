using namespace std;

#include "StandardQueryBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryBuilder =
    org::apache::lucene::queryparser::flexible::core::builders::QueryBuilder;
using QueryTreeBuilder = org::apache::lucene::queryparser::flexible::core::
    builders::QueryTreeBuilder;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using Query = org::apache::lucene::search::Query;
} // namespace org::apache::lucene::queryparser::flexible::standard::builders