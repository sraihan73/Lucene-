#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class TermsEnum;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util::automaton
{
class CompiledAutomaton;
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
namespace org::apache::lucene::index
{

using BytesRef = org::apache::lucene::util::BytesRef;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

/**
 * Access to the terms in a specific field.  See {@link Fields}.
 * @lucene.experimental
 */

class Terms : public std::enable_shared_from_this<Terms>
{
  GET_CLASS_NAME(Terms)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  Terms();

  /** Returns an iterator that will step through all
   *  terms. This method will not return null. */
public:
  virtual std::shared_ptr<TermsEnum> iterator() = 0;

  /** Returns a TermsEnum that iterates over all terms and
   *  documents that are accepted by the provided {@link
   *  CompiledAutomaton}.  If the <code>startTerm</code> is
   *  provided then the returned enum will only return terms
   *  {@code > startTerm}, but you still must call
   *  next() first to get to the first term.  Note that the
   *  provided <code>startTerm</code> must be accepted by
   *  the automaton.
   *
   *  <p>This is an expert low-level API and will only work
   *  for {@code NORMAL} compiled automata.  To handle any
   *  compiled automata you should instead use
   *  {@link CompiledAutomaton#getTermsEnum} instead.
   *
   *  <p><b>NOTE</b>: the returned TermsEnum cannot seek</p>.
   *
   *  <p><b>NOTE</b>: the terms dictionary is free to
   *  return arbitrary terms as long as the resulted visited
   *  docs is the same.  E.g., {@link BlockTreeTermsWriter}
   *  creates auto-prefix terms during indexing to reduce the
   *  number of terms visited. */
  virtual std::shared_ptr<TermsEnum>
  intersect(std::shared_ptr<CompiledAutomaton> compiled,
            std::shared_ptr<BytesRef> startTerm) ;

private:
  class AutomatonTermsEnumAnonymousInnerClass : public AutomatonTermsEnum
  {
    GET_CLASS_NAME(AutomatonTermsEnumAnonymousInnerClass)
  private:
    std::shared_ptr<Terms> outerInstance;

    std::shared_ptr<BytesRef> startTerm;

  public:
    AutomatonTermsEnumAnonymousInnerClass(
        std::shared_ptr<Terms> outerInstance,
        std::shared_ptr<org::apache::lucene::index::TermsEnum> termsEnum,
        std::shared_ptr<CompiledAutomaton> compiled,
        std::shared_ptr<BytesRef> startTerm);

  protected:
    std::shared_ptr<BytesRef>
    nextSeekTerm(std::shared_ptr<BytesRef> term)  override;

  protected:
    std::shared_ptr<AutomatonTermsEnumAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AutomatonTermsEnumAnonymousInnerClass>(
          AutomatonTermsEnum::shared_from_this());
    }
  };

  /** Returns the number of terms for this field, or -1 if this
   *  measure isn't stored by the codec. Note that, just like
   *  other term measures, this measure does not take deleted
   *  documents into account. */
public:
  virtual int64_t size() = 0;

  /** Returns the sum of {@link TermsEnum#totalTermFreq} for
   *  all terms in this field, or -1 if this measure isn't
   *  stored by the codec (or if this fields omits term freq
   *  and positions).  Note that, just like other term
   *  measures, this measure does not take deleted documents
   *  into account. */
  virtual int64_t getSumTotalTermFreq() = 0;

  /** Returns the sum of {@link TermsEnum#docFreq()} for
   *  all terms in this field, or -1 if this measure isn't
   *  stored by the codec.  Note that, just like other term
   *  measures, this measure does not take deleted documents
   *  into account. */
  virtual int64_t getSumDocFreq() = 0;

  /** Returns the number of documents that have at least one
   *  term for this field, or -1 if this measure isn't
   *  stored by the codec.  Note that, just like other term
   *  measures, this measure does not take deleted documents
   *  into account. */
  virtual int getDocCount() = 0;

  /** Returns true if documents in this field store
   *  per-document term frequency ({@link PostingsEnum#freq}). */
  virtual bool hasFreqs() = 0;

  /** Returns true if documents in this field store offsets. */
  virtual bool hasOffsets() = 0;

  /** Returns true if documents in this field store positions. */
  virtual bool hasPositions() = 0;

  /** Returns true if documents in this field store payloads. */
  virtual bool hasPayloads() = 0;

  /** Zero-length array of {@link Terms}. */
  static std::deque<std::shared_ptr<Terms>> const EMPTY_ARRAY;

  /** Returns the smallest term (in lexicographic order) in the field.
   *  Note that, just like other term measures, this measure does not
   *  take deleted documents into account.  This returns
   *  null when there are no terms. */
  virtual std::shared_ptr<BytesRef> getMin() ;

  /** Returns the largest term (in lexicographic order) in the field.
   *  Note that, just like other term measures, this measure does not
   *  take deleted documents into account.  This returns
   *  null when there are no terms. */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("fallthrough") public
  // org.apache.lucene.util.BytesRef getMax() throws java.io.IOException
  virtual std::shared_ptr<BytesRef> getMax() ;

  /**
   * Expert: returns additional information about this Terms instance
   * for debugging purposes.
   */
  virtual std::any getStats() ;
};

} // namespace org::apache::lucene::index
