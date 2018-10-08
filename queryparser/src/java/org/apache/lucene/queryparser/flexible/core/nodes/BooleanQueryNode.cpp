using namespace std;

#include "BooleanQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

BooleanQueryNode::BooleanQueryNode(deque<std::shared_ptr<QueryNode>> &clauses)
{
  setLeaf(false);
  allocate();
  set(clauses);
}

wstring BooleanQueryNode::toString()
{
  if (getChildren().empty() || getChildren().empty()) {
    return L"<bool operation='default'/>";
  }
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"<bool operation='default'>");
  for (auto child : getChildren()) {
    sb->append(L"\n");
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(child->toString());
  }
  sb->append(L"\n</bool>");
  return sb->toString();
}

shared_ptr<std::wstring> BooleanQueryNode::toQueryString(
    shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  if (getChildren().empty() || getChildren().empty()) {
    return L"";
  }

  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  wstring filler = L"";
  for (auto child : getChildren()) {
    sb->append(filler)->append(child->toQueryString(escapeSyntaxParser));
    filler = L" ";
  }

  // in case is root or the parent is a group node avoid parenthesis
  if ((getParent() != nullptr &&
       std::dynamic_pointer_cast<GroupQueryNode>(getParent()) != nullptr) ||
      isRoot()) {
    return sb->toString();
  } else {
    return L"( " + sb->toString() + L" )";
  }
}

shared_ptr<QueryNode>
BooleanQueryNode::cloneTree() 
{
  shared_ptr<BooleanQueryNode> clone =
      std::static_pointer_cast<BooleanQueryNode>(QueryNodeImpl::cloneTree());

  // nothing to do here

  return clone;
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes