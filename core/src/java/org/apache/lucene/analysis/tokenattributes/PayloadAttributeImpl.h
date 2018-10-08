#pragma once
#include "../../util/AttributeImpl.h"
#include "PayloadAttribute.h"
#include "stringhelper.h"
#include <any>
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

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
using BytesRef = org::apache::lucene::util::BytesRef;

/** Default implementation of {@link PayloadAttribute}. */
class PayloadAttributeImpl : public AttributeImpl,
                             public PayloadAttribute,
                             public Cloneable
{
  GET_CLASS_NAME(PayloadAttributeImpl)
private:
  std::shared_ptr<BytesRef> payload;

  /**
   * Initialize this attribute with no payload.
   */
public:
  PayloadAttributeImpl();

  /**
   * Initialize this attribute with the given payload.
   */
  PayloadAttributeImpl(std::shared_ptr<BytesRef> payload);

  std::shared_ptr<BytesRef> getPayload() override;

  void setPayload(std::shared_ptr<BytesRef> payload) override;

  void clear() override;

  std::shared_ptr<PayloadAttributeImpl> clone() override;

  bool equals(std::any other) override;

  virtual int hashCode();

  void copyTo(std::shared_ptr<AttributeImpl> target) override;

  void reflectWith(AttributeReflector reflector) override;

protected:
  std::shared_ptr<PayloadAttributeImpl> shared_from_this()
  {
    return std::static_pointer_cast<PayloadAttributeImpl>(
        org.apache.lucene.util.AttributeImpl::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::tokenattributes
