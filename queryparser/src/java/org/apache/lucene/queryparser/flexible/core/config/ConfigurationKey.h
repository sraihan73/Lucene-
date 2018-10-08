#pragma once
#include "stringhelper.h"
#include <memory>

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
 * An instance of this class represents a key that is used to retrieve a value
 * from {@link AbstractQueryConfig}. It also holds the value's type, which is
GET_CLASS_NAME(represents)
 * defined in the generic argument.
 *
 * @see AbstractQueryConfig
 */
template <typename T>
class ConfigurationKey final
    : public std::enable_shared_from_this<ConfigurationKey>
{
  GET_CLASS_NAME(ConfigurationKey)

private:
  ConfigurationKey() {}

  /**
   * Creates a new instance.
   *
   * @param <T> the value's type
   *
   * @return a new instance
   */
public:
  template <typename T>
  static std::shared_ptr<ConfigurationKey<T>> newInstance()
  {
    return std::make_shared<ConfigurationKey<T>>();
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/config/
