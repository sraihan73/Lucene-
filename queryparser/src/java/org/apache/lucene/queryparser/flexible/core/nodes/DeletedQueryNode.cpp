using namespace std;

#include "DeletedQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;
using RemoveDeletedQueryNodesProcessor = org::apache::lucene::queryparser::
    flexible::core::processors::RemoveDeletedQueryNodesProcessor;

DeletedQueryNode::DeletedQueryNode()
{
  // empty constructor
}

shared_ptr<std::wstring>
DeletedQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escaper)
{
  return L"[DELETEDCHILD]";
}

wstring DeletedQueryNode::toString() { return L"<deleted/>"; }

shared_ptr<QueryNode>
DeletedQueryNode::cloneTree() 
{
  shared_ptr<DeletedQueryNode> clone =
      std::static_pointer_cast<DeletedQueryNode>(QueryNodeImpl::cloneTree());

  return clone;
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes