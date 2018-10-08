#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::util
{
class AttributeSource;
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
namespace org::apache::lucene::index
{

using BytesRef = org::apache::lucene::util::BytesRef;
using AttributeSource = org::apache::lucene::util::AttributeSource;

/**
 * Abstract class for enumerating a subset of all terms.
 *
 * <p>Term enumerations are always ordered by
 * {@link BytesRef#compareTo}.  Each term in the enumeration is
 * greater than all that precede it.</p>
 * <p><em>Please note:</em> Consumers of this enum cannot
 * call {@code seek()}, it is forward only; it throws
 * {@link UnsupportedOperationException} when a seeking method
 * is called.
 */
class FilteredTermsEnum : public TermsEnum
{
  GET_CLASS_NAME(FilteredTermsEnum)

private:
  std::shared_ptr<BytesRef> initialSeekTerm;
  bool doSeek = false;

  /** Which term the enum is currently positioned to. */
protected:
  std::shared_ptr<BytesRef> actualTerm;

  /** The delegate {@link TermsEnum}. */
  const std::shared_ptr<TermsEnum> tenum;

  /** Return value, if term should be accepted or the iteration should
   * {@code END}. The {@code *_SEEK} values denote, that after handling the
   * current term the enum should call {@link #nextSeekTerm} and step forward.
   * @see #accept(BytesRef)
   */
protected:
  enum class AcceptStatus {
    GET_CLASS_NAME(AcceptStatus)
    /** Accept the term and position the enum at the next term. */
    YES,
    /** Accept the term and advance ({@link
     * FilteredTermsEnum#nextSeekTerm(BytesRef)}) to the next term. */
    YES_AND_SEEK,
    /** Reject the term and position the enum at the next term. */
    NO,
    /** Reject the term and advance ({@link
     * FilteredTermsEnum#nextSeekTerm(BytesRef)}) to the next term. */
    NO_AND_SEEK,
    /** Reject the term and stop enumerating. */
    END
  };

  /** Return if term is accepted, not accepted or the iteration should ended
   * (and possibly seek).
   */
protected:
  virtual AcceptStatus accept(std::shared_ptr<BytesRef> term) = 0;

  /**
   * Creates a filtered {@link TermsEnum} on a terms enum.
   * @param tenum the terms enumeration to filter.
   */
public:
  FilteredTermsEnum(std::shared_ptr<TermsEnum> tenum);

  /**
   * Creates a filtered {@link TermsEnum} on a terms enum.
   * @param tenum the terms enumeration to filter.
   */
  FilteredTermsEnum(std::shared_ptr<TermsEnum> tenum, bool const startWithSeek);

  /**
   * Use this method to set the initial {@link BytesRef}
   * to seek before iterating. This is a convenience method for
   * subclasses that do not override {@link #nextSeekTerm}.
   * If the initial seek term is {@code null} (default),
GET_CLASS_NAME(es)
   * the enum is empty.
   * <P>You can only use this method, if you keep the default
   * implementation of {@link #nextSeekTerm}.
   */
protected:
  void setInitialSeekTerm(std::shared_ptr<BytesRef> term);

  /** On the first call to {@link #next} or if {@link #accept} returns
   * {@link AcceptStatus#YES_AND_SEEK} or {@link AcceptStatus#NO_AND_SEEK},
   * this method will be called to eventually seek the underlying TermsEnum
   * to a new position.
   * On the first call, {@code currentTerm} will be {@code null}, later
   * calls will provide the term the underlying enum is positioned at.
   * This method returns per default only one time the initial seek term
   * and then {@code null}, so no repositioning is ever done.
   * <p>Override this method, if you want a more sophisticated TermsEnum,
   * that repositions the iterator during enumeration.
   * If this method always returns {@code null} the enum is empty.
   * <p><em>Please note:</em> This method should always provide a greater term
   * than the last enumerated term, else the behaviour of this enum
   * violates the contract for TermsEnums.
   */
  virtual std::shared_ptr<BytesRef>
  nextSeekTerm(std::shared_ptr<BytesRef> currentTerm) ;

  /**
   * Returns the related attributes, the returned {@link AttributeSource}
   * is shared with the delegate {@code TermsEnum}.
   */
public:
  std::shared_ptr<AttributeSource> attributes() override;

  std::shared_ptr<BytesRef> term()  override;

  int docFreq()  override;

  int64_t totalTermFreq()  override;

  /** This enum does not support seeking!
   * @throws UnsupportedOperationException In general, subclasses do not
   *         support seeking.
   */
  bool seekExact(std::shared_ptr<BytesRef> term)  override;

  /** This enum does not support seeking!
   * @throws UnsupportedOperationException In general, subclasses do not
   *         support seeking.
   */
  SeekStatus
  seekCeil(std::shared_ptr<BytesRef> term)  override;

  /** This enum does not support seeking!
   * @throws UnsupportedOperationException In general, subclasses do not
   *         support seeking.
   */
  void seekExact(int64_t ord)  override;

  int64_t ord()  override;

  std::shared_ptr<PostingsEnum> postings(std::shared_ptr<PostingsEnum> reuse,
                                         int flags)  override;

  /** This enum does not support seeking!
   * @throws UnsupportedOperationException In general, subclasses do not
   *         support seeking.
   */
  void seekExact(std::shared_ptr<BytesRef> term,
                 std::shared_ptr<TermState> state)  override;

  /**
   * Returns the filtered enums term state
   */
  std::shared_ptr<TermState> termState()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("fallthrough") @Override public
  // org.apache.lucene.util.BytesRef next() throws java.io.IOException
  std::shared_ptr<BytesRef> next()  override;

protected:
  std::shared_ptr<FilteredTermsEnum> shared_from_this()
  {
    return std::static_pointer_cast<FilteredTermsEnum>(
        TermsEnum::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
