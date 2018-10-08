using namespace std;

#include "MatchAllDocsQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

MatchAllDocsQueryNode::MatchAllDocsQueryNode()
{
  // empty constructor
}

wstring MatchAllDocsQueryNode::toString()
{
  return L"<matchAllDocs field='*' term='*'/>";
}

shared_ptr<std::wstring> MatchAllDocsQueryNode::toQueryString(
    shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  return L"*:*";
}

shared_ptr<MatchAllDocsQueryNode>
MatchAllDocsQueryNode::cloneTree() 
{
  shared_ptr<MatchAllDocsQueryNode> clone =
      std::static_pointer_cast<MatchAllDocsQueryNode>(
          QueryNodeImpl::cloneTree());

  // nothing to clone

  return clone;
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes