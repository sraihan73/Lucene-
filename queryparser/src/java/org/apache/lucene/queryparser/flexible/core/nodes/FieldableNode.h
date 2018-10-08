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
namespace org::apache::lucene::queryparser::flexible::core::nodes
{

/**
 * A query node implements {@link FieldableNode} interface to indicate that its
 * children and itself are associated to a specific field.
 *
 * If it has any children which also implements this interface, it must ensure
 * the children are associated to the same field.
 *
 */
class FieldableNode : public QueryNode
{
  GET_CLASS_NAME(FieldableNode)

  /**
   * Returns the field associated to the node and every node under it.
   *
   * @return the field name
   */
public:
  virtual std::shared_ptr<std::wstring> getField() = 0;

  /**
   * Associates the node to a field.
   *
   * @param fieldName
   *          the field name
   */
  virtual void setField(std::shared_ptr<std::wstring> fieldName) = 0;
};

} // namespace org::apache::lucene::queryparser::flexible::core::nodes
