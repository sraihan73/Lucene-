using namespace std;

#include "GroupQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using QueryNodeError =
    org::apache::lucene::queryparser::flexible::core::QueryNodeError;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

GroupQueryNode::GroupQueryNode(shared_ptr<QueryNode> query)
{
  if (query == nullptr) {
    throw make_shared<QueryNodeError>(make_shared<MessageImpl>(
        QueryParserMessages::PARAMETER_VALUE_NOT_SUPPORTED, L"query", L"null"));
  }

  allocate();
  setLeaf(false);
  add(query);
}

shared_ptr<QueryNode> GroupQueryNode::getChild() { return getChildren()[0]; }

wstring GroupQueryNode::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return wstring(L"<group>") + L"\n" + getChild()->toString() + L"\n</group>";
}

shared_ptr<std::wstring>
GroupQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  if (getChild() == nullptr) {
    return L"";
  }

  return L"( " + getChild()->toQueryString(escapeSyntaxParser) + L" )";
}

shared_ptr<QueryNode>
GroupQueryNode::cloneTree() 
{
  shared_ptr<GroupQueryNode> clone =
      std::static_pointer_cast<GroupQueryNode>(QueryNodeImpl::cloneTree());

  return clone;
}

void GroupQueryNode::setChild(shared_ptr<QueryNode> child)
{
  deque<std::shared_ptr<QueryNode>> deque =
      deque<std::shared_ptr<QueryNode>>();
  deque.push_back(child);
  this->set(deque);
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes