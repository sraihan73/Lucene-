#pragma once
#include "stringhelper.h"
#include <deque>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/config/FieldConfigListener.h"

#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/config/FieldConfig.h"

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
 * This class can be used to hold any query configuration and no field
 * configuration. For field configuration, it creates an empty
 * {@link FieldConfig} object and delegate it to field config listeners,
 * these are responsible for setting up all the field configuration.
 *
 * {@link QueryConfigHandler} should be extended by classes that intends to
 * provide configuration to {@link QueryNodeProcessor} objects.
GET_CLASS_NAME(es)
 *
 * The class that extends {@link QueryConfigHandler} should also provide
 * {@link FieldConfig} objects for each collection field.
GET_CLASS_NAME(that)
 *
 * @see FieldConfig
 * @see FieldConfigListener
 * @see QueryConfigHandler
 */
class QueryConfigHandler : public AbstractQueryConfig
{
  GET_CLASS_NAME(QueryConfigHandler)

private:
  const std::deque<std::shared_ptr<FieldConfigListener>> listeners =
      std::deque<std::shared_ptr<FieldConfigListener>>();

  /**
   * Returns an implementation of
   * {@link FieldConfig} for a specific field name. If the implemented
   * {@link QueryConfigHandler} does not know a specific field name, it may
   * return <code>null</code>, indicating there is no configuration for that
   * field.
   *
   * @param fieldName
   *          the field name
   * @return a {@link FieldConfig} object containing the field name
   *         configuration or <code>null</code>, if the implemented
   *         {@link QueryConfigHandler} has no configuration for that field
   */
public:
  virtual std::shared_ptr<FieldConfig>
  getFieldConfig(const std::wstring &fieldName);

  /**
   * Adds a listener. The added listeners are called in the order they are
   * added.
   *
   * @param listener
   *          the listener to be added
   */
  virtual void
  addFieldConfigListener(std::shared_ptr<FieldConfigListener> listener);

protected:
  std::shared_ptr<QueryConfigHandler> shared_from_this()
  {
    return std::static_pointer_cast<QueryConfigHandler>(
        AbstractQueryConfig::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/config/
