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
namespace org::apache::lucene::queryparser::flexible::spans
{

using Attribute = org::apache::lucene::util::Attribute;

/**
 * This attribute is used by the {@link UniqueFieldQueryNodeProcessor}
 * processor. It holds a value that defines which is the unique field name that
 * should be set in every {@link FieldableNode}.
 *
 * @see UniqueFieldQueryNodeProcessor
 */
class UniqueFieldAttribute : public Attribute
{
  GET_CLASS_NAME(UniqueFieldAttribute)
public:
  virtual void setUniqueField(std::shared_ptr<std::wstring> uniqueField) = 0;

  virtual std::shared_ptr<std::wstring> getUniqueField() = 0;
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/spans/
