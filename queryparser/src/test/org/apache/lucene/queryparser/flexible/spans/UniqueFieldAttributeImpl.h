#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/AttributeImpl.h"

#include  "core/src/java/org/apache/lucene/util/AttributeReflector.h"

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

using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

/**
 * This attribute is used by the {@link UniqueFieldQueryNodeProcessor}
 * processor. It holds a value that defines which is the unique field name that
 * should be set in every {@link FieldableNode}.
 *
 * @see UniqueFieldQueryNodeProcessor
 */
class UniqueFieldAttributeImpl : public AttributeImpl,
                                 public UniqueFieldAttribute
{
  GET_CLASS_NAME(UniqueFieldAttributeImpl)

private:
  std::shared_ptr<std::wstring> uniqueField;

public:
  UniqueFieldAttributeImpl();

  void clear() override;

  void setUniqueField(std::shared_ptr<std::wstring> uniqueField) override;

  std::shared_ptr<std::wstring> getUniqueField() override;

  void copyTo(std::shared_ptr<AttributeImpl> target) override;

  bool equals(std::any other) override;

  virtual int hashCode();

  void reflectWith(AttributeReflector reflector) override;

protected:
  std::shared_ptr<UniqueFieldAttributeImpl> shared_from_this()
  {
    return std::static_pointer_cast<UniqueFieldAttributeImpl>(
        org.apache.lucene.util.AttributeImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/spans/
