using namespace std;

#include "PhraseSlopQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using QueryNodeError =
    org::apache::lucene::queryparser::flexible::core::QueryNodeError;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

PhraseSlopQueryNode::PhraseSlopQueryNode(shared_ptr<QueryNode> query, int value)
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

shared_ptr<QueryNode> PhraseSlopQueryNode::getChild()
{
  return getChildren()[0];
}

int PhraseSlopQueryNode::getValue() { return this->value; }

shared_ptr<std::wstring> PhraseSlopQueryNode::getValueString()
{
  optional<float> f = static_cast<Float>(this->value);
  if (f == f.value()) {
    return L"" + f.value();
  } else {
    return L"" + f;
  }
}

wstring PhraseSlopQueryNode::toString()
{
  return L"<phraseslop value='" + getValueString() + L"'>" +
         L"\n"
         // C++ TODO: There is no native C++ equivalent to 'toString':
         + getChild()->toString() + L"\n</phraseslop>";
}

shared_ptr<std::wstring> PhraseSlopQueryNode::toQueryString(
    shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  if (getChild() == nullptr) {
    return L"";
  }
  return getChild()->toQueryString(escapeSyntaxParser) + L"~" +
         getValueString();
}

shared_ptr<QueryNode>
PhraseSlopQueryNode::cloneTree() 
{
  shared_ptr<PhraseSlopQueryNode> clone =
      std::static_pointer_cast<PhraseSlopQueryNode>(QueryNodeImpl::cloneTree());

  clone->value = this->value;

  return clone;
}

shared_ptr<std::wstring> PhraseSlopQueryNode::getField()
{
  shared_ptr<QueryNode> child = getChild();

  if (std::dynamic_pointer_cast<FieldableNode>(child) != nullptr) {
    return (std::static_pointer_cast<FieldableNode>(child))->getField();
  }

  return nullptr;
}

void PhraseSlopQueryNode::setField(shared_ptr<std::wstring> fieldName)
{
  shared_ptr<QueryNode> child = getChild();

  if (std::dynamic_pointer_cast<FieldableNode>(child) != nullptr) {
    (std::static_pointer_cast<FieldableNode>(child))->setField(fieldName);
  }
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes