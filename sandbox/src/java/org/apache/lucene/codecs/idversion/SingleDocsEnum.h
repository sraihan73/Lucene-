#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::codecs::idversion
{

using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;

class SingleDocsEnum : public PostingsEnum
{
  GET_CLASS_NAME(SingleDocsEnum)

private:
  int doc = 0;
  int singleDocID = 0;

  /** For reuse */
public:
  virtual void reset(int singleDocID);

  int nextDoc() override;

  int docID() override;

  int advance(int target) override;

  int64_t cost() override;

  int freq() override;

  int nextPosition()  override;

  int startOffset()  override;

  int endOffset()  override;

  std::shared_ptr<BytesRef> getPayload()  override;

protected:
  std::shared_ptr<SingleDocsEnum> shared_from_this()
  {
    return std::static_pointer_cast<SingleDocsEnum>(
        org.apache.lucene.index.PostingsEnum::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::idversion
