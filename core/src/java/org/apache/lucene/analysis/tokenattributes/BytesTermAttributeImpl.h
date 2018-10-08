#pragma once
#include "../../util/AttributeImpl.h"
#include "BytesTermAttribute.h"
#include "TermToBytesRefAttribute.h"
#include "stringhelper.h"
#include <any>
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::analysis::tokenattributes
{

using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;

/** Implementation class for {@link BytesTermAttribute}.
 * @lucene.internal
 */
class BytesTermAttributeImpl : public AttributeImpl,
                               public BytesTermAttribute,
                               public TermToBytesRefAttribute
{
  GET_CLASS_NAME(BytesTermAttributeImpl)
private:
  std::shared_ptr<BytesRef> bytes;

  /** Initialize this attribute with no bytes. */
public:
  BytesTermAttributeImpl();

  std::shared_ptr<BytesRef> getBytesRef() override;

  void setBytesRef(std::shared_ptr<BytesRef> bytes) override;

  void clear() override;

  void copyTo(std::shared_ptr<AttributeImpl> target) override;

  std::shared_ptr<AttributeImpl> clone() override;

  void reflectWith(AttributeReflector reflector) override;

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<BytesTermAttributeImpl> shared_from_this()
  {
    return std::static_pointer_cast<BytesTermAttributeImpl>(
        org.apache.lucene.util.AttributeImpl::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/analysis/tokenattributes/
