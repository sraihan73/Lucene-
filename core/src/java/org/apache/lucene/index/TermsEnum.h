#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefIterator.h"
#include  "core/src/java/org/apache/lucene/index/TermState.h"
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
namespace org::apache::lucene::index
{
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;

/** Iterator to seek ({@link #seekCeil(BytesRef)}, {@link
 * #seekExact(BytesRef)}) or step through ({@link
 * #next} terms to obtain frequency information ({@link
 * #docFreq}), {@link PostingsEnum} or {@link
 * PostingsEnum} for the current term ({@link
 * #postings}.
 *
 * <p>Term enumerations are always ordered by
 * BytesRef.compareTo, which is Unicode sort
 * order if the terms are UTF-8 bytes.  Each term in the
 * enumeration is greater than the one before it.</p>
 *
 * <p>The TermsEnum is unpositioned when you first obtain it
 * and you must first successfully call {@link #next} or one
 * of the <code>seek</code> methods.
 *
 * @lucene.experimental */
class TermsEnum : public std::enable_shared_from_this<TermsEnum>,
                  public BytesRefIterator
{
  GET_CLASS_NAME(TermsEnum)

private:
  std::shared_ptr<AttributeSource> atts = nullptr;

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  TermsEnum();

  /** Returns the related attributes. */
public:
  virtual std::shared_ptr<AttributeSource> attributes();

  /** Represents returned result from {@link #seekCeil}. */
public:
  enum class SeekStatus {
    GET_CLASS_NAME(SeekStatus)
    /** The term was not found, and the end of iteration was hit. */
    END,
    /** The precise term was found. */
    FOUND,
    /** A different term was found after the requested term */
    NOT_FOUND
  };

  /** Attempts to seek to the exact term, returning
   *  true if the term is found.  If this returns false, the
   *  enum is unpositioned.  For some codecs, seekExact may
   *  be substantially faster than {@link #seekCeil}. */
public:
  virtual bool seekExact(std::shared_ptr<BytesRef> text) ;

  /** Seeks to the specified term, if it exists, or to the
   *  next (ceiling) term.  Returns SeekStatus to
   *  indicate whether exact term was found, a different
   *  term was found, or EOF was hit.  The target term may
   *  be before or after the current term.  If this returns
   *  SeekStatus.END, the enum is unpositioned. */
  virtual SeekStatus seekCeil(std::shared_ptr<BytesRef> text) = 0;

  /** Seeks to the specified term by ordinal (position) as
   *  previously returned by {@link #ord}.  The target ord
   *  may be before or after the current ord, and must be
   *  within bounds. */
  virtual void seekExact(int64_t ord) = 0;

  /**
   * Expert: Seeks a specific position by {@link TermState} previously obtained
   * from {@link #termState()}. Callers should maintain the {@link TermState} to
   * use this method. Low-level implementations may position the TermsEnum
   * without re-seeking the term dictionary.
   * <p>
   * Seeking by {@link TermState} should only be used iff the state was obtained
   * from the same {@link TermsEnum} instance.
   * <p>
   * NOTE: Using this method with an incompatible {@link TermState} might leave
   * this {@link TermsEnum} in undefined state. On a segment level
   * {@link TermState} instances are compatible only iff the source and the
   * target {@link TermsEnum} operate on the same field. If operating on segment
   * level, TermState instances must not be used across segments.
   * <p>
   * NOTE: A seek by {@link TermState} might not restore the
   * {@link AttributeSource}'s state. {@link AttributeSource} states must be
   * maintained separately if this method is used.
   * @param term the term the TermState corresponds to
   * @param state the {@link TermState}
   * */
  virtual void seekExact(std::shared_ptr<BytesRef> term,
                         std::shared_ptr<TermState> state) ;

  /** Returns current term. Do not call this when the enum
   *  is unpositioned. */
  virtual std::shared_ptr<BytesRef> term() = 0;

  /** Returns ordinal position for current term.  This is an
   *  optional method (the codec may throw {@link
   *  UnsupportedOperationException}).  Do not call this
   *  when the enum is unpositioned. */
  virtual int64_t ord() = 0;

  /** Returns the number of documents containing the current
   *  term.  Do not call this when the enum is unpositioned.
   *  {@link SeekStatus#END}.*/
  virtual int docFreq() = 0;

  /** Returns the total number of occurrences of this term
   *  across all documents (the sum of the freq() for each
   *  doc that has this term).  This will be -1 if the
   *  codec doesn't support this measure.  Note that, like
   *  other term measures, this measure does not take
   *  deleted documents into account. */
  virtual int64_t totalTermFreq() = 0;

  /** Get {@link PostingsEnum} for the current term.  Do not
   *  call this when the enum is unpositioned.  This method
   *  will not return null.
   *  <p>
   *  <b>NOTE</b>: the returned iterator may return deleted documents, so
   *  deleted documents have to be checked on top of the {@link PostingsEnum}.
   *  <p>
   *  Use this method if you only require documents and frequencies,
   *  and do not need any proximity data.
   *  This method is equivalent to
   *  {@link #postings(PostingsEnum, int) postings(reuse, PostingsEnum.FREQS)}
   *
   * @param reuse pass a prior PostingsEnum for possible reuse
   * @see #postings(PostingsEnum, int)
   */
  std::shared_ptr<PostingsEnum>
  postings(std::shared_ptr<PostingsEnum> reuse) ;

  /** Get {@link PostingsEnum} for the current term, with
   *  control over whether freqs, positions, offsets or payloads
   *  are required.  Do not call this when the enum is
   *  unpositioned.  This method will not return null.
   *  <p>
   *  <b>NOTE</b>: the returned iterator may return deleted documents, so
   *  deleted documents have to be checked on top of the {@link PostingsEnum}.
   *
   * @param reuse pass a prior PostingsEnum for possible reuse
   * @param flags specifies which optional per-document values
   *        you require; see {@link PostingsEnum#FREQS}
   */
  virtual std::shared_ptr<PostingsEnum>
  postings(std::shared_ptr<PostingsEnum> reuse, int flags) = 0;

  /**
   * Expert: Returns the TermsEnums internal state to position the TermsEnum
   * without re-seeking the term dictionary.
   * <p>
   * NOTE: A seek by {@link TermState} might not capture the
   * {@link AttributeSource}'s state. Callers must maintain the
   * {@link AttributeSource} states separately
   *
   * @see TermState
   * @see #seekExact(BytesRef, TermState)
   */
  virtual std::shared_ptr<TermState> termState() ;

private:
  class TermStateAnonymousInnerClass : public TermState
  {
    GET_CLASS_NAME(TermStateAnonymousInnerClass)
  private:
    std::shared_ptr<TermsEnum> outerInstance;

  public:
    TermStateAnonymousInnerClass(std::shared_ptr<TermsEnum> outerInstance);

    void copyFrom(std::shared_ptr<TermState> other) override;

  protected:
    std::shared_ptr<TermStateAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TermStateAnonymousInnerClass>(
          TermState::shared_from_this());
    }
  };

  /** An empty TermsEnum for quickly returning an empty instance e.g.
   * in {@link org.apache.lucene.search.MultiTermQuery}
   * <p><em>Please note:</em> This enum should be unmodifiable,
   * but it is currently possible to add Attributes to it.
   * This should not be a problem, as the enum is always empty and
   * the existence of unused Attributes does not matter.
   */
public:
  static const std::shared_ptr<TermsEnum> EMPTY;

private:
  class TermsEnumAnonymousInnerClass;
};

} // namespace org::apache::lucene::index
class TermsEnum::TermsEnumAnonymousInnerClass : public TermsEnum
{
  GET_CLASS_NAME(TermsEnum::TermsEnumAnonymousInnerClass)
public:
  TermsEnumAnonymousInnerClass();

  SeekStatus seekCeil(std::shared_ptr<BytesRef> term) override;
  void seekExact(int64_t ord) override;
  std::shared_ptr<BytesRef> term() override;

  int docFreq() override;

  int64_t totalTermFreq() override;

  int64_t ord() override;

  std::shared_ptr<PostingsEnum> postings(std::shared_ptr<PostingsEnum> reuse,
                                         int flags) override;

  std::shared_ptr<BytesRef> next() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<AttributeSource> attributes() override;

  std::shared_ptr<TermState> termState() override;

  void seekExact(std::shared_ptr<BytesRef> term,
                 std::shared_ptr<TermState> state) override;

protected:
  std::shared_ptr<TermsEnumAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<TermsEnumAnonymousInnerClass>(
        TermsEnum::shared_from_this());
  }
};
