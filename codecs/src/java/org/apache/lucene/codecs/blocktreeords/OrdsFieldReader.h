#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class FieldInfo;
}

namespace org::apache::lucene::codecs::blocktreeords
{
class FSTOrdsOutputs;
}
namespace org::apache::lucene::codecs::blocktreeords
{
class Output;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::codecs::blocktreeords
{
class OrdsBlockTreeTermsReader;
}
namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::util::automaton
{
class CompiledAutomaton;
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
namespace org::apache::lucene::codecs::blocktreeords
{

using Output =
    org::apache::lucene::codecs::blocktreeords::FSTOrdsOutputs::Output;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using FST = org::apache::lucene::util::fst::FST;

/** BlockTree's implementation of {@link Terms}. */
class OrdsFieldReader final : public Terms, public Accountable
{
  GET_CLASS_NAME(OrdsFieldReader)
public:
  const int64_t numTerms;
  const std::shared_ptr<FieldInfo> fieldInfo;
  const int64_t sumTotalTermFreq;
  const int64_t sumDocFreq;
  const int docCount;
  const int64_t indexStartFP;
  const int64_t rootBlockFP;
  const std::shared_ptr<Output> rootCode;
  const std::shared_ptr<BytesRef> minTerm;
  const std::shared_ptr<BytesRef> maxTerm;
  const int longsSize;
  const std::shared_ptr<OrdsBlockTreeTermsReader> parent;

  const std::shared_ptr<FST<std::shared_ptr<Output>>> index;
  // private bool DEBUG;

  OrdsFieldReader(std::shared_ptr<OrdsBlockTreeTermsReader> parent,
                  std::shared_ptr<FieldInfo> fieldInfo, int64_t numTerms,
                  std::shared_ptr<Output> rootCode, int64_t sumTotalTermFreq,
                  int64_t sumDocFreq, int docCount, int64_t indexStartFP,
                  int longsSize, std::shared_ptr<IndexInput> indexIn,
                  std::shared_ptr<BytesRef> minTerm,
                  std::shared_ptr<BytesRef> maxTerm) ;

  std::shared_ptr<BytesRef> getMin()  override;

  std::shared_ptr<BytesRef> getMax()  override;

  bool hasFreqs() override;

  bool hasOffsets() override;

  bool hasPositions() override;

  bool hasPayloads() override;

  std::shared_ptr<TermsEnum> iterator()  override;

  int64_t size() override;

  int64_t getSumTotalTermFreq() override;

  int64_t getSumDocFreq() override;

  int getDocCount() override;

  std::shared_ptr<TermsEnum>
  intersect(std::shared_ptr<CompiledAutomaton> compiled,
            std::shared_ptr<BytesRef> startTerm)  override;

  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<OrdsFieldReader> shared_from_this()
  {
    return std::static_pointer_cast<OrdsFieldReader>(
        org.apache.lucene.index.Terms::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blocktreeords
