#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs::idversion
{
class IDVersionTermState;
}

namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::codecs
{
class BlockTermState;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::store
{
class IndexOutput;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::store
{
class DataOutput;
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

using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using PushPostingsWriterBase =
    org::apache::lucene::codecs::PushPostingsWriterBase;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;

class IDVersionPostingsWriter final : public PushPostingsWriterBase
{
  GET_CLASS_NAME(IDVersionPostingsWriter)

public:
  static const std::wstring TERMS_CODEC;

  // Increment version to change it
  static constexpr int VERSION_START = 1;
  static constexpr int VERSION_CURRENT = VERSION_START;

  static const std::shared_ptr<IDVersionTermState> emptyState;
  std::shared_ptr<IDVersionTermState> lastState;

  int lastDocID = 0;

private:
  int lastPosition = 0;
  int64_t lastVersion = 0;

  const std::shared_ptr<Bits> liveDocs;
  std::wstring segment;

public:
  IDVersionPostingsWriter(std::shared_ptr<Bits> liveDocs);

  std::shared_ptr<BlockTermState> newTermState() override;

  void
  init(std::shared_ptr<IndexOutput> termsOut,
       std::shared_ptr<SegmentWriteState> state)  override;

  int setField(std::shared_ptr<FieldInfo> fieldInfo) override;

  void startTerm() override;

  void startDoc(int docID, int termDocFreq)  override;

  void addPosition(int position, std::shared_ptr<BytesRef> payload,
                   int startOffset, int endOffset)  override;

  void finishDoc()  override;

  /** Called when we are done adding docs to this term */
  void finishTerm(std::shared_ptr<BlockTermState> _state) throw(
      IOException) override;

private:
  int64_t lastEncodedVersion = 0;

public:
  void encodeTerm(std::deque<int64_t> &longs,
                  std::shared_ptr<DataOutput> out,
                  std::shared_ptr<FieldInfo> fieldInfo,
                  std::shared_ptr<BlockTermState> _state,
                  bool absolute)  override;

  virtual ~IDVersionPostingsWriter();

protected:
  std::shared_ptr<IDVersionPostingsWriter> shared_from_this()
  {
    return std::static_pointer_cast<IDVersionPostingsWriter>(
        org.apache.lucene.codecs.PushPostingsWriterBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::idversion
