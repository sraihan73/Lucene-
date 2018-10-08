using namespace std;

#include "FieldQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

FieldQueryNode::FieldQueryNode(shared_ptr<std::wstring> field,
                               shared_ptr<std::wstring> text, int begin,
                               int end)
{
  this->field = field;
  this->text = text;
  this->begin = begin;
  this->end = end;
  this->setLeaf(true);
}

shared_ptr<std::wstring>
FieldQueryNode::getTermEscaped(shared_ptr<EscapeQuerySyntax> escaper)
{
  return escaper->escape(this->text, Locale::getDefault(),
                         EscapeQuerySyntax::Type::NORMAL);
}

shared_ptr<std::wstring>
FieldQueryNode::getTermEscapeQuoted(shared_ptr<EscapeQuerySyntax> escaper)
{
  return escaper->escape(this->text, Locale::getDefault(),
                         EscapeQuerySyntax::Type::STRING);
}

shared_ptr<std::wstring>
FieldQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escaper)
{
  if (isDefaultField(this->field)) {
    return getTermEscaped(escaper);
  } else {
    return this->field + L":" + getTermEscaped(escaper);
  }
}

wstring FieldQueryNode::toString()
{
  return L"<field start='" + to_wstring(this->begin) + L"' end='" +
         to_wstring(this->end) + L"' field='" + this->field + L"' text='" +
         this->text + L"'/>";
}

wstring FieldQueryNode::getTextAsString()
{
  if (this->text == nullptr) {
    return L"";
  } else {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return this->text->toString();
  }
}

wstring FieldQueryNode::getFieldAsString()
{
  if (this->field == nullptr) {
    return L"";
  } else {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return this->field->toString();
  }
}

int FieldQueryNode::getBegin() { return this->begin; }

void FieldQueryNode::setBegin(int begin) { this->begin = begin; }

int FieldQueryNode::getEnd() { return this->end; }

void FieldQueryNode::setEnd(int end) { this->end = end; }

shared_ptr<std::wstring> FieldQueryNode::getField() { return this->field; }

void FieldQueryNode::setField(shared_ptr<std::wstring> field)
{
  this->field = field;
}

int FieldQueryNode::getPositionIncrement() { return this->positionIncrement; }

void FieldQueryNode::setPositionIncrement(int pi)
{
  this->positionIncrement = pi;
}

shared_ptr<std::wstring> FieldQueryNode::getText() { return this->text; }

void FieldQueryNode::setText(shared_ptr<std::wstring> text)
{
  this->text = text;
}

shared_ptr<FieldQueryNode>
FieldQueryNode::cloneTree() 
{
  shared_ptr<FieldQueryNode> fqn =
      std::static_pointer_cast<FieldQueryNode>(QueryNodeImpl::cloneTree());
  fqn->begin = this->begin;
  fqn->end = this->end;
  fqn->field = this->field;
  fqn->text = this->text;
  fqn->positionIncrement = this->positionIncrement;
  fqn->toQueryStringIgnoreFields = this->toQueryStringIgnoreFields;

  return fqn;
}

shared_ptr<std::wstring> FieldQueryNode::getValue() { return getText(); }

void FieldQueryNode::setValue(shared_ptr<std::wstring> value)
{
  setText(value);
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes