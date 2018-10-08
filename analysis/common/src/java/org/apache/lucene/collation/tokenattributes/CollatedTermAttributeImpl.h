#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttributeImpl.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::collation::tokenattributes
{

using CharTermAttributeImpl =
    org::apache::lucene::analysis::tokenattributes::CharTermAttributeImpl;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Extension of {@link CharTermAttributeImpl} that encodes the term
 * text as a binary Unicode collation key instead of as UTF-8 bytes.
 */
class CollatedTermAttributeImpl : public CharTermAttributeImpl
{
  GET_CLASS_NAME(CollatedTermAttributeImpl)
private:
  const std::shared_ptr<Collator> collator;

  /**
   * Create a new CollatedTermAttributeImpl
   * @param collator Collation key generator
   */
public:
  CollatedTermAttributeImpl(std::shared_ptr<Collator> collator);

  std::shared_ptr<BytesRef> getBytesRef() override;

protected:
  std::shared_ptr<CollatedTermAttributeImpl> shared_from_this()
  {
    return std::static_pointer_cast<CollatedTermAttributeImpl>(
        org.apache.lucene.analysis.tokenattributes
            .CharTermAttributeImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/collation/tokenattributes/
