using namespace std;

#include "WildcardQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::standard::nodes
{
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

WildcardQueryNode::WildcardQueryNode(shared_ptr<std::wstring> field,
                                     shared_ptr<std::wstring> text, int begin,
                                     int end)
    : org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode(
          field, text, begin, end)
{
}

WildcardQueryNode::WildcardQueryNode(shared_ptr<FieldQueryNode> fqn)
    : WildcardQueryNode(fqn->getField(), fqn->getText(), fqn->getBegin(),
                        fqn->getEnd())
{
}

shared_ptr<std::wstring>
WildcardQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escaper)
{
  if (isDefaultField(this->field)) {
    return this->text;
  } else {
    return this->field + L":" + this->text;
  }
}

wstring WildcardQueryNode::toString()
{
  return L"<wildcard field='" + this->field + L"' term='" + this->text + L"'/>";
}

shared_ptr<WildcardQueryNode>
WildcardQueryNode::cloneTree() 
{
  shared_ptr<WildcardQueryNode> clone =
      std::static_pointer_cast<WildcardQueryNode>(FieldQueryNode::cloneTree());

  // nothing to do here

  return clone;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::nodes