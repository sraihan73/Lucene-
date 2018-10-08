using namespace std;

#include "AnyQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

AnyQueryNode::AnyQueryNode(deque<std::shared_ptr<QueryNode>> &clauses,
                           shared_ptr<std::wstring> field,
                           int minimumMatchingElements)
    : AndQueryNode(clauses)
{
  this->field = field;
  this->minimumMatchingmElements = minimumMatchingElements;

  if (clauses.size() > 0) {

    for (auto clause : clauses) {

      if (std::dynamic_pointer_cast<FieldQueryNode>(clause) != nullptr) {

        if (std::dynamic_pointer_cast<QueryNodeImpl>(clause) != nullptr) {
          (std::static_pointer_cast<QueryNodeImpl>(clause))
              ->toQueryStringIgnoreFields = true;
        }

        if (std::dynamic_pointer_cast<FieldableNode>(clause) != nullptr) {
          (std::static_pointer_cast<FieldableNode>(clause))->setField(field);
        }
      }
    }
  }
}

int AnyQueryNode::getMinimumMatchingElements()
{
  return this->minimumMatchingmElements;
}

shared_ptr<std::wstring> AnyQueryNode::getField() { return this->field; }

wstring AnyQueryNode::getFieldAsString()
{
  if (this->field == nullptr) {
    return L"";
  } else {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return this->field->toString();
  }
}

void AnyQueryNode::setField(shared_ptr<std::wstring> field)
{
  this->field = field;
}

shared_ptr<QueryNode>
AnyQueryNode::cloneTree() 
{
  shared_ptr<AnyQueryNode> clone =
      std::static_pointer_cast<AnyQueryNode>(AndQueryNode::cloneTree());

  clone->field = this->field;
  clone->minimumMatchingmElements = this->minimumMatchingmElements;

  return clone;
}

wstring AnyQueryNode::toString()
{
  if (getChildren().empty() || getChildren().empty()) {
    return L"<any field='" + this->field + L"'  matchelements=" +
           to_wstring(this->minimumMatchingmElements) + L"/>";
  }
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"<any field='" + this->field + L"'  matchelements=" +
             to_wstring(this->minimumMatchingmElements) + L">");
  for (auto clause : getChildren()) {
    sb->append(L"\n");
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(clause->toString());
  }
  sb->append(L"\n</any>");
  return sb->toString();
}

shared_ptr<std::wstring>
AnyQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  wstring anySTR = L"ANY " + to_wstring(this->minimumMatchingmElements);

  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  if (getChildren().empty() || getChildren().empty()) {
    // no childs case
  } else {
    wstring filler = L"";
    for (auto clause : getChildren()) {
      sb->append(filler)->append(clause->toQueryString(escapeSyntaxParser));
      filler = L" ";
    }
  }

  if (isDefaultField(this->field)) {
    return L"( " + sb->toString() + L" ) " + anySTR;
  } else {
    return this->field + L":(( " + sb->toString() + L" ) " + anySTR + L")";
  }
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes