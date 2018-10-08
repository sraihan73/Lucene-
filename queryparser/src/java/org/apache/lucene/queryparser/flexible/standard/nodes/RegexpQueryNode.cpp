using namespace std;

#include "RegexpQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::standard::nodes
{
using FieldableNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldableNode;
using QueryNodeImpl =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNodeImpl;
using TextableQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::TextableQueryNode;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;
using RegexpQuery = org::apache::lucene::search::RegexpQuery;
using BytesRef = org::apache::lucene::util::BytesRef;

RegexpQueryNode::RegexpQueryNode(shared_ptr<std::wstring> field,
                                 shared_ptr<std::wstring> text, int begin,
                                 int end)
{
  this->field = field;
  this->text = text->substr(begin, end - begin);
}

shared_ptr<BytesRef> RegexpQueryNode::textToBytesRef()
{
  return make_shared<BytesRef>(text);
}

wstring RegexpQueryNode::toString()
{
  return L"<regexp field='" + this->field + L"' term='" + this->text + L"'/>";
}

shared_ptr<RegexpQueryNode>
RegexpQueryNode::cloneTree() 
{
  shared_ptr<RegexpQueryNode> clone =
      std::static_pointer_cast<RegexpQueryNode>(QueryNodeImpl::cloneTree());
  clone->field = this->field;
  clone->text = this->text;
  return clone;
}

shared_ptr<std::wstring> RegexpQueryNode::getText() { return text; }

void RegexpQueryNode::setText(shared_ptr<std::wstring> text)
{
  this->text = text;
}

shared_ptr<std::wstring> RegexpQueryNode::getField() { return field; }

wstring RegexpQueryNode::getFieldAsString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return field->toString();
}

void RegexpQueryNode::setField(shared_ptr<std::wstring> field)
{
  this->field = field;
}

shared_ptr<std::wstring>
RegexpQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  return isDefaultField(field) ? L"/" + text + L"/"
                               : field + L":/" + text + L"/";
}
} // namespace org::apache::lucene::queryparser::flexible::standard::nodes