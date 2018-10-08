using namespace std;

#include "PrefixWildcardQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::standard::nodes
{
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;

PrefixWildcardQueryNode::PrefixWildcardQueryNode(shared_ptr<std::wstring> field,
                                                 shared_ptr<std::wstring> text,
                                                 int begin, int end)
    : WildcardQueryNode(field, text, begin, end)
{
}

PrefixWildcardQueryNode::PrefixWildcardQueryNode(shared_ptr<FieldQueryNode> fqn)
    : PrefixWildcardQueryNode(fqn->getField(), fqn->getText(), fqn->getBegin(),
                              fqn->getEnd())
{
}

wstring PrefixWildcardQueryNode::toString()
{
  return L"<prefixWildcard field='" + this->field + L"' term='" + this->text +
         L"'/>";
}

shared_ptr<PrefixWildcardQueryNode>
PrefixWildcardQueryNode::cloneTree() 
{
  shared_ptr<PrefixWildcardQueryNode> clone =
      std::static_pointer_cast<PrefixWildcardQueryNode>(
          WildcardQueryNode::cloneTree());

  // nothing to do here

  return clone;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::nodes