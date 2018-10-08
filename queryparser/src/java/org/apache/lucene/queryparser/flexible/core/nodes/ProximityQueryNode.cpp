using namespace std;

#include "ProximityQueryNode.h"

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

deque<Type> Type::valueList;

Type::StaticConstructor::StaticConstructor() {}

Type::StaticConstructor Type::staticConstructor;
int Type::nextOrdinal = 0;
Type::Type(const wstring &name, InnerEnum innerEnum)
    : nameValue(name), ordinalValue(nextOrdinal++), innerEnumValue(innerEnum)
{
}

bool Type::operator==(const Type &other)
{
  return this->ordinalValue == other.ordinalValue;
}

bool Type::operator!=(const Type &other)
{
  return this->ordinalValue != other.ordinalValue;
}

deque<Type> Type::values() { return valueList; }

int Type::ordinal() { return ordinalValue; }

wstring Type::toString() { return nameValue; }

Type Type::valueOf(const wstring &name)
{
  for (auto enumInstance : Type::valueList) {
    if (enumInstance.nameValue == name) {
      return enumInstance;
    }
  }
}

ProximityQueryNode::ProximityType::ProximityType(Type type)
    : ProximityType(type, 0)
{
}

ProximityQueryNode::ProximityType::ProximityType(Type type, int distance)
{
  this->pType = type;
  this->pDistance = distance;
}

ProximityQueryNode::ProximityQueryNode(
    deque<std::shared_ptr<QueryNode>> &clauses, shared_ptr<std::wstring> field,
    Type type, int distance, bool inorder)
    : BooleanQueryNode(clauses)
{
  setLeaf(false);
  this->proximityType = type;
  this->inorder = inorder;
  this->field = field;
  if (type == Type::NUMBER) {
    if (distance <= 0) {
      throw make_shared<QueryNodeError>(make_shared<MessageImpl>(
          QueryParserMessages::PARAMETER_VALUE_NOT_SUPPORTED, L"distance",
          distance));

    } else {
      this->distance = distance;
    }
  }
  clearFields(clauses, field);
}

ProximityQueryNode::ProximityQueryNode(
    deque<std::shared_ptr<QueryNode>> &clauses, shared_ptr<std::wstring> field,
    Type type, bool inorder)
    : ProximityQueryNode(clauses, field, type, -1, inorder)
{
}

void ProximityQueryNode::clearFields(deque<std::shared_ptr<QueryNode>> &nodes,
                                     shared_ptr<std::wstring> field)
{
  if (nodes.empty() || nodes.empty()) {
    return;
  }

  for (auto clause : nodes) {

    if (std::dynamic_pointer_cast<FieldQueryNode>(clause) != nullptr) {
      (std::static_pointer_cast<FieldQueryNode>(clause))
          ->toQueryStringIgnoreFields = true;
      (std::static_pointer_cast<FieldQueryNode>(clause))->setField(field);
    }
  }
}

ProximityQueryNode::Type ProximityQueryNode::getProximityType()
{
  return this->proximityType;
}

wstring ProximityQueryNode::toString()
{
  wstring distanceSTR =
      ((this->distance == -1)
           ? (L"")
           : (L" distance='" + to_wstring(this->distance)) + L"'");

  if (getChildren().empty() || getChildren().empty()) {
    return L"<proximity field='" + this->field + L"' inorder='" +
           StringHelper::toString(this->inorder) + L"' type='" +
           this->proximityType.toString() + L"'" + distanceSTR + L"/>";
  }
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"<proximity field='" + this->field + L"' inorder='" +
             StringHelper::toString(this->inorder) + L"' type='" +
             this->proximityType.toString() + L"'" + distanceSTR + L">");
  for (auto child : getChildren()) {
    sb->append(L"\n");
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(child->toString());
  }
  sb->append(L"\n</proximity>");
  return sb->toString();
}

shared_ptr<std::wstring> ProximityQueryNode::toQueryString(
    shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  wstring withinSTR =
      this->proximityType.toQueryString() +
      ((this->distance == -1) ? (L"") : (L" " + to_wstring(this->distance))) +
      ((this->inorder) ? (L" INORDER") : (L""));

  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  if (getChildren().empty() || getChildren().empty()) {
    // no children case
  } else {
    wstring filler = L"";
    for (auto child : getChildren()) {
      sb->append(filler)->append(child->toQueryString(escapeSyntaxParser));
      filler = L" ";
    }
  }

  if (isDefaultField(this->field)) {
    return L"( " + sb->toString() + L" ) " + withinSTR;
  } else {
    return this->field + L":(( " + sb->toString() + L" ) " + withinSTR + L")";
  }
}

shared_ptr<QueryNode>
ProximityQueryNode::cloneTree() 
{
  shared_ptr<ProximityQueryNode> clone =
      std::static_pointer_cast<ProximityQueryNode>(
          BooleanQueryNode::cloneTree());

  clone->proximityType = this->proximityType;
  clone->distance = this->distance;
  clone->field = this->field;

  return clone;
}

int ProximityQueryNode::getDistance() { return this->distance; }

shared_ptr<std::wstring> ProximityQueryNode::getField() { return this->field; }

wstring ProximityQueryNode::getFieldAsString()
{
  if (this->field == nullptr) {
    return L"";
  } else {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return this->field->toString();
  }
}

void ProximityQueryNode::setField(shared_ptr<std::wstring> field)
{
  this->field = field;
}

bool ProximityQueryNode::isInOrder() { return this->inorder; }
} // namespace org::apache::lucene::queryparser::flexible::core::nodes