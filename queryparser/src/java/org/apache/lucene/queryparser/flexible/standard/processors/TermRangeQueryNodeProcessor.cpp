using namespace std;

#include "TermRangeQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using DateTools = org::apache::lucene::document::DateTools;
using Resolution = org::apache::lucene::document::DateTools::Resolution;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using FieldConfig =
    org::apache::lucene::queryparser::flexible::core::config::FieldConfig;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;
using TermRangeQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::TermRangeQueryNode;

TermRangeQueryNodeProcessor::TermRangeQueryNodeProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode> TermRangeQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<TermRangeQueryNode>(node) != nullptr) {
    shared_ptr<TermRangeQueryNode> termRangeNode =
        std::static_pointer_cast<TermRangeQueryNode>(node);
    shared_ptr<FieldQueryNode> upper = termRangeNode->getUpperBound();
    shared_ptr<FieldQueryNode> lower = termRangeNode->getLowerBound();

    DateTools::Resolution dateRes = nullptr;
    bool inclusive = false;
    shared_ptr<Locale> locale =
        getQueryConfigHandler()->get(ConfigurationKeys::LOCALE);

    if (locale == nullptr) {
      locale = Locale::getDefault();
    }

    shared_ptr<TimeZone> timeZone =
        getQueryConfigHandler()->get(ConfigurationKeys::TIMEZONE);

    if (timeZone == nullptr) {
      timeZone = TimeZone::getDefault();
    }

    shared_ptr<std::wstring> field = termRangeNode->getField();
    wstring fieldStr = L"";

    if (field != nullptr) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      fieldStr = field->toString();
    }

    shared_ptr<FieldConfig> fieldConfig =
        getQueryConfigHandler()->getFieldConfig(fieldStr);

    if (fieldConfig != nullptr) {
      dateRes = fieldConfig->get(ConfigurationKeys::DATE_RESOLUTION);
    }

    if (termRangeNode->isUpperInclusive()) {
      inclusive = true;
    }

    wstring part1 = lower->getTextAsString();
    wstring part2 = upper->getTextAsString();

    try {
      shared_ptr<DateFormat> df =
          DateFormat::getDateInstance(DateFormat::SHORT, locale);
      df->setLenient(true);

      if (part1.length() > 0) {
        Date d1 = df->parse(part1);
        part1 = DateTools::dateToString(d1, dateRes);
        lower->setText(part1);
      }

      if (part2.length() > 0) {
        Date d2 = df->parse(part2);
        if (inclusive) {
          // The user can only specify the date, not the time, so make sure
          // the time is set to the latest possible time of that date to
          // really
          // include all documents:
          shared_ptr<Calendar> cal = Calendar::getInstance(timeZone, locale);
          cal->setTime(d2);
          cal->set(Calendar::HOUR_OF_DAY, 23);
          cal->set(Calendar::MINUTE, 59);
          cal->set(Calendar::SECOND, 59);
          cal->set(Calendar::MILLISECOND, 999);
          d2 = cal->getTime();
        }

        part2 = DateTools::dateToString(d2, dateRes);
        upper->setText(part2);
      }

    } catch (const runtime_error &e) {
      // not a date
      shared_ptr<Analyzer> analyzer =
          getQueryConfigHandler()->get(ConfigurationKeys::ANALYZER);
      if (analyzer != nullptr) {
        // because we call utf8ToString, this will only work with the default
        // TermToBytesRefAttribute
        part1 = analyzer->normalize(lower->getFieldAsString(), part1)
                    ->utf8ToString();
        part2 = analyzer->normalize(lower->getFieldAsString(), part2)
                    ->utf8ToString();
        lower->setText(part1);
        upper->setText(part2);
      }
    }
  }

  return node;
}

shared_ptr<QueryNode> TermRangeQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

deque<std::shared_ptr<QueryNode>>
TermRangeQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors