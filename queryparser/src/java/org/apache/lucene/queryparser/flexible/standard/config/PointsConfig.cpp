using namespace std;

#include "PointsConfig.h"

namespace org::apache::lucene::queryparser::flexible::standard::config
{
using PointValues = org::apache::lucene::index::PointValues;

PointsConfig::PointsConfig(shared_ptr<NumberFormat> format, type_info type)
{
  setNumberFormat(format);
  setType(type);
}

shared_ptr<NumberFormat> PointsConfig::getNumberFormat() { return format; }

type_info PointsConfig::getType() { return type; }

void PointsConfig::setType(type_info type)
{
  if (type == nullptr) {
    throw invalid_argument(L"type must not be null!");
  }
  if (Integer::typeid->equals(type) == false &&
      Long::typeid->equals(type) == false &&
      Float::typeid->equals(type) == false &&
      Double::typeid->equals(type) == false) {
    throw invalid_argument(L"unsupported numeric type: " + type);
  }
  this->type = type;
}

void PointsConfig::setNumberFormat(shared_ptr<NumberFormat> format)
{
  if (format == nullptr) {
    throw invalid_argument(L"format must not be null!");
  }
  this->format = format;
}

int PointsConfig::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + format->hashCode();
  result = prime * result + type.hash_code();
  return result;
}

bool PointsConfig::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<PointsConfig> other = any_cast<std::shared_ptr<PointsConfig>>(obj);
  if (!format->equals(other->format)) {
    return false;
  }
  if (!type.equals(other->type)) {
    return false;
  }
  return true;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::config