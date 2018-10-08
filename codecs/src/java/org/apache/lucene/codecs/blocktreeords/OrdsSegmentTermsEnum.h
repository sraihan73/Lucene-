#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
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

namespace org::apache::lucene::codecs::blocktreeords
{
class OrdsSegmentTermsEnumFrame;
}
namespace org::apache::lucene::codecs::blocktreeords
{
class OrdsFieldReader;
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
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::index
{
class TermState;
}
namespace org::apache::lucene::util
{
class IntsRef;
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

// import java.io.*;
// import java.nio.charset.StandardCharsets;

using Output =
    org::apache::lucene::codecs::blocktreeords::FSTOrdsOutputs::Output;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using TermState = org::apache::lucene::index::TermState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRef = org::apache::lucene::util::IntsRef;
using FST = org::apache::lucene::util::fst::FST;

/** Iterates through terms in this field. */
class OrdsSegmentTermsEnum final : public TermsEnum
{
  GET_CLASS_NAME(OrdsSegmentTermsEnum)

  // Lazy init:
public:
  std::shared_ptr<IndexInput> in_;

  // static bool DEBUG = true;

private:
  std::deque<std::shared_ptr<OrdsSegmentTermsEnumFrame>> stack;
  const std::shared_ptr<OrdsSegmentTermsEnumFrame> staticFrame;

public:
  std::shared_ptr<OrdsSegmentTermsEnumFrame> currentFrame;
  bool termExists = false;
  const std::shared_ptr<OrdsFieldReader> fr;

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
  // org.apache.lucene.util.fst.FST.Arc<org.apache.lucene.codecs.blocktreeords.FSTOrdsOutputs.Output>[]
  // arcs = new org.apache.lucene.util.fst.FST.Arc[1];
  std::deque<FST::Arc<std::shared_ptr<Output>>> arcs =
      std::deque<std::shared_ptr<FST::Arc>>(1);

public:
  bool positioned = false;

  OrdsSegmentTermsEnum(std::shared_ptr<OrdsFieldReader> fr) ;

  // Not private to avoid synthetic access$NNN methods
  void initIndexInput();

private:
  std::shared_ptr<OrdsSegmentTermsEnumFrame>
  getFrame(int ord) ;

  std::shared_ptr<FST::Arc<std::shared_ptr<Output>>> getArc(int ord);

  // Pushes a frame we seek'd to
public:
  std::shared_ptr<OrdsSegmentTermsEnumFrame>
  pushFrame(std::shared_ptr<FST::Arc<std::shared_ptr<Output>>> arc,
            std::shared_ptr<Output> frameData, int length) ;

  // Pushes next'd frame or seek'd frame; we later
  // lazy-load the frame only when needed
  std::shared_ptr<OrdsSegmentTermsEnumFrame>
  pushFrame(std::shared_ptr<FST::Arc<std::shared_ptr<Output>>> arc,
            int64_t fp, int length, int64_t termOrd) ;

  // asserts only
private:
  bool clearEOF();

  // asserts only
  bool setEOF();

  // for debugging
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unused") static std::wstring
  // brToString(org.apache.lucene.util.BytesRef b)
  static std::wstring brToString(std::shared_ptr<BytesRef> b);

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

  int64_t ord() override;

  int docFreq()  override;

  int64_t totalTermFreq()  override;

  std::shared_ptr<PostingsEnum> postings(std::shared_ptr<PostingsEnum> reuse,
                                         int flags)  override;

  void seekExact(std::shared_ptr<BytesRef> target,
                 std::shared_ptr<TermState> otherState) override;

  std::shared_ptr<TermState> termState()  override;

  void seekExact(int64_t targetOrd)  override;

  virtual std::wstring toString();

  /** Holds a single input (IntsRef) + output pair. */
private:
  class InputOutput : public std::enable_shared_from_this<InputOutput>
  {
    GET_CLASS_NAME(InputOutput)
  public:
    std::shared_ptr<IntsRef> input;
    std::shared_ptr<Output> output;

    virtual std::wstring toString();
  };

private:
  const std::shared_ptr<FST::Arc<std::shared_ptr<Output>>> arc =
      std::make_shared<FST::Arc<std::shared_ptr<Output>>>();

  // TODO: this is similar to Util.getByOutput ... can we refactor/share?
  /** Specialized getByOutput that can understand the ranges (startOrd to
   * endOrd) we use here, not just startOrd. */
  std::shared_ptr<InputOutput>
  getByOutput(int64_t targetOrd) ;

protected:
  std::shared_ptr<OrdsSegmentTermsEnum> shared_from_this()
  {
    return std::static_pointer_cast<OrdsSegmentTermsEnum>(
        org.apache.lucene.index.TermsEnum::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blocktreeords
