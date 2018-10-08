using namespace std;

#include "PointRangeQueryNode.h"

namespace org::apache::lucene::queryparser::flexible::standard::nodes
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using PointsConfig =
    org::apache::lucene::queryparser::flexible::standard::config::PointsConfig;

PointRangeQueryNode::PointRangeQueryNode(
    shared_ptr<PointQueryNode> lower, shared_ptr<PointQueryNode> upper,
    bool lowerInclusive, bool upperInclusive,
    shared_ptr<PointsConfig> numericConfig) 
{
  setBounds(lower, upper, lowerInclusive, upperInclusive, numericConfig);
}

void PointRangeQueryNode::setBounds(
    shared_ptr<PointQueryNode> lower, shared_ptr<PointQueryNode> upper,
    bool lowerInclusive, bool upperInclusive,
    shared_ptr<PointsConfig> pointsConfig) 
{

  if (pointsConfig == nullptr) {
    throw invalid_argument(L"pointsConfig must not be null!");
  }

  type_info lowerNumberType, upperNumberType;

  if (lower != nullptr && lower->getValue() != nullptr) {
    lowerNumberType = lower->getValue()->getClass();
  } else {
    lowerNumberType = nullptr;
  }

  if (upper != nullptr && upper->getValue() != nullptr) {
    upperNumberType = upper->getValue()->getClass();
  } else {
    upperNumberType = nullptr;
  }

  if (lowerNumberType != nullptr &&
      !lowerNumberType.equals(pointsConfig->getType())) {
    throw invalid_argument(
        L"lower value's type should be the same as numericConfig type: " +
        lowerNumberType + L" != " + pointsConfig->getType());
  }

  if (upperNumberType != nullptr &&
      !upperNumberType.equals(pointsConfig->getType())) {
    throw invalid_argument(
        L"upper value's type should be the same as numericConfig type: " +
        upperNumberType + L" != " + pointsConfig->getType());
  }

  AbstractRangeQueryNode<PointQueryNode>::setBounds(
      lower, upper, lowerInclusive, upperInclusive);
  this->numericConfig = pointsConfig;
}

shared_ptr<PointsConfig> PointRangeQueryNode::getPointsConfig()
{
  return this->numericConfig;
}

wstring PointRangeQueryNode::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"<pointRange lowerInclusive='");
  sb->append(isLowerInclusive());
  sb->append(L"' upperInclusive='");
  sb->append(isUpperInclusive());
  sb->append(L"' type='");
  sb->append(numericConfig->getType().name());
  sb->append(L"'>\n");
  sb->append(getLowerBound())->append(L'\n');
  sb->append(getUpperBound())->append(L'\n');
  sb->append(L"</pointRange>");
  return sb->toString();
}
} // namespace org::apache::lucene::queryparser::flexible::standard::nodes