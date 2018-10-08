using namespace std;

#include "QuotedFieldQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

QuotedFieldQueryNode::QuotedFieldQueryNode(shared_ptr<std::wstring> field,
                                           shared_ptr<std::wstring> text,
                                           int begin, int end)
    : FieldQueryNode(field, text, begin, end)
{
}

shared_ptr<std::wstring>
QuotedFieldQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escaper)
{
  if (isDefaultField(this->field)) {
    return L"\"" + getTermEscapeQuoted(escaper) + L"\"";
  } else {
    return this->field + L":" + L"\"" + getTermEscapeQuoted(escaper) + L"\"";
  }
}

wstring QuotedFieldQueryNode::toString()
{
  return L"<quotedfield start='" + to_wstring(this->begin) + L"' end='" +
         to_wstring(this->end) + L"' field='" + this->field + L"' term='" +
         this->text + L"'/>";
}

shared_ptr<QuotedFieldQueryNode>
QuotedFieldQueryNode::cloneTree() 
{
  shared_ptr<QuotedFieldQueryNode> clone =
      std::static_pointer_cast<QuotedFieldQueryNode>(
          FieldQueryNode::cloneTree());
  // nothing to do here
  return clone;
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes