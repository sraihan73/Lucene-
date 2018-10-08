using namespace std;

#include "TokenizedPhraseQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

TokenizedPhraseQueryNode::TokenizedPhraseQueryNode()
{
  setLeaf(false);
  allocate();
}

wstring TokenizedPhraseQueryNode::toString()
{
  if (getChildren().empty() || getChildren().empty()) {
    return L"<tokenizedphrase/>";
  }
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"<tokenizedtphrase>");
  for (auto child : getChildren()) {
    sb->append(L"\n");
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(child->toString());
  }
  sb->append(L"\n</tokenizedphrase>");
  return sb->toString();
}

shared_ptr<std::wstring> TokenizedPhraseQueryNode::toQueryString(
    shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  if (getChildren().empty() || getChildren().empty()) {
    return L"";
  }

  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  wstring filler = L"";
  for (auto child : getChildren()) {
    sb->append(filler)->append(child->toQueryString(escapeSyntaxParser));
    filler = L",";
  }

  return L"[TP[" + sb->toString() + L"]]";
}

shared_ptr<QueryNode>
TokenizedPhraseQueryNode::cloneTree() 
{
  shared_ptr<TokenizedPhraseQueryNode> clone =
      std::static_pointer_cast<TokenizedPhraseQueryNode>(
          QueryNodeImpl::cloneTree());

  // nothing to do

  return clone;
}

shared_ptr<std::wstring> TokenizedPhraseQueryNode::getField()
{
  deque<std::shared_ptr<QueryNode>> children = getChildren();

  if (children.empty() || children.empty()) {
    return nullptr;

  } else {
    return (std::static_pointer_cast<FieldableNode>(children[0]))->getField();
  }
}

void TokenizedPhraseQueryNode::setField(shared_ptr<std::wstring> fieldName)
{
  deque<std::shared_ptr<QueryNode>> children = getChildren();

  if (children.size() > 0) {

    for (auto child : getChildren()) {

      if (std::dynamic_pointer_cast<FieldableNode>(child) != nullptr) {
        (std::static_pointer_cast<FieldableNode>(child))->setField(fieldName);
      }
    }
  }
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes