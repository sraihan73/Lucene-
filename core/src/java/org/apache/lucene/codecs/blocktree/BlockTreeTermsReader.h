#pragma once
#include "../../util/fst/Outputs.h"
#include "../FieldsProducer.h"
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::codecs
{
class PostingsReaderBase;
}
namespace org::apache::lucene::codecs::blocktree
{
class FieldReader;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::index
{
class Terms;
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
namespace org::apache::lucene::codecs::blocktree
{

using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using Terms = org::apache::lucene::index::Terms;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using ByteSequenceOutputs = org::apache::lucene::util::fst::ByteSequenceOutputs;
using Outputs = org::apache::lucene::util::fst::Outputs;

/** A block-based terms index and dictionary that assigns
 *  terms to variable length blocks according to how they
 *  share prefixes.  The terms index is a prefix trie
 *  whose leaves are term blocks.  The advantage of this
 *  approach is that seekExact is often able to
 *  determine a term cannot exist without doing any IO, and
 *  intersection with Automata is very fast.  Note that this
 *  terms dictionary has its own fixed terms index (ie, it
 *  does not support a pluggable terms index
 *  implementation).
 *
 *  <p><b>NOTE</b>: this terms dictionary supports
 *  min/maxItemsPerBlock during indexing to control how
 *  much memory the terms index uses.</p>
 *
 *  <p>If auto-prefix terms were indexed (see
 *  {@link BlockTreeTermsWriter}), then the {@link Terms#intersect}
 *  implementation here will make use of these terms only if the
 *  automaton has a binary sink state, i.e. an accept state
 *  which has a transition to itself accepting all byte values.
 *  For example, both {@link PrefixQuery} and {@link TermRangeQuery}
 *  pass such automata to {@link Terms#intersect}.</p>
 *
 *  <p>The data structure used by this implementation is very
 *  similar to a burst trie
 *  (http://citeseer.ist.psu.edu/viewdoc/summary?doi=10.1.1.18.3499),
 *  but with added logic to break up too-large blocks of all
 *  terms sharing a given prefix into smaller ones.</p>
 *
 *  <p>Use {@link org.apache.lucene.index.CheckIndex} with the
 * <code>-verbose</code> option to see summary statistics on the blocks in the
 *  dictionary.
 *
 *  See {@link BlockTreeTermsWriter}.
 *
 * @lucene.experimental
 */

class BlockTreeTermsReader final : public FieldsProducer
{
  GET_CLASS_NAME(BlockTreeTermsReader)

public:
  static const std::shared_ptr<Outputs<std::shared_ptr<BytesRef>>> FST_OUTPUTS;

  static const std::shared_ptr<BytesRef> NO_OUTPUT;

  static constexpr int OUTPUT_FLAGS_NUM_BITS = 2;
  static constexpr int OUTPUT_FLAGS_MASK = 0x3;
  static constexpr int OUTPUT_FLAG_IS_FLOOR = 0x1;
  static constexpr int OUTPUT_FLAG_HAS_TERMS = 0x2;

  /** Extension of terms file */
  static const std::wstring TERMS_EXTENSION;
  static const std::wstring TERMS_CODEC_NAME;

  /** Initial terms format. */
  static constexpr int VERSION_START = 2;

  /** Auto-prefix terms have been superseded by points. */
  static constexpr int VERSION_AUTO_PREFIX_TERMS_REMOVED = 3;

  /** Current terms format. */
  static constexpr int VERSION_CURRENT = VERSION_AUTO_PREFIX_TERMS_REMOVED;

  /** Extension of terms index file */
  static const std::wstring TERMS_INDEX_EXTENSION;
  static const std::wstring TERMS_INDEX_CODEC_NAME;

  // Open input to the main terms dict file (_X.tib)
  const std::shared_ptr<IndexInput> termsIn;

  // private static final bool DEBUG = BlockTreeTermsWriter.DEBUG;

  // Reads the terms dict entries, to gather state to
  // produce DocsEnum on demand
  const std::shared_ptr<PostingsReaderBase> postingsReader;

private:
  const std::map_obj<std::wstring, std::shared_ptr<FieldReader>> fields =
      std::map_obj<std::wstring, std::shared_ptr<FieldReader>>();

public:
  const std::wstring segment;

  const int version;

  /** Sole constructor. */
  BlockTreeTermsReader(
      std::shared_ptr<PostingsReaderBase> postingsReader,
      std::shared_ptr<SegmentReadState> state) ;

private:
  static std::shared_ptr<BytesRef>
  readBytesRef(std::shared_ptr<IndexInput> in_) ;

  /** Seek {@code input} to the directory offset. */
  static void seekDir(std::shared_ptr<IndexInput> input) ;

  // for debugging
  // private static std::wstring toHex(int v) {
  //   return "0x" + Integer.toHexString(v);
  // }

public:
  virtual ~BlockTreeTermsReader();

  std::shared_ptr<Iterator<std::wstring>> iterator() override;

  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

  int size() override;

  // for debugging
  std::wstring brToString(std::shared_ptr<BytesRef> b);

  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  void checkIntegrity()  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<BlockTreeTermsReader> shared_from_this()
  {
    return std::static_pointer_cast<BlockTreeTermsReader>(
        org.apache.lucene.codecs.FieldsProducer::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blocktree
