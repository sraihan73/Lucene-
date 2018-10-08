using namespace std;

#include "PointQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::standard::nodes
{
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using FieldValuePairQueryNode = org::apache::lucene::queryparser::flexible::
    core::nodes::FieldValuePairQueryNode;
using QueryNodeImpl =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNodeImpl;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;
using Type = org::apache::lucene::queryparser::flexible::core::parser::
    EscapeQuerySyntax::Type;
using PointsConfig =
    org::apache::lucene::queryparser::flexible::standard::config::PointsConfig;

PointQueryNode::PointQueryNode(shared_ptr<std::wstring> field,
                               shared_ptr<Number> value,
                               shared_ptr<NumberFormat> numberFormat)
    : org::apache::lucene::queryparser::flexible::core::nodes::QueryNodeImpl()
{

  setNumberFormat(numberFormat);
  setField(field);
  setValue(value);
}

shared_ptr<std::wstring> PointQueryNode::getField() { return this->field; }

void PointQueryNode::setField(shared_ptr<std::wstring> fieldName)
{
  this->field = fieldName;
}

shared_ptr<std::wstring>
PointQueryNode::getTermEscaped(shared_ptr<EscapeQuerySyntax> escaper)
{
  return escaper->escape(numberFormat->format(this->value), Locale::ROOT,
                         EscapeQuerySyntax::Type::NORMAL);
}

shared_ptr<std::wstring>
PointQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  if (isDefaultField(this->field)) {
    return getTermEscaped(escapeSyntaxParser);
  } else {
    return this->field + L":" + getTermEscaped(escapeSyntaxParser);
  }
}

void PointQueryNode::setNumberFormat(shared_ptr<NumberFormat> format)
{
  this->numberFormat = format;
}

shared_ptr<NumberFormat> PointQueryNode::getNumberFormat()
{
  return this->numberFormat;
}

shared_ptr<Number> PointQueryNode::getValue() { return value; }

void PointQueryNode::setValue(shared_ptr<Number> value) { this->value = value; }

wstring PointQueryNode::toString()
{
  return L"<numeric field='" + this->field + L"' number='" +
         numberFormat->format(value) + L"'/>";
}
} // namespace org::apache::lucene::queryparser::flexible::standard::nodes