#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"

#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/store/DataInput.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"

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
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using DataInput = org::apache::lucene::store::DataInput;
using IndexInput = org::apache::lucene::store::IndexInput;

class IDVersionPostingsReader final : public PostingsReaderBase
{
  GET_CLASS_NAME(IDVersionPostingsReader)

public:
  void
  init(std::shared_ptr<IndexInput> termsIn,
       std::shared_ptr<SegmentReadState> state)  override;

  std::shared_ptr<BlockTermState> newTermState() override;

  virtual ~IDVersionPostingsReader();

  void decodeTerm(std::deque<int64_t> &longs, std::shared_ptr<DataInput> in_,
                  std::shared_ptr<FieldInfo> fieldInfo,
                  std::shared_ptr<BlockTermState> _termState,
                  bool absolute)  override;

  std::shared_ptr<PostingsEnum>
  postings(std::shared_ptr<FieldInfo> fieldInfo,
           std::shared_ptr<BlockTermState> termState,
           std::shared_ptr<PostingsEnum> reuse,
           int flags)  override;

  int64_t ramBytesUsed() override;

  void checkIntegrity()  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<IDVersionPostingsReader> shared_from_this()
  {
    return std::static_pointer_cast<IDVersionPostingsReader>(
        org.apache.lucene.codecs.PostingsReaderBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/idversion/
