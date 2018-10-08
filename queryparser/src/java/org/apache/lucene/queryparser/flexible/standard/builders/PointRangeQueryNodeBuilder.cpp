using namespace std;

#include "PointRangeQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using DoublePoint = org::apache::lucene::document::DoublePoint;
using FloatPoint = org::apache::lucene::document::FloatPoint;
using IntPoint = org::apache::lucene::document::IntPoint;
using LongPoint = org::apache::lucene::document::LongPoint;
using PointValues = org::apache::lucene::index::PointValues;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using StringUtils =
    org::apache::lucene::queryparser::flexible::core::util::StringUtils;
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using PointsConfig =
    org::apache::lucene::queryparser::flexible::standard::config::PointsConfig;
using PointQueryNode =
    org::apache::lucene::queryparser::flexible::standard::nodes::PointQueryNode;
using PointRangeQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::PointRangeQueryNode;
using Query = org::apache::lucene::search::Query;

PointRangeQueryNodeBuilder::PointRangeQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<Query> PointRangeQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  shared_ptr<PointRangeQueryNode> numericRangeNode =
      std::static_pointer_cast<PointRangeQueryNode>(queryNode);

  shared_ptr<PointQueryNode> lowerNumericNode =
      numericRangeNode->getLowerBound();
  shared_ptr<PointQueryNode> upperNumericNode =
      numericRangeNode->getUpperBound();

  shared_ptr<Number> lowerNumber = lowerNumericNode->getValue();
  shared_ptr<Number> upperNumber = upperNumericNode->getValue();

  shared_ptr<PointsConfig> pointsConfig = numericRangeNode->getPointsConfig();
  type_info numberType = pointsConfig->getType();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring field = StringUtils::toString(numericRangeNode->getField());
  bool minInclusive = numericRangeNode->isLowerInclusive();
  bool maxInclusive = numericRangeNode->isUpperInclusive();

  // TODO: push down cleaning up of crazy nulls and inclusive/exclusive
  // elsewhere
  if (Integer::typeid->equals(numberType)) {
    optional<int> lower = static_cast<optional<int>>(lowerNumber);
    if (!lower) {
      lower = numeric_limits<int>::min();
    }
    if (minInclusive == false) {
      lower = lower + 1;
    }

    optional<int> upper = static_cast<optional<int>>(upperNumber);
    if (!upper) {
      upper = numeric_limits<int>::max();
    }
    if (maxInclusive == false) {
      upper = upper - 1;
    }
    return IntPoint::newRangeQuery(field, lower, upper);
  } else if (Long::typeid->equals(numberType)) {
    optional<int64_t> lower = static_cast<optional<int64_t>>(lowerNumber);
    if (!lower) {
      lower = numeric_limits<int64_t>::min();
    }
    if (minInclusive == false) {
      lower = lower + 1;
    }

    optional<int64_t> upper = static_cast<optional<int64_t>>(upperNumber);
    if (!upper) {
      upper = numeric_limits<int64_t>::max();
    }
    if (maxInclusive == false) {
      upper = upper - 1;
    }
    return LongPoint::newRangeQuery(field, lower, upper);
  } else if (Float::typeid->equals(numberType)) {
    optional<float> lower = static_cast<optional<float>>(lowerNumber);
    if (!lower) {
      lower = -numeric_limits<float>::infinity();
    }
    if (minInclusive == false) {
      lower = Math::nextUp(lower);
    }

    optional<float> upper = static_cast<optional<float>>(upperNumber);
    if (!upper) {
      upper = numeric_limits<float>::infinity();
    }
    if (maxInclusive == false) {
      upper = Math::nextDown(upper);
    }
    return FloatPoint::newRangeQuery(field, lower, upper);
  } else if (Double::typeid->equals(numberType)) {
    optional<double> lower = static_cast<optional<double>>(lowerNumber);
    if (!lower) {
      lower = -numeric_limits<double>::infinity();
    }
    if (minInclusive == false) {
      lower = Math::nextUp(lower);
    }

    optional<double> upper = static_cast<optional<double>>(upperNumber);
    if (!upper) {
      upper = numeric_limits<double>::infinity();
    }
    if (maxInclusive == false) {
      upper = Math::nextDown(upper);
    }
    return DoublePoint::newRangeQuery(field, lower, upper);
  } else {
    throw make_shared<QueryNodeException>(make_shared<MessageImpl>(
        QueryParserMessages::UNSUPPORTED_NUMERIC_DATA_TYPE, numberType));
  }
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders