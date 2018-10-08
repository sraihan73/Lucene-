#pragma once
#include "../../util/AttributeImpl.h"
#include "TypeAttribute.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class AttributeImpl;
}

namespace org::apache::lucene::util
{
class AttributeReflector;
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
namespace org::apache::lucene::analysis::tokenattributes
{

using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

/** Default implementation of {@link TypeAttribute}. */
class TypeAttributeImpl : public AttributeImpl,
                          public TypeAttribute,
                          public Cloneable
{
  GET_CLASS_NAME(TypeAttributeImpl)
private:
  // C++ NOTE: Fields cannot have the same name as methods:
  std::wstring type_;

  /** Initialize this attribute with {@link TypeAttribute#DEFAULT_TYPE} */
public:
  TypeAttributeImpl();

  /** Initialize this attribute with <code>type</code> */
  TypeAttributeImpl(const std::wstring &type);

  std::wstring type() override;

  void setType(const std::wstring &type) override;

  void clear() override;

  bool equals(std::any other) override;

  virtual int hashCode();

  void copyTo(std::shared_ptr<AttributeImpl> target) override;

  void reflectWith(AttributeReflector reflector) override;

protected:
  std::shared_ptr<TypeAttributeImpl> shared_from_this()
  {
    return std::static_pointer_cast<TypeAttributeImpl>(
        org.apache.lucene.util.AttributeImpl::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::tokenattributes
