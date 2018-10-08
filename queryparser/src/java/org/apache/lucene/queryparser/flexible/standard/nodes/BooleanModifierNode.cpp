using namespace std;

#include "BooleanModifierNode.h"

namespace org::apache::lucene::queryparser::flexible::standard::nodes
{
using ModifierQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::ModifierQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using BooleanQuery2ModifierNodeProcessor = org::apache::lucene::queryparser::
    flexible::standard::processors::BooleanQuery2ModifierNodeProcessor;

BooleanModifierNode::BooleanModifierNode(shared_ptr<QueryNode> node,
                                         Modifier mod)
    : org::apache::lucene::queryparser::flexible::core::nodes::
          ModifierQueryNode(node, mod)
{
}
} // namespace org::apache::lucene::queryparser::flexible::standard::nodes