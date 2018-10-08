using namespace std;

#include "AbstractQueryConfig.h"

namespace org::apache::lucene::queryparser::flexible::core::config
{

AbstractQueryConfig::AbstractQueryConfig()
{
  // although this class is public, it can only be constructed from package
}

template <typename T>
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") public <T> T
// get(ConfigurationKey<T> key)
T AbstractQueryConfig::get(shared_ptr<ConfigurationKey<T>> key)
{

  if (key == nullptr) {
    throw invalid_argument(L"key must not be null!");
  }

  return any_cast<T>(this->configMap[key]);
}

template <typename T>
bool AbstractQueryConfig::has(shared_ptr<ConfigurationKey<T>> key)
{

  if (key == nullptr) {
    throw invalid_argument(L"key must not be null!");
  }

  return this->configMap.find(key) != this->configMap.end();
}

template <typename T>
void AbstractQueryConfig::set(shared_ptr<ConfigurationKey<T>> key, T value)
{

  if (key == nullptr) {
    throw invalid_argument(L"key must not be null!");
  }

  if (value == nullptr) {
    unset(key);

  } else {
    this->configMap.emplace(key, value);
  }
}

template <typename T>
bool AbstractQueryConfig::unset(shared_ptr<ConfigurationKey<T>> key)
{

  if (key == nullptr) {
    throw invalid_argument(L"key must not be null!");
  }

  return this->configMap.erase(key) != nullptr;
}
} // namespace org::apache::lucene::queryparser::flexible::core::config