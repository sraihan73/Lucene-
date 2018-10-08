using namespace std;

#include "QueryNodeOperation.h"

namespace org::apache::lucene::queryparser::flexible::core::util
{
using QueryNodeError =
    org::apache::lucene::queryparser::flexible::core::QueryNodeError;
using AndQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::AndQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;

QueryNodeOperation::QueryNodeOperation()
{
  // Exists only to defeat instantiation.
}

shared_ptr<QueryNode> QueryNodeOperation::logicalAnd(shared_ptr<QueryNode> q1,
                                                     shared_ptr<QueryNode> q2)
{
  if (q1 == nullptr) {
    return q2;
  }
  if (q2 == nullptr) {
    return q1;
  }

  ANDOperation op = nullptr;
  if (std::dynamic_pointer_cast<AndQueryNode>(q1) != nullptr &&
      std::dynamic_pointer_cast<AndQueryNode>(q2) != nullptr) {
    op = ANDOperation::BOTH;
  } else if (std::dynamic_pointer_cast<AndQueryNode>(q1) != nullptr) {
    op = ANDOperation::Q1;
  } else if (std::dynamic_pointer_cast<AndQueryNode>(q2) != nullptr) {
    op = ANDOperation::Q2;
  } else {
    op = ANDOperation::NONE;
  }

  try {
    shared_ptr<QueryNode> result = nullptr;
    switch (op) {
    case org::apache::lucene::queryparser::flexible::core::util::
        QueryNodeOperation::ANDOperation::NONE: {
      deque<std::shared_ptr<QueryNode>> children =
          deque<std::shared_ptr<QueryNode>>();
      children.push_back(q1->cloneTree());
      children.push_back(q2->cloneTree());
      result = make_shared<AndQueryNode>(children);
      return result;
    }
    case org::apache::lucene::queryparser::flexible::core::util::
        QueryNodeOperation::ANDOperation::Q1:
      result = q1->cloneTree();
      result->add(q2->cloneTree());
      return result;
    case org::apache::lucene::queryparser::flexible::core::util::
        QueryNodeOperation::ANDOperation::Q2:
      result = q2->cloneTree();
      result->add(q1->cloneTree());
      return result;
    case org::apache::lucene::queryparser::flexible::core::util::
        QueryNodeOperation::ANDOperation::BOTH:
      result = q1->cloneTree();
      result->add(q2->cloneTree()->getChildren());
      return result;
    }
  } catch (const CloneNotSupportedException &e) {
    throw make_shared<QueryNodeError>(e);
  }

  return nullptr;
}
} // namespace org::apache::lucene::queryparser::flexible::core::util