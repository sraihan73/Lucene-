using namespace std;

#include "TestQueryTreeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::core::builders
{
using junit::framework::Assert;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeImpl =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNodeImpl;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;
using UnescapedCharSequence = org::apache::lucene::queryparser::flexible::core::
    util::UnescapedCharSequence;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSetFieldBuilder() throws
// org.apache.lucene.queryparser.flexible.core.QueryNodeException
void TestQueryTreeBuilder::testSetFieldBuilder() 
{
  shared_ptr<QueryTreeBuilder> qtb = make_shared<QueryTreeBuilder>();
  qtb->setBuilder(L"field", make_shared<DummyBuilder>());
  any result = qtb->build(make_shared<FieldQueryNode>(
      make_shared<UnescapedCharSequence>(L"field"), L"foo", 0, 0));
  Assert::assertEquals(L"OK", result);

  // LUCENE-4890
  qtb = make_shared<QueryTreeBuilder>();
  qtb->setBuilder(DummyQueryNodeInterface::typeid, make_shared<DummyBuilder>());
  result = qtb->build(make_shared<DummyQueryNode>());
  Assert::assertEquals(L"OK", result);
}

shared_ptr<std::wstring> TestQueryTreeBuilder::DummyQueryNode::toQueryString(
    shared_ptr<EscapeQuerySyntax> escapeSyntaxParser)
{
  return L"DummyQueryNode";
}

any TestQueryTreeBuilder::DummyBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  return L"OK";
}
} // namespace org::apache::lucene::queryparser::flexible::core::builders