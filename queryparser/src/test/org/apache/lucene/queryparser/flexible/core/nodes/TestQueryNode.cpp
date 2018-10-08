using namespace std;

#include "TestQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestQueryNode::testAddChildren() 
{
  shared_ptr<QueryNode> nodeA = make_shared<FieldQueryNode>(L"foo", L"A", 0, 1);
  shared_ptr<QueryNode> nodeB = make_shared<FieldQueryNode>(L"foo", L"B", 1, 2);
  shared_ptr<BooleanQueryNode> bq =
      make_shared<BooleanQueryNode>(Arrays::asList(nodeA));
  bq->add(Arrays::asList(nodeB));
  assertEquals(2, bq->getChildren().size());
}

void TestQueryNode::testTags() 
{
  shared_ptr<QueryNode> node = make_shared<FieldQueryNode>(L"foo", L"A", 0, 1);

  node->setTag(L"TaG", any());
  assertTrue(node->getTagMap().size() > 0);
  assertTrue(node->containsTag(L"tAg"));
  assertTrue(node->getTag(L"tAg") != nullptr);
}

void TestQueryNode::testRemoveFromParent() 
{
  shared_ptr<BooleanQueryNode> booleanNode = make_shared<BooleanQueryNode>(
      Collections::emptyList<std::shared_ptr<QueryNode>>());
  shared_ptr<FieldQueryNode> fieldNode =
      make_shared<FieldQueryNode>(L"foo", L"A", 0, 1);
  assertNull(fieldNode->getParent());

  booleanNode->add(fieldNode);
  assertNotNull(fieldNode->getParent());

  fieldNode->removeFromParent();
  assertNull(fieldNode->getParent());
  /* LUCENE-5805 - QueryNodeImpl.removeFromParent does a lot of work without any
   * effect */
  assertFalse(find(booleanNode->getChildren().begin(),
                   booleanNode->getChildren().end(),
                   fieldNode) != booleanNode->getChildren().end());

  booleanNode->add(fieldNode);
  assertNotNull(fieldNode->getParent());

  booleanNode->set(Collections::emptyList<std::shared_ptr<QueryNode>>());
  assertNull(fieldNode->getParent());
}

void TestQueryNode::testRemoveChildren() 
{
  shared_ptr<BooleanQueryNode> booleanNode = make_shared<BooleanQueryNode>(
      Collections::emptyList<std::shared_ptr<QueryNode>>());
  shared_ptr<FieldQueryNode> fieldNode =
      make_shared<FieldQueryNode>(L"foo", L"A", 0, 1);

  booleanNode->add(fieldNode);
  assertTrue(booleanNode->getChildren().size() == 1);

  booleanNode->removeChildren(fieldNode);
  assertTrue(booleanNode->getChildren().empty());
  assertNull(fieldNode->getParent());
}
} // namespace org::apache::lucene::queryparser::flexible::core::nodes