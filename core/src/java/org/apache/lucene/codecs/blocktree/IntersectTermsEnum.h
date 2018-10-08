#pragma once
#include "../../index/TermsEnum.h"
#include "../../util/fst/Outputs.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexInput;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::codecs::blocktree
{
class IntersectTermsEnumFrame;
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
class Automaton;
}
namespace org::apache::lucene::util::automaton
{
class Transition;
}
namespace org::apache::lucene::util::fst
{
class BytesReader;
}
namespace org::apache::lucene::codecs::blocktree
{
class FieldReader;
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
namespace org::apache::lucene::codecs::blocktree
{

using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using TermState = org::apache::lucene::index::TermState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexInput = org::apache::lucene::store::IndexInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using RunAutomaton = org::apache::lucene::util::automaton::RunAutomaton;
using Transition = org::apache::lucene::util::automaton::Transition;
using ByteSequenceOutputs = org::apache::lucene::util::fst::ByteSequenceOutputs;
using FST = org::apache::lucene::util::fst::FST;
using Outputs = org::apache::lucene::util::fst::Outputs;

/** This is used to implement efficient {@link Terms#intersect} for
 *  block-tree.  Note that it cannot seek, except for the initial term on
 *  init.  It just "nexts" through the intersection of the automaton and
 *  the terms.  It does not use the terms index at all: on init, it
 *  loads the root block, and scans its way to the initial term.
 *  Likewise, in next it scans until it finds a term that matches the
 *  current automaton transition.  If the index has auto-prefix terms
 *  (only for DOCS_ONLY fields currently) it will visit these terms
 *  when possible and then skip the real terms that auto-prefix term
 *  matched. */

class IntersectTermsEnum final : public TermsEnum
{
  GET_CLASS_NAME(IntersectTermsEnum)

  // static bool DEBUG = BlockTreeTermsWriter.DEBUG;

public:
  const std::shared_ptr<IndexInput> in_;
  static const std::shared_ptr<Outputs<std::shared_ptr<BytesRef>>> fstOutputs;

  std::deque<std::shared_ptr<IntersectTermsEnumFrame>> stack;

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) private
  // org.apache.lucene.util.fst.FST.Arc<org.apache.lucene.util.BytesRef>[] arcs =
  // new org.apache.lucene.util.fst.FST.Arc[5];
  std::deque<FST::Arc<std::shared_ptr<BytesRef>>> arcs =
      std::deque<std::shared_ptr<FST::Arc>>(5);

public:
  const std::shared_ptr<RunAutomaton> runAutomaton;
  const std::shared_ptr<Automaton> automaton;
  const std::shared_ptr<BytesRef> commonSuffix;

private:
  std::shared_ptr<IntersectTermsEnumFrame> currentFrame;
  std::shared_ptr<Transition> currentTransition;

  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<BytesRef> term_ = std::make_shared<BytesRef>();

  const std::shared_ptr<FST::BytesReader> fstReader;

  const bool allowAutoPrefixTerms;

public:
  const std::shared_ptr<FieldReader> fr;

  /** Which state in the automaton accepts all possible suffixes. */
private:
  const int sinkState;

  std::shared_ptr<BytesRef> savedStartTerm;

  /** True if we did return the current auto-prefix term */
  bool useAutoPrefixTerm = false;

  // TODO: in some cases we can filter by length?  eg
  // regexp foo*bar must be at least length 6 bytes
public:
  IntersectTermsEnum(std::shared_ptr<FieldReader> fr,
                     std::shared_ptr<Automaton> automaton,
                     std::shared_ptr<RunAutomaton> runAutomaton,
                     std::shared_ptr<BytesRef> commonSuffix,
                     std::shared_ptr<BytesRef> startTerm,
                     int sinkState) ;

  // only for assert:
private:
  bool setSavedStartTerm(std::shared_ptr<BytesRef> startTerm);

public:
  std::shared_ptr<TermState> termState()  override;

private:
  std::shared_ptr<IntersectTermsEnumFrame> getFrame(int ord) ;

  std::shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> getArc(int ord);

  std::shared_ptr<IntersectTermsEnumFrame>
  pushFrame(int state) ;

public:
  std::shared_ptr<BytesRef> term() override;

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

  bool popPushNext() ;

  bool skipPastLastAutoPrefixTerm() ;

  // Only used internally when there are no more terms in next():
private:
  class NoMoreTermsException final : public std::runtime_error
  {
    GET_CLASS_NAME(NoMoreTermsException)

    // Only used internally when there are no more terms in next():
  public:
    static const std::shared_ptr<NoMoreTermsException> INSTANCE;

  private:
    NoMoreTermsException();

  public:
    std::runtime_error fillInStackTrace() override;

  protected:
    std::shared_ptr<NoMoreTermsException> shared_from_this()
    {
      return std::static_pointer_cast<NoMoreTermsException>(
          RuntimeException::shared_from_this());
    }
  };

public:
  std::shared_ptr<BytesRef> next()  override;

private:
  std::shared_ptr<BytesRef> _next() ;

  const std::shared_ptr<Transition> scratchTransition =
      std::make_shared<Transition>();

  /** Returns true if, from this state, the automaton accepts any suffix
   *  starting with a label between start and end, inclusive.  We just
   *  look for a transition, matching this range, to the sink state.  */
  bool acceptsSuffixRange(int state, int start, int end);

  // for debugging
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unused") static std::wstring
  // brToString(org.apache.lucene.util.BytesRef b)
  static std::wstring brToString(std::shared_ptr<BytesRef> b);

private:
  void copyTerm();

public:
  bool seekExact(std::shared_ptr<BytesRef> text) override;

  void seekExact(int64_t ord) override;

  int64_t ord() override;

  SeekStatus seekCeil(std::shared_ptr<BytesRef> text) override;

protected:
  std::shared_ptr<IntersectTermsEnum> shared_from_this()
  {
    return std::static_pointer_cast<IntersectTermsEnum>(
        org.apache.lucene.index.TermsEnum::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blocktree
