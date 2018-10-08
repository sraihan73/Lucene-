using namespace std;

#include "PathQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;
using Type = org::apache::lucene::queryparser::flexible::core::parser::
    EscapeQuerySyntax::Type;

// C++ TODO: No base class can be determined:
PathQueryNode::QueryText::QueryText(shared_ptr<std::wstring> value, int begin,
                                    int end)
{
  this->value = value;
  this->begin = begin;
  this->end = end;
}

shared_ptr<QueryText>
PathQueryNode::QueryText::clone() 
{
  shared_ptr<QueryText> clone =
      std::static_pointer_cast<QueryText>(outerInstance->super.clone());
  clone->value = this->value;
  clone->begin = this->begin;
  clone->end = this->end;
  return clone;
}

shared_ptr<std::wstring> PathQueryNode::QueryText::getValue() { return value; }

int PathQueryNode::QueryText::getBegin() { return begin; }

int PathQueryNode::QueryText::getEnd() { return end; }

wstring PathQueryNode::QueryText::toString()
{
  return value + L", " + to_wstring(begin) + L", " + to_wstring(end);
}

PathQueryNode::PathQueryNode(deque<std::shared_ptr<QueryText>> &pathElements)
{
  this->values = pathElements;
  if (pathElements.size() <= 1) {
    // this should not happen
    throw runtime_error(
        L"PathQuerynode requires more 2 or more path elements.");
  }
}

deque<std::shared_ptr<QueryText>> PathQueryNode::getPathElements()
{
  return values;
}

void PathQueryNode::setPathElements(
    deque<std::shared_ptr<QueryText>> &elements)
{
  this->values = elements;
}

shared_ptr<QueryText> PathQueryNode::getPathElement(int index)
{
  return values[index];
}

shared_ptr<std::wstring> PathQueryNode::getFirstPathElement()
{
  return values[0]->value;
}

deque<std::shared_ptr<QueryText>>
PathQueryNode::getPathElements(int startIndex)
{
  deque<std::shared_ptr<PathQueryNode::QueryText>> rValues =
      deque<std::shared_ptr<PathQueryNode::QueryText>>();
  for (int i = startIndex; i < this->values.size(); i++) {
    try {
      rValues.push_back(this->values[i]->clone());
    } catch (const CloneNotSupportedException &e) {
      // this will not happen
    }
  }
  return rValues;
}

shared_ptr<std::wstring> PathQueryNode::getPathString()
{
  shared_ptr<StringBuilder> path = make_shared<StringBuilder>();

  for (auto pathelement : values) {
    path->append(L"/")->append(pathelement->value);
  }
  return path->toString();
}

shared_ptr<std::wstring>
PathQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escaper)
{
  shared_ptr<StringBuilder> path = make_shared<StringBuilder>();
  path->append(L"/")->append(getFirstPathElement());

  for (auto pathelement : getPathElements(1)) {
    shared_ptr<std::wstring> value =
        escaper->escape(pathelement->value, Locale::getDefault(),
                        EscapeQuerySyntax::Type::STRING);
    path->append(L"/\"")->append(value)->append(L"\"");
  }
  return path->toString();
}

wstring PathQueryNode::toString()
{
  shared_ptr<QueryText> text = this->values[0];

  return L"<path start='" + to_wstring(text->begin) + L"' end='" +
         to_wstring(text->end) + L"' path='" + getPathString() + L"'/>";
}

shared_ptr<QueryNode>
PathQueryNode::cloneTree() 
{
  shared_ptr<PathQueryNode> clone =
      std::static_pointer_cast<PathQueryNode>(QueryNodeImpl::cloneTree());

  // copy children
  if (this->values.size() > 0) {
    deque<std::shared_ptr<QueryText>> localValues =
        deque<std::shared_ptr<QueryText>>();
    for (auto value : this->values) {
      localValues.push_back(value->clone());
    }
    clone->values = localValues;
  }

  return clone;
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes