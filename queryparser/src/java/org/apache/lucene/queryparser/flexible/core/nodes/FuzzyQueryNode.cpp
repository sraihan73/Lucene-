using namespace std;

#include "FuzzyQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

FuzzyQueryNode::FuzzyQueryNode(shared_ptr<std::wstring> field,
                               shared_ptr<std::wstring> term,
                               float minSimilarity, int begin, int end)
    : FieldQueryNode(field, term, begin, end)
{
  this->similarity = minSimilarity;
  setLeaf(true);
}

void FuzzyQueryNode::setPrefixLength(int prefixLength)
{
  this->prefixLength = prefixLength;
}

int FuzzyQueryNode::getPrefixLength() { return this->prefixLength; }

shared_ptr<std::wstring>
FuzzyQueryNode::toQueryString(shared_ptr<EscapeQuerySyntax> escaper)
{
  if (isDefaultField(this->field)) {
    return getTermEscaped(escaper) + L"~" + to_wstring(this->similarity);
  } else {
    return this->field + L":" + getTermEscaped(escaper) + L"~" +
           to_wstring(this->similarity);
  }
}

wstring FuzzyQueryNode::toString()
{
  return L"<fuzzy field='" + this->field + L"' similarity='" +
         to_wstring(this->similarity) + L"' term='" + this->text + L"'/>";
}

void FuzzyQueryNode::setSimilarity(float similarity)
{
  this->similarity = similarity;
}

shared_ptr<FuzzyQueryNode>
FuzzyQueryNode::cloneTree() 
{
  shared_ptr<FuzzyQueryNode> clone =
      std::static_pointer_cast<FuzzyQueryNode>(FieldQueryNode::cloneTree());

  clone->similarity = this->similarity;

  return clone;
}

float FuzzyQueryNode::getSimilarity() { return this->similarity; }
} // namespace org::apache::lucene::queryparser::flexible::core::nodes