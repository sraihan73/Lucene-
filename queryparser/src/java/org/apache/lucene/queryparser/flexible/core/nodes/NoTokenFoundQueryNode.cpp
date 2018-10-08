using namespace std;

#include "NoTokenFoundQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

NoTokenFoundQueryNode::NoTokenFoundQueryNode() : DeletedQueryNode() {}

shared_ptr<std::wstring>
NoTokenFoundQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escaper)
{
  return L"[NTF]";
}

wstring NoTokenFoundQueryNode::toString() { return L"<notokenfound/>"; }

shared_ptr<QueryNode>
NoTokenFoundQueryNode::cloneTree() 
{
  shared_ptr<NoTokenFoundQueryNode> clone =
      std::static_pointer_cast<NoTokenFoundQueryNode>(
          DeletedQueryNode::cloneTree());

  // nothing to do here

  return clone;
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes