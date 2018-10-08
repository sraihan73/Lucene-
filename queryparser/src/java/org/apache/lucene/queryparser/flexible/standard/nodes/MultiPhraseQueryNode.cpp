using namespace std;

#include "MultiPhraseQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::standard::nodes
{
using FieldableNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldableNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeImpl =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNodeImpl;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;
using MultiPhraseQuery = org::apache::lucene::search::MultiPhraseQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;

MultiPhraseQueryNode::MultiPhraseQueryNode()
{
  setLeaf(false);
  allocate();
}

wstring MultiPhraseQueryNode::toString()
{
  if (getChildren().empty() || getChildren().empty()) {
    return L"<multiPhrase/>";
  }
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"<multiPhrase>");
  for (auto child : getChildren()) {
    sb->append(L"\n");
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(child->toString());
  }
  sb->append(L"\n</multiPhrase>");
  return sb->toString();
}

shared_ptr<std::wstring> MultiPhraseQueryNode::toQueryString(
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

  return L"[MTP[" + sb->toString() + L"]]";
}

shared_ptr<QueryNode>
MultiPhraseQueryNode::cloneTree() 
{
  shared_ptr<MultiPhraseQueryNode> clone =
      std::static_pointer_cast<MultiPhraseQueryNode>(
          QueryNodeImpl::cloneTree());

  // nothing to do

  return clone;
}

shared_ptr<std::wstring> MultiPhraseQueryNode::getField()
{
  deque<std::shared_ptr<QueryNode>> children = getChildren();

  if (children.empty() || children.empty()) {
    return nullptr;

  } else {
    return (std::static_pointer_cast<FieldableNode>(children[0]))->getField();
  }
}

void MultiPhraseQueryNode::setField(shared_ptr<std::wstring> fieldName)
{
  deque<std::shared_ptr<QueryNode>> children = getChildren();

  if (children.size() > 0) {

    for (auto child : children) {

      if (std::dynamic_pointer_cast<FieldableNode>(child) != nullptr) {
        (std::static_pointer_cast<FieldableNode>(child))->setField(fieldName);
      }
    }
  }
}
} // namespace org::apache::lucene::queryparser::flexible::standard::nodes