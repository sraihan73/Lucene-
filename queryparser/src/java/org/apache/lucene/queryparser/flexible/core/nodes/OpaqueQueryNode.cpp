using namespace std;

#include "OpaqueQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

OpaqueQueryNode::OpaqueQueryNode(shared_ptr<std::wstring> schema,
                                 shared_ptr<std::wstring> value)
{
  this->setLeaf(true);

  this->schema = schema;
  this->value = value;
}

wstring OpaqueQueryNode::toString()
{
  return L"<opaque schema='" + this->schema + L"' value='" + this->value +
         L"'/>";
}

shared_ptr<std::wstring>
OpaqueQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  return L"@" + this->schema + L":'" + this->value + L"'";
}

shared_ptr<QueryNode>
OpaqueQueryNode::cloneTree() 
{
  shared_ptr<OpaqueQueryNode> clone =
      std::static_pointer_cast<OpaqueQueryNode>(QueryNodeImpl::cloneTree());

  clone->schema = this->schema;
  clone->value = this->value;

  return clone;
}

shared_ptr<std::wstring> OpaqueQueryNode::getSchema() { return this->schema; }

shared_ptr<std::wstring> OpaqueQueryNode::getValue() { return this->value; }
} // namespace org::apache::lucene::queryparser::flexible::core::nodes