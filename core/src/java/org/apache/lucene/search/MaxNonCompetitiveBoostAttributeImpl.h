#pragma once
#include "stringhelper.h"
#include <limits>
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
namespace org::apache::lucene::search
{

using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;

/** Implementation class for {@link MaxNonCompetitiveBoostAttribute}.
 * @lucene.internal
 */
class MaxNonCompetitiveBoostAttributeImpl final
    : public AttributeImpl,
      public MaxNonCompetitiveBoostAttribute
{
  GET_CLASS_NAME(MaxNonCompetitiveBoostAttributeImpl)
private:
  float maxNonCompetitiveBoost = -std::numeric_limits<float>::infinity();
  std::shared_ptr<BytesRef> competitiveTerm = nullptr;

public:
  void setMaxNonCompetitiveBoost(float const maxNonCompetitiveBoost) override;

  float getMaxNonCompetitiveBoost() override;

  void setCompetitiveTerm(std::shared_ptr<BytesRef> competitiveTerm) override;

  std::shared_ptr<BytesRef> getCompetitiveTerm() override;

  void clear() override;

  void copyTo(std::shared_ptr<AttributeImpl> target) override;

  void reflectWith(AttributeReflector reflector) override;

protected:
  std::shared_ptr<MaxNonCompetitiveBoostAttributeImpl> shared_from_this()
  {
    return std::static_pointer_cast<MaxNonCompetitiveBoostAttributeImpl>(
        org.apache.lucene.util.AttributeImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
