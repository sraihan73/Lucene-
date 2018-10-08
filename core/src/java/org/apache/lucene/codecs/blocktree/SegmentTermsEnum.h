#pragma once
#include "../../index/TermsEnum.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexInput;
}

namespace org::apache::lucene::codecs::blocktree
{
class SegmentTermsEnumFrame;
}
namespace org::apache::lucene::codecs::blocktree
{
class FieldReader;
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
namespace org::apache::lucene::codecs::blocktree
{
class Stats;
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
namespace org::apache::lucene::codecs::blocktree
{

using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using TermState = org::apache::lucene::index::TermState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using FST = org::apache::lucene::util::fst::FST;

/** Iterates through terms in this field.  This implementation skips
 *  any auto-prefix terms it encounters. */

class SegmentTermsEnum final : public TermsEnum
{
  GET_CLASS_NAME(SegmentTermsEnum)

  // Lazy init:
public:
  std::shared_ptr<IndexInput> in_;

private:
  std::deque<std::shared_ptr<SegmentTermsEnumFrame>> stack;
  const std::shared_ptr<SegmentTermsEnumFrame> staticFrame;

public:
  std::shared_ptr<SegmentTermsEnumFrame> currentFrame;
  bool termExists = false;
  const std::shared_ptr<FieldReader> fr;

private:
  int targetBeforeCurrentLength = 0;

  // static bool DEBUG = BlockTreeTermsWriter.DEBUG;

  const std::shared_ptr<ByteArrayDataInput> scratchReader =
      std::make_shared<ByteArrayDataInput>();

  // What prefix of the current term was present in the index; when we only
  // next() through the index, this stays at 0.  It's only set when we
  // seekCeil/Exact:
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
  // org.apache.lucene.util.fst.FST.Arc<org.apache.lucene.util.BytesRef>[] arcs =
  // new org.apache.lucene.util.fst.FST.Arc[1];
  std::deque<FST::Arc<std::shared_ptr<BytesRef>>> arcs =
      std::deque<std::shared_ptr<FST::Arc>>(1);

public:
  SegmentTermsEnum(std::shared_ptr<FieldReader> fr) ;

  // Not private to avoid synthetic access$NNN methods
  void initIndexInput();

  /** Runs next() through the entire terms dict,
   *  computing aggregate statistics. */
  std::shared_ptr<Stats> computeBlockStats() ;

private:
  std::shared_ptr<SegmentTermsEnumFrame> getFrame(int ord) ;

  std::shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> getArc(int ord);

  // Pushes a frame we seek'd to
public:
  std::shared_ptr<SegmentTermsEnumFrame>
  pushFrame(std::shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> arc,
            std::shared_ptr<BytesRef> frameData, int length) ;

  // Pushes next'd frame or seek'd frame; we later
  // lazy-load the frame only when needed
  std::shared_ptr<SegmentTermsEnumFrame>
  pushFrame(std::shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> arc,
            int64_t fp, int length) ;

  // asserts only
private:
  bool clearEOF();

  // asserts only
  bool setEOF();

  /*
  // for debugging
  @SuppressWarnings("unused")
  static std::wstring brToString(BytesRef b) {
    try {
      return b.utf8ToString() + " " + b;
    } catch (Throwable t) {
      // If BytesRef isn't actually UTF8, or it's eg a
      // prefix of UTF8 that ends mid-unicode-char, we
      // fallback to hex:
      return b.toString();
    }
  }

  // for debugging
  @SuppressWarnings("unused")
  static std::wstring brToString(BytesRefBuilder b) {
    return brToString(b.get());
  }
  */

public:
  bool seekExact(std::shared_ptr<BytesRef> target)  override;

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

protected:
  std::shared_ptr<SegmentTermsEnum> shared_from_this()
  {
    return std::static_pointer_cast<SegmentTermsEnum>(
        org.apache.lucene.index.TermsEnum::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blocktree
