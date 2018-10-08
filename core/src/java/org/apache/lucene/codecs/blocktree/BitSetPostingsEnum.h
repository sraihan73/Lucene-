#pragma once
#include "../../index/PostingsEnum.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BitSet;
}

namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
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
namespace org::apache::lucene::codecs::blocktree
{

using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using BitSet = org::apache::lucene::util::BitSet;
using BytesRef = org::apache::lucene::util::BytesRef;

/** Takes a {@link FixedBitSet} and creates a DOCS {@link PostingsEnum} from it.
 */

class BitSetPostingsEnum : public PostingsEnum
{
  GET_CLASS_NAME(BitSetPostingsEnum)
private:
  const std::shared_ptr<BitSet> bits;
  std::shared_ptr<DocIdSetIterator> in_;

public:
  BitSetPostingsEnum(std::shared_ptr<BitSet> bits);

  int freq()  override;

  int docID() override;

  int nextDoc()  override;

  int advance(int target)  override;

  int64_t cost() override;

  virtual void reset();

  std::shared_ptr<BytesRef> getPayload() override;

  int nextPosition() override;

  int startOffset() override;

  int endOffset() override;

protected:
  std::shared_ptr<BitSetPostingsEnum> shared_from_this()
  {
    return std::static_pointer_cast<BitSetPostingsEnum>(
        org.apache.lucene.index.PostingsEnum::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blocktree
