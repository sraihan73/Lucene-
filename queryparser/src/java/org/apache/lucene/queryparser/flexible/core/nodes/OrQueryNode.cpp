using namespace std;

#include "OrQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

OrQueryNode::OrQueryNode(deque<std::shared_ptr<QueryNode>> &clauses)
    : BooleanQueryNode(clauses)
{
  if ((clauses.empty()) || (clauses.empty())) {
    throw invalid_argument(L"OR query must have at least one clause");
  }
}

wstring OrQueryNode::toString()
{
  if (getChildren().empty() || getChildren().empty()) {
    return L"<bool operation='or'/>";
  }
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"<bool operation='or'>");
  for (auto child : getChildren()) {
    sb->append(L"\n");
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(child->toString());
  }
  sb->append(L"\n</bool>");
  return sb->toString();
}

shared_ptr<std::wstring>
OrQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  if (getChildren().empty() || getChildren().empty()) {
    return L"";
  }

  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  wstring filler = L"";
  for (shared_ptr<Iterator<std::shared_ptr<QueryNode>>> it =
           getChildren().begin();
       it->hasNext();) {
    sb->append(filler)->append(it->next().toQueryString(escapeSyntaxParser));
    filler = L" OR ";
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