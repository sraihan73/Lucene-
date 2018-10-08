#pragma once
#include "../../util/AttributeImpl.h"
#include "OffsetAttribute.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
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

/** Default implementation of {@link OffsetAttribute}. */
class OffsetAttributeImpl : public AttributeImpl,
                            public OffsetAttribute,
                            public Cloneable
{
  GET_CLASS_NAME(OffsetAttributeImpl)
private:
  // C++ NOTE: Fields cannot have the same name as methods:
  int startOffset_ = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  int endOffset_ = 0;

  /** Initialize this attribute with startOffset and endOffset of 0. */
public:
  OffsetAttributeImpl();

  int startOffset() override;

  void setOffset(int startOffset, int endOffset) override;

  int endOffset() override;

  void clear() override;

  bool equals(std::any other) override;

  virtual int hashCode();

  void copyTo(std::shared_ptr<AttributeImpl> target) override;

  void reflectWith(AttributeReflector reflector) override;

protected:
  std::shared_ptr<OffsetAttributeImpl> shared_from_this()
  {
    return std::static_pointer_cast<OffsetAttributeImpl>(
        org.apache.lucene.util.AttributeImpl::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::tokenattributes
