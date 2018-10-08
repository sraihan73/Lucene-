using namespace std;

#include "MatchNoDocsQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{

MatchNoDocsQueryNode::MatchNoDocsQueryNode()
{
  // empty constructor
}

wstring MatchNoDocsQueryNode::toString() { return L"<matchNoDocsQueryNode/>"; }
} // namespace org::apache::lucene::queryparser::flexible::core::nodes