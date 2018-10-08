#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/BytesRefFSTEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "TermsIndexReaderBase.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::fst
{
class PositiveIntOutputs;
}

namespace org::apache::lucene::codecs::blockterms
{
class FieldIndexData;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class BytesRefFSTEnum;
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
class IndexInput;
}
namespace org::apache::lucene::util
{
class Accountable;
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
using BytesRefFSTEnum = org::apache::lucene::util::fst::BytesRefFSTEnum;
using FST = org::apache::lucene::util::fst::FST;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;

/** See {@link VariableGapTermsIndexWriter}
 *
 * @lucene.experimental */
class VariableGapTermsIndexReader : public TermsIndexReaderBase
{
  GET_CLASS_NAME(VariableGapTermsIndexReader)

private:
  const std::shared_ptr<PositiveIntOutputs> fstOutputs =
      PositiveIntOutputs::getSingleton();

public:
  const std::unordered_map<std::wstring, std::shared_ptr<FieldIndexData>>
      fields =
          std::unordered_map<std::wstring, std::shared_ptr<FieldIndexData>>();

  VariableGapTermsIndexReader(std::shared_ptr<SegmentReadState> state) throw(
      IOException);

private:
  class IndexEnum : public FieldIndexEnum
  {
    GET_CLASS_NAME(IndexEnum)
  private:
    const std::shared_ptr<BytesRefFSTEnum<int64_t>> fstEnum;
    std::shared_ptr<BytesRefFSTEnum::InputOutput<int64_t>> current;

  public:
    IndexEnum(std::shared_ptr<FST<int64_t>> fst);

    std::shared_ptr<BytesRef> term() override;

    int64_t
    seek(std::shared_ptr<BytesRef> target)  override;

    int64_t next()  override;

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
    std::shared_ptr<VariableGapTermsIndexReader> outerInstance;

    const std::shared_ptr<FST<int64_t>> fst;

  public:
    FieldIndexData(std::shared_ptr<VariableGapTermsIndexReader> outerInstance,
                   std::shared_ptr<IndexInput> in_,
                   std::shared_ptr<FieldInfo> fieldInfo,
                   int64_t indexStart) ;

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    virtual std::wstring toString();
  };

public:
  std::shared_ptr<FieldIndexEnum>
  getFieldEnum(std::shared_ptr<FieldInfo> fieldInfo) override;

  virtual ~VariableGapTermsIndexReader();

private:
  void seekDir(std::shared_ptr<IndexInput> input) ;

public:
  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<VariableGapTermsIndexReader> shared_from_this()
  {
    return std::static_pointer_cast<VariableGapTermsIndexReader>(
        TermsIndexReaderBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blockterms
