using namespace std;

#include "SlopQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;
using QueryNodeError =
    org::apache::lucene::queryparser::flexible::core::QueryNodeError;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;

SlopQueryNode::SlopQueryNode(shared_ptr<QueryNode> query, int value)
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

shared_ptr<QueryNode> SlopQueryNode::getChild() { return getChildren()[0]; }

int SlopQueryNode::getValue() { return this->value; }

shared_ptr<std::wstring> SlopQueryNode::getValueString()
{
  optional<float> f = static_cast<Float>(this->value);
  if (f == f.value()) {
    return L"" + f.value();
  } else {
    return L"" + f;
  }
}

wstring SlopQueryNode::toString()
{
  return L"<slop value='" + getValueString() + L"'>" +
         L"\n"
         // C++ TODO: There is no native C++ equivalent to 'toString':
         + getChild()->toString() + L"\n</slop>";
}

shared_ptr<std::wstring>
SlopQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  if (getChild() == nullptr) {
    return L"";
  }
  return getChild()->toQueryString(escapeSyntaxParser) + L"~" +
         getValueString();
}

shared_ptr<QueryNode>
SlopQueryNode::cloneTree() 
{
  shared_ptr<SlopQueryNode> clone =
      std::static_pointer_cast<SlopQueryNode>(QueryNodeImpl::cloneTree());

  clone->value = this->value;

  return clone;
}

shared_ptr<std::wstring> SlopQueryNode::getField()
{
  shared_ptr<QueryNode> child = getChild();

  if (std::dynamic_pointer_cast<FieldableNode>(child) != nullptr) {
    return (std::static_pointer_cast<FieldableNode>(child))->getField();
  }

  return nullptr;
}

void SlopQueryNode::setField(shared_ptr<std::wstring> fieldName)
{
  shared_ptr<QueryNode> child = getChild();

  if (std::dynamic_pointer_cast<FieldableNode>(child) != nullptr) {
    (std::static_pointer_cast<FieldableNode>(child))->setField(fieldName);
  }
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes