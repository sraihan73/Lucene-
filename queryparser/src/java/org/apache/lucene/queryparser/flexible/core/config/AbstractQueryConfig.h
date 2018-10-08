#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::flexible::core::config
{
template <typename T>
class ConfigurationKey;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::queryparser::flexible::core::config
{

/**
 * <p>
 * This class is the base of {@link QueryConfigHandler} and {@link FieldConfig}.
 * It has operations to set, unset and get configuration values.
 * </p>
 * <p>
 * Each configuration is is a key-&gt;value pair. The key should be an unique
 * {@link ConfigurationKey} instance and it also holds the value's type.
 * </p>
 *
 * @see ConfigurationKey
 */
class AbstractQueryConfig
    : public std::enable_shared_from_this<AbstractQueryConfig>
{
  GET_CLASS_NAME(AbstractQueryConfig)

private:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private final java.util.HashMap<ConfigurationKey<?>, Object>
  // configMap = new java.util.HashMap<>();
  const std::unordered_map < ConfigurationKey < ? >,
      std::any > configMap = std::unordered_map < ConfigurationKey < ? >,
                 std::any > ();

public:
  AbstractQueryConfig();

  /**
   * Returns the value held by the given key.
   *
   * @param <T> the value's type
   *
   * @param key the key, cannot be <code>null</code>
   *
   * @return the value held by the given key
   */
  template <typename T>
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") public <T> T
  // get(ConfigurationKey<T> key)
  T get(std::shared_ptr<ConfigurationKey<T>> key);

  /**
   * Returns true if there is a value set with the given key, otherwise false.
   *
   * @param <T> the value's type
   * @param key the key, cannot be <code>null</code>
   * @return true if there is a value set with the given key, otherwise false
   */
  template <typename T>
  bool has(std::shared_ptr<ConfigurationKey<T>> key);

  /**
   * Sets a key and its value.
   *
   * @param <T> the value's type
   * @param key the key, cannot be <code>null</code>
   * @param value value to set
   */
  template <typename T>
  void set(std::shared_ptr<ConfigurationKey<T>> key, T value);

  /**
   * Unsets the given key and its value.
   *
   * @param <T> the value's type
   * @param key the key
   * @return true if the key and value was set and removed, otherwise false
   */
  template <typename T>
  bool unset(std::shared_ptr<ConfigurationKey<T>> key);
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/config/
