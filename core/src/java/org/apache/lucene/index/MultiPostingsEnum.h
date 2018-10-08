#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class MultiTermsEnum;
}

namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::index
{
class EnumWithSlice;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class ReaderSlice;
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
namespace org::apache::lucene::index
{

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Exposes {@link PostingsEnum}, merged from {@link PostingsEnum}
 * API of sub-segments.
 *
 * @lucene.experimental
 */

class MultiPostingsEnum final : public PostingsEnum
{
  GET_CLASS_NAME(MultiPostingsEnum)
private:
  const std::shared_ptr<MultiTermsEnum> parent;

public:
  std::deque<std::shared_ptr<PostingsEnum>> const subPostingsEnums;

private:
  std::deque<std::shared_ptr<EnumWithSlice>> const subs;

public:
  int numSubs = 0;
  int upto = 0;
  std::shared_ptr<PostingsEnum> current;
  int currentBase = 0;
  int doc = -1;

  /** Sole constructor.
   * @param parent The {@link MultiTermsEnum} that created us.
   * @param subReaderCount How many sub-readers are being merged. */
  MultiPostingsEnum(std::shared_ptr<MultiTermsEnum> parent, int subReaderCount);

  /** Returns {@code true} if this instance can be reused by
   *  the provided {@link MultiTermsEnum}. */
  bool canReuse(std::shared_ptr<MultiTermsEnum> parent);

  /** Re-use and reset this instance on the provided slices. */
  std::shared_ptr<MultiPostingsEnum>
  reset(std::deque<std::shared_ptr<EnumWithSlice>> &subs, int const numSubs);

  /** How many sub-readers we are merging.
   *  @see #getSubs */
  int getNumSubs();

  /** Returns sub-readers we are merging. */
  std::deque<std::shared_ptr<EnumWithSlice>> getSubs();

  int freq()  override;

  int docID() override;

  int advance(int target)  override;

  int nextDoc()  override;

  int nextPosition()  override;

  int startOffset()  override;

  int endOffset()  override;

  std::shared_ptr<BytesRef> getPayload()  override;

  // TODO: implement bulk read more efficiently than super
  /** Holds a {@link PostingsEnum} along with the
   *  corresponding {@link ReaderSlice}. */
public:
  class EnumWithSlice final : public std::enable_shared_from_this<EnumWithSlice>
  {
    GET_CLASS_NAME(EnumWithSlice)
    /** {@link PostingsEnum} for this sub-reader. */
  public:
    std::shared_ptr<PostingsEnum> postingsEnum;

    /** {@link ReaderSlice} describing how this sub-reader
     *  fits into the composite reader. */
    std::shared_ptr<ReaderSlice> slice;

    EnumWithSlice();

    virtual std::wstring toString();
  };

public:
  int64_t cost() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<MultiPostingsEnum> shared_from_this()
  {
    return std::static_pointer_cast<MultiPostingsEnum>(
        PostingsEnum::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
