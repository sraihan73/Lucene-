#pragma once
#include "../../index/TermsEnum.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs::blocktree
{
class BitSetPostingsEnum;
}

namespace org::apache::lucene::util
{
class BitSet;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
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
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BitSet = org::apache::lucene::util::BitSet;
using BytesRef = org::apache::lucene::util::BytesRef;

/** Silly stub class, used only when writing an auto-prefix
 *  term in order to expose DocsEnum over a FixedBitSet.  We
 *  pass this to {@link PostingsWriterBase#writeTerm} so
 *  that it can pull .docs() multiple times for the
 *  current term. */

class BitSetTermsEnum : public TermsEnum
{
  GET_CLASS_NAME(BitSetTermsEnum)
private:
  const std::shared_ptr<BitSetPostingsEnum> postingsEnum;

public:
  BitSetTermsEnum(std::shared_ptr<BitSet> docs);

  SeekStatus seekCeil(std::shared_ptr<BytesRef> text) override;

  void seekExact(int64_t ord) override;

  std::shared_ptr<BytesRef> term() override;

  std::shared_ptr<BytesRef> next() override;

  int64_t ord() override;

  int docFreq() override;

  int64_t totalTermFreq() override;

  std::shared_ptr<PostingsEnum> postings(std::shared_ptr<PostingsEnum> reuse,
                                         int flags) override;

protected:
  std::shared_ptr<BitSetTermsEnum> shared_from_this()
  {
    return std::static_pointer_cast<BitSetTermsEnum>(
        org.apache.lucene.index.TermsEnum::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blocktree
