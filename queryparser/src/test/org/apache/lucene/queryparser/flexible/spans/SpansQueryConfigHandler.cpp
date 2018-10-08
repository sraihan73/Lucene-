using namespace std;

#include "SpansQueryConfigHandler.h"

namespace org::apache::lucene::queryparser::flexible::spans
{
using ConfigurationKey =
    org::apache::lucene::queryparser::flexible::core::config::ConfigurationKey;
using FieldConfig =
    org::apache::lucene::queryparser::flexible::core::config::FieldConfig;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
const shared_ptr<org::apache::lucene::queryparser::flexible::core::config::
                     ConfigurationKey<wstring>>
    SpansQueryConfigHandler::UNIQUE_FIELD = org::apache::lucene::queryparser::
        flexible::core::config::ConfigurationKey::newInstance();

SpansQueryConfigHandler::SpansQueryConfigHandler()
{
  // empty constructor
}

shared_ptr<FieldConfig>
SpansQueryConfigHandler::getFieldConfig(const wstring &fieldName)
{

  // there is no field configuration, always return null
  return nullptr;
}
} // namespace org::apache::lucene::queryparser::flexible::spans