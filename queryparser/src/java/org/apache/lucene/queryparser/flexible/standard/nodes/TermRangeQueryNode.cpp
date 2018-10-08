using namespace std;

#include "TermRangeQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::standard::nodes
{
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;

TermRangeQueryNode::TermRangeQueryNode(shared_ptr<FieldQueryNode> lower,
                                       shared_ptr<FieldQueryNode> upper,
                                       bool lowerInclusive, bool upperInclusive)
{
  setBounds(lower, upper, lowerInclusive, upperInclusive);
}
} // namespace org::apache::lucene::queryparser::flexible::standard::nodes