using namespace std;

#include "AndQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

AndQueryNode::AndQueryNode(deque<std::shared_ptr<QueryNode>> &clauses)
    : BooleanQueryNode(clauses)
{
  if ((clauses.empty()) || (clauses.empty())) {
    throw invalid_argument(L"AND query must have at least one clause");
  }
}

wstring AndQueryNode::toString()
{
  if (getChildren().empty() || getChildren().empty()) {
    return L"<bool operation='and'/>";
  }
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"<bool operation='and'>");
  for (auto child : getChildren()) {
    sb->append(L"\n");
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(child->toString());
  }
  sb->append(L"\n</bool>");
  return sb->toString();
}

shared_ptr<std::wstring>
AndQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  if (getChildren().empty() || getChildren().empty()) {
    return L"";
  }

  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  wstring filler = L"";
  for (auto child : getChildren()) {
    sb->append(filler)->append(child->toQueryString(escapeSyntaxParser));
    filler = L" AND ";
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
} // namespace org::apache::lucene::queryparser::flexible::core::nodes