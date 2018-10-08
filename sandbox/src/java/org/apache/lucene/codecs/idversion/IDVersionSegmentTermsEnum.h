#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexInput;
}

namespace org::apache::lucene::codecs::idversion
{
class IDVersionSegmentTermsEnumFrame;
}
namespace org::apache::lucene::codecs::idversion
{
class VersionFieldReader;
}
namespace org::apache::lucene::store
{
class ByteArrayDataInput;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
namespace org::apache::lucene::util::fst
{
class BytesReader;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class Arc;
}
namespace org::apache::lucene::util::fst
{
template <typename Atypename B>
class PairOutputs;
}
namespace org::apache::lucene::util::fst
{
template <typename Atypename B>
class Pair;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::index
{
class TermState;
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

using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using TermState = org::apache::lucene::index::TermState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::PairOutputs::Pair;

/** Iterates through terms in this field; this class is public so users
 *  can cast it to call {@link #seekExact(BytesRef, long)} for
GET_CLASS_NAME(is)
 *  optimistic-concurreny, and also {@link #getVersion} to get the
 *  version of the currently seek'd term. */
class IDVersionSegmentTermsEnum final : public TermsEnum
{
  GET_CLASS_NAME(IDVersionSegmentTermsEnum)

  // Lazy init:
public:
  std::shared_ptr<IndexInput> in_;

  // static bool DEBUG = false;

private:
  std::deque<std::shared_ptr<IDVersionSegmentTermsEnumFrame>> stack;
  const std::shared_ptr<IDVersionSegmentTermsEnumFrame> staticFrame;

public:
  std::shared_ptr<IDVersionSegmentTermsEnumFrame> currentFrame;
  bool termExists = false;
  const std::shared_ptr<VersionFieldReader> fr;

private:
  int targetBeforeCurrentLength = 0;

  const std::shared_ptr<ByteArrayDataInput> scratchReader =
      std::make_shared<ByteArrayDataInput>();

  // What prefix of the current term was present in the index:
  int validIndexPrefix = 0;

  // assert only:
  bool eof = false;

public:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<BytesRefBuilder> term_ =
      std::make_shared<BytesRefBuilder>();

private:
  const std::shared_ptr<FST::BytesReader> fstReader;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) private
  // org.apache.lucene.util.fst.FST.Arc<org.apache.lucene.util.fst.PairOutputs.Pair<org.apache.lucene.util.BytesRef,long>>[]
  // arcs = new org.apache.lucene.util.fst.FST.Arc[1];
  std::deque<FST::Arc<Pair<std::shared_ptr<BytesRef>, int64_t>>> arcs =
      std::deque<std::shared_ptr<FST::Arc>>(1);

public:
  IDVersionSegmentTermsEnum(std::shared_ptr<VersionFieldReader> fr) throw(
      IOException);

  // Not private to avoid synthetic access$NNN methods
  void initIndexInput();

private:
  std::shared_ptr<IDVersionSegmentTermsEnumFrame>
  getFrame(int ord) ;

  std::shared_ptr<FST::Arc<Pair<std::shared_ptr<BytesRef>, int64_t>>>
  getArc(int ord);

  // Pushes a frame we seek'd to
public:
  std::shared_ptr<IDVersionSegmentTermsEnumFrame> pushFrame(
      std::shared_ptr<FST::Arc<Pair<std::shared_ptr<BytesRef>, int64_t>>> arc,
      std::shared_ptr<Pair<std::shared_ptr<BytesRef>, int64_t>> frameData,
      int length) ;

  // Pushes next'd frame or seek'd frame; we later
  // lazy-load the frame only when needed
  std::shared_ptr<IDVersionSegmentTermsEnumFrame> pushFrame(
      std::shared_ptr<FST::Arc<Pair<std::shared_ptr<BytesRef>, int64_t>>> arc,
      int64_t fp, int length) ;

  // asserts only
private:
  bool clearEOF();

  // asserts only
  bool setEOF();

public:
  bool seekExact(std::shared_ptr<BytesRef> target)  override;

  // for debugging
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unused") static std::wstring
  // brToString(org.apache.lucene.util.BytesRef b)
  static std::wstring brToString(std::shared_ptr<BytesRef> b);

  /** Get the version of the currently seek'd term; only valid if we are
   *  positioned. */
  int64_t getVersion();

  /** Optimized version of {@link #seekExact(BytesRef)} that can
   *  sometimes fail-fast if the version indexed with the requested ID
   *  is less than the specified minIDVersion.  Applications that index
   *  a monotonically increasing global version with each document can
   *  use this for fast optimistic concurrency. */
  bool seekExact(std::shared_ptr<BytesRef> target,
                 int64_t minIDVersion) ;

  SeekStatus
  seekCeil(std::shared_ptr<BytesRef> target)  override;

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unused") private void
  // printSeekState(java.io.PrintStream out) throws java.io.IOException
  void printSeekState(std::shared_ptr<PrintStream> out) ;

  /* Decodes only the term bytes of the next term.  If caller then asks for
     metadata, ie docFreq, totalTermFreq or pulls a D/&PEnum, we then (lazily)
     decode all metadata up to the current term. */
public:
  std::shared_ptr<BytesRef> next()  override;

  std::shared_ptr<BytesRef> term() override;

  int docFreq()  override;

  int64_t totalTermFreq()  override;

  std::shared_ptr<PostingsEnum> postings(std::shared_ptr<PostingsEnum> reuse,
                                         int flags)  override;

  void seekExact(std::shared_ptr<BytesRef> target,
                 std::shared_ptr<TermState> otherState) override;

  std::shared_ptr<TermState> termState()  override;

  void seekExact(int64_t ord) override;

  int64_t ord() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<IDVersionSegmentTermsEnum> shared_from_this()
  {
    return std::static_pointer_cast<IDVersionSegmentTermsEnum>(
        org.apache.lucene.index.TermsEnum::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::idversion
