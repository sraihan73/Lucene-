#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "TermsIndexReaderBase.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class PagedBytes;
}

namespace org::apache::lucene::util
{
class Reader;
}
namespace org::apache::lucene::codecs::blockterms
{
class FieldIndexData;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util::packed
{
class MonotonicBlockPackedReader;
}
namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::util
{
class Accountable;
}
namespace org::apache::lucene::index
{
class FieldInfo;
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
namespace org::apache::lucene::codecs::blockterms
{

using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using PagedBytes = org::apache::lucene::util::PagedBytes;
using MonotonicBlockPackedReader =
    org::apache::lucene::util::packed::MonotonicBlockPackedReader;

/**
 * TermsIndexReader for simple every Nth terms indexes.
 *
 * @see FixedGapTermsIndexWriter
 * @lucene.experimental
 */
class FixedGapTermsIndexReader : public TermsIndexReaderBase
{
  GET_CLASS_NAME(FixedGapTermsIndexReader)

  // NOTE: long is overkill here, but we use this in a
  // number of places to multiply out the actual ord, and we
  // will overflow int during those multiplies.  So to avoid
  // having to upgrade each multiple to long in multiple
  // places (error prone), we use long here:
private:
  const int64_t indexInterval;

  const int packedIntsVersion;
  const int blocksize;

  static constexpr int PAGED_BYTES_BITS = 15;

  // all fields share this single logical byte[]
  const std::shared_ptr<PagedBytes::Reader> termBytesReader;

public:
  const std::unordered_map<std::wstring, std::shared_ptr<FieldIndexData>>
      fields =
          std::unordered_map<std::wstring, std::shared_ptr<FieldIndexData>>();

  FixedGapTermsIndexReader(std::shared_ptr<SegmentReadState> state) throw(
      IOException);

private:
  class IndexEnum : public FieldIndexEnum
  {
    GET_CLASS_NAME(IndexEnum)
  private:
    std::shared_ptr<FixedGapTermsIndexReader> outerInstance;

    const std::shared_ptr<FieldIndexData> fieldIndex;
    // C++ NOTE: Fields cannot have the same name as methods:
    const std::shared_ptr<BytesRef> term_ = std::make_shared<BytesRef>();
    // C++ NOTE: Fields cannot have the same name as methods:
    int64_t ord_ = 0;

  public:
    IndexEnum(std::shared_ptr<FixedGapTermsIndexReader> outerInstance,
              std::shared_ptr<FieldIndexData> fieldIndex);

    std::shared_ptr<BytesRef> term() override;

    int64_t seek(std::shared_ptr<BytesRef> target) override;

    int64_t next() override;

    int64_t ord() override;

    int64_t seek(int64_t ord) override;

  protected:
    std::shared_ptr<IndexEnum> shared_from_this()
    {
      return std::static_pointer_cast<IndexEnum>(
          FieldIndexEnum::shared_from_this());
    }
  };

public:
  bool supportsOrd() override;

private:
  class FieldIndexData final
      : public std::enable_shared_from_this<FieldIndexData>,
        public Accountable
  {
    GET_CLASS_NAME(FieldIndexData)
  private:
    std::shared_ptr<FixedGapTermsIndexReader> outerInstance;

    // where this field's terms begin in the packed byte[]
    // data
  public:
    const int64_t termBytesStart;

    // offset into index termBytes
    const std::shared_ptr<MonotonicBlockPackedReader> termOffsets;

    // index pointers into main terms dict
    const std::shared_ptr<MonotonicBlockPackedReader> termsDictOffsets;

    const int64_t numIndexTerms;
    const int64_t termsStart;

    FieldIndexData(std::shared_ptr<FixedGapTermsIndexReader> outerInstance,
                   std::shared_ptr<IndexInput> in_,
                   std::shared_ptr<PagedBytes> termBytes, int64_t indexStart,
                   int64_t termsStart, int64_t packedIndexStart,
                   int64_t packedOffsetsStart,
                   int64_t numIndexTerms) ;

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    virtual std::wstring toString();
  };

public:
  std::shared_ptr<FieldIndexEnum>
  getFieldEnum(std::shared_ptr<FieldInfo> fieldInfo) override;

  virtual ~FixedGapTermsIndexReader();

private:
  void seekDir(std::shared_ptr<IndexInput> input) ;

public:
  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<FixedGapTermsIndexReader> shared_from_this()
  {
    return std::static_pointer_cast<FixedGapTermsIndexReader>(
        TermsIndexReaderBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blockterms
