using namespace std;

#include "BoostQueryNode.h"

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

BoostQueryNode::BoostQueryNode(shared_ptr<QueryNode> query, float value)
{
  if (query == nullptr) {
    throw make_shared<QueryNodeError>(make_shared<MessageImpl>(
        QueryParserMessages::NODE_ACTION_NOT_SUPPORTED, L"query", L"null"));
  }

  this->value = value;
  setLeaf(false);
  allocate();
  add(query);
}

shared_ptr<QueryNode> BoostQueryNode::getChild()
{
  deque<std::shared_ptr<QueryNode>> children = getChildren();

  if (children.empty() || children.empty()) {
    return nullptr;
  }

  return children[0];
}

float BoostQueryNode::getValue() { return this->value; }

shared_ptr<std::wstring> BoostQueryNode::getValueString()
{
  optional<float> f = static_cast<Float>(this->value);
  if (f == f.value()) {
    return L"" + f.value();
  } else {
    return L"" + f;
  }
}

wstring BoostQueryNode::toString()
{
  return L"<boost value='" + getValueString() + L"'>" +
         L"\n"
         // C++ TODO: There is no native C++ equivalent to 'toString':
         + getChild()->toString() + L"\n</boost>";
}

shared_ptr<std::wstring>
BoostQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  if (getChild() == nullptr) {
    return L"";
  }
  return getChild()->toQueryString(escapeSyntaxParser) + L"^" +
         getValueString();
}

shared_ptr<QueryNode>
BoostQueryNode::cloneTree() 
{
  shared_ptr<BoostQueryNode> clone =
      std::static_pointer_cast<BoostQueryNode>(QueryNodeImpl::cloneTree());

  clone->value = this->value;

  return clone;
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes