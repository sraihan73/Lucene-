#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexInput;
}

namespace org::apache::lucene::codecs::blocktreeords
{
class OrdsIntersectTermsEnumFrame;
}
namespace org::apache::lucene::codecs::blocktreeords
{
class FSTOrdsOutputs;
}
namespace org::apache::lucene::codecs::blocktreeords
{
class Output;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class Arc;
}
namespace org::apache::lucene::util::automaton
{
class RunAutomaton;
}
namespace org::apache::lucene::util::automaton
{
class CompiledAutomaton;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util::fst
{
class BytesReader;
}
namespace org::apache::lucene::codecs::blocktreeords
{
class OrdsFieldReader;
}
namespace org::apache::lucene::index
{
class TermState;
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
namespace org::apache::lucene::codecs::blocktreeords
{

using Output =
    org::apache::lucene::codecs::blocktreeords::FSTOrdsOutputs::Output;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using TermState = org::apache::lucene::index::TermState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexInput = org::apache::lucene::store::IndexInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using RunAutomaton = org::apache::lucene::util::automaton::RunAutomaton;
using FST = org::apache::lucene::util::fst::FST;

// NOTE: cannot seek!
class OrdsIntersectTermsEnum final : public TermsEnum
{
  GET_CLASS_NAME(OrdsIntersectTermsEnum)
public:
  const std::shared_ptr<IndexInput> in_;

private:
  std::deque<std::shared_ptr<OrdsIntersectTermsEnumFrame>> stack;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) private
  // org.apache.lucene.util.fst.FST.Arc<org.apache.lucene.codecs.blocktreeords.FSTOrdsOutputs.Output>[]
  // arcs = new org.apache.lucene.util.fst.FST.Arc[5];
  std::deque<FST::Arc<std::shared_ptr<Output>>> arcs =
      std::deque<std::shared_ptr<FST::Arc>>(5);

public:
  const std::shared_ptr<RunAutomaton> runAutomaton;
  const std::shared_ptr<CompiledAutomaton> compiledAutomaton;

private:
  std::shared_ptr<OrdsIntersectTermsEnumFrame> currentFrame;

  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<BytesRef> term_ = std::make_shared<BytesRef>();

  const std::shared_ptr<FST::BytesReader> fstReader;

public:
  const std::shared_ptr<OrdsFieldReader> fr;

private:
  std::shared_ptr<BytesRef> savedStartTerm;

  // TODO: in some cases we can filter by length?  eg
  // regexp foo*bar must be at least length 6 bytes
public:
  OrdsIntersectTermsEnum(
      std::shared_ptr<OrdsFieldReader> fr,
      std::shared_ptr<CompiledAutomaton> compiled,
      std::shared_ptr<BytesRef> startTerm) ;

  // only for assert:
private:
  bool setSavedStartTerm(std::shared_ptr<BytesRef> startTerm);

public:
  std::shared_ptr<TermState> termState()  override;

private:
  std::shared_ptr<OrdsIntersectTermsEnumFrame>
  getFrame(int ord) ;

  std::shared_ptr<FST::Arc<std::shared_ptr<Output>>> getArc(int ord);

  std::shared_ptr<OrdsIntersectTermsEnumFrame>
  pushFrame(int state) ;

public:
  std::shared_ptr<BytesRef> term() override;

  // TODO: do we need ord() here?  OrdsIntersectTermsEnumFrame tracks termOrd
  // but it may be buggy!

  int docFreq()  override;

  int64_t totalTermFreq()  override;

  std::shared_ptr<PostingsEnum> postings(std::shared_ptr<PostingsEnum> reuse,
                                         int flags)  override;

private:
  int getState();

  // NOTE: specialized to only doing the first-time
  // seek, but we could generalize it to allow
  // arbitrary seekExact/Ceil.  Note that this is a
  // seekFloor!
  void seekToStartTerm(std::shared_ptr<BytesRef> target) ;

public:
  std::shared_ptr<BytesRef> next()  override;

private:
  void copyTerm();

public:
  bool seekExact(std::shared_ptr<BytesRef> text) override;

  void seekExact(int64_t ord) override;

  int64_t ord() override;

  SeekStatus seekCeil(std::shared_ptr<BytesRef> text) override;

protected:
  std::shared_ptr<OrdsIntersectTermsEnum> shared_from_this()
  {
    return std::static_pointer_cast<OrdsIntersectTermsEnum>(
        org.apache.lucene.index.TermsEnum::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blocktreeords
