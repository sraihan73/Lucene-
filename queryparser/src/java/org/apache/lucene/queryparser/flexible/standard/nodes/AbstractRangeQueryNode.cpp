using namespace std;

#include "AbstractRangeQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::standard::nodes
{
using FieldValuePairQueryNode = org::apache::lucene::queryparser::flexible::
    core::nodes::FieldValuePairQueryNode;
using FieldableNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldableNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeImpl =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNodeImpl;
using RangeQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::RangeQueryNode;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;
using StringUtils =
    org::apache::lucene::queryparser::flexible::core::util::StringUtils;
} // namespace org::apache::lucene::queryparser::flexible::standard::nodes