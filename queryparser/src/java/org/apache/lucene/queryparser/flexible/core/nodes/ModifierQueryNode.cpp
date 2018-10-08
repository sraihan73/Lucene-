using namespace std;

#include "ModifierQueryNode.h"

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

Modifier Modifier::MOD_NONE(L"MOD_NONE", InnerEnum::MOD_NONE);
Modifier Modifier::MOD_NOT(L"MOD_NOT", InnerEnum::MOD_NOT);
Modifier Modifier::MOD_REQ(L"MOD_REQ", InnerEnum::MOD_REQ);

deque<Modifier> Modifier::valueList;

Modifier::StaticConstructor::StaticConstructor()
{
  valueList.push_back(MOD_NONE);
  valueList.push_back(MOD_NOT);
  valueList.push_back(MOD_REQ);
}

Modifier::StaticConstructor Modifier::staticConstructor;
int Modifier::nextOrdinal = 0;
Modifier::Modifier(const wstring &name, InnerEnum innerEnum)
    : nameValue(name), ordinalValue(nextOrdinal++), innerEnumValue(innerEnum)
{
}

wstring ModifierQueryNode::Modifier::ToString()
{
  switch (shared_from_this()) {
  case MOD_NONE:
    return L"MOD_NONE";
  case MOD_NOT:
    return L"MOD_NOT";
  case MOD_REQ:
    return L"MOD_REQ";
  }
  // this code is never executed
  return L"MOD_DEFAULT";
}

wstring ModifierQueryNode::Modifier::toDigitString()
{
  switch (shared_from_this()) {
  case MOD_NONE:
    return L"";
  case MOD_NOT:
    return L"-";
  case MOD_REQ:
    return L"+";
  }
  // this code is never executed
  return L"";
}

wstring ModifierQueryNode::Modifier::toLargeString()
{
  switch (shared_from_this()) {
  case MOD_NONE:
    return L"";
  case MOD_NOT:
    return L"NOT ";
  case MOD_REQ:
    return L"+";
  }
  // this code is never executed
  return L"";
}

bool Modifier::operator==(const Modifier &other)
{
  return this->ordinalValue == other.ordinalValue;
}

bool Modifier::operator!=(const Modifier &other)
{
  return this->ordinalValue != other.ordinalValue;
}

deque<Modifier> Modifier::values() { return valueList; }

int Modifier::ordinal() { return ordinalValue; }

Modifier Modifier::valueOf(const wstring &name)
{
  for (auto enumInstance : Modifier::valueList) {
    if (enumInstance.nameValue == name) {
      return enumInstance;
    }
  }
}

ModifierQueryNode::ModifierQueryNode(shared_ptr<QueryNode> query, Modifier mod)
{
  if (query == nullptr) {
    throw make_shared<QueryNodeError>(make_shared<MessageImpl>(
        QueryParserMessages::PARAMETER_VALUE_NOT_SUPPORTED, L"query", L"null"));
  }

  allocate();
  setLeaf(false);
  add(query);
  this->modifier = mod;
}

shared_ptr<QueryNode> ModifierQueryNode::getChild() { return getChildren()[0]; }

ModifierQueryNode::Modifier ModifierQueryNode::getModifier()
{
  return this->modifier;
}

wstring ModifierQueryNode::toString()
{
  return L"<modifier operation='" + this->modifier.toString() + L"'>" +
         L"\n"
         // C++ TODO: There is no native C++ equivalent to 'toString':
         + getChild()->toString() + L"\n</modifier>";
}

shared_ptr<std::wstring> ModifierQueryNode::toQueryString(
    shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  if (getChild() == nullptr) {
    return L"";
  }

  wstring leftParenthensis = L"";
  wstring rightParenthensis = L"";

  if (getChild() != nullptr &&
      std::dynamic_pointer_cast<ModifierQueryNode>(getChild()) != nullptr) {
    leftParenthensis = L"(";
    rightParenthensis = L")";
  }

  if (std::dynamic_pointer_cast<BooleanQueryNode>(getChild()) != nullptr) {
    return this->modifier.toLargeString() + leftParenthensis +
           getChild()->toQueryString(escapeSyntaxParser) + rightParenthensis;
  } else {
    return this->modifier.toDigitString() + leftParenthensis +
           getChild()->toQueryString(escapeSyntaxParser) + rightParenthensis;
  }
}

shared_ptr<QueryNode>
ModifierQueryNode::cloneTree() 
{
  shared_ptr<ModifierQueryNode> clone =
      std::static_pointer_cast<ModifierQueryNode>(QueryNodeImpl::cloneTree());

  clone->modifier = this->modifier;

  return clone;
}

void ModifierQueryNode::setChild(shared_ptr<QueryNode> child)
{
  deque<std::shared_ptr<QueryNode>> deque =
      deque<std::shared_ptr<QueryNode>>();
  deque.push_back(child);
  this->set(deque);
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes