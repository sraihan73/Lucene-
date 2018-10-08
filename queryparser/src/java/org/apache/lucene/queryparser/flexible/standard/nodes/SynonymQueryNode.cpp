using namespace std;

#include "SynonymQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::standard::nodes
{
using BooleanQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::BooleanQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;

SynonymQueryNode::SynonymQueryNode(deque<std::shared_ptr<QueryNode>> &clauses)
    : org::apache::lucene::queryparser::flexible::core::nodes::BooleanQueryNode(
          clauses)
{
}
} // namespace org::apache::lucene::queryparser::flexible::standard::nodes