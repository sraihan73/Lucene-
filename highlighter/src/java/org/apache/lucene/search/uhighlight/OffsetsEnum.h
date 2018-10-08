#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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

namespace org::apache::lucene::search::uhighlight
{

using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * An enumeration/iterator of a term and its offsets for use by {@link
 * FieldHighlighter}. It is advanced and is placed in a priority queue by
 * {@link FieldHighlighter#highlightOffsetsEnums(OffsetsEnum)} based on the
 * start offset.
 *
 * @lucene.internal
 */
class OffsetsEnum : public std::enable_shared_from_this<OffsetsEnum>,
                    public Comparable<std::shared_ptr<OffsetsEnum>>
{
  GET_CLASS_NAME(OffsetsEnum)

  // note: the ordering clearly changes as the postings enum advances
  // note: would be neat to use some Comparator utilities with method
  //  references but our methods throw IOException
public:
  int compareTo(std::shared_ptr<OffsetsEnum> other) override;

  /**
   * Advances to the next position and returns true, or if can't then returns
   * false. Note that the initial state of this class is not positioned.
   */
  virtual bool nextPosition() = 0;

  /** An estimate of the number of occurrences of this term/OffsetsEnum. */
  virtual int freq() = 0;

  /**
   * The term at this position; usually always the same.
   * This BytesRef is safe to continue to refer to, even after we move to the
   * next position.
   */
  virtual std::shared_ptr<BytesRef> getTerm() = 0;

  virtual int startOffset() = 0;

  virtual int endOffset() = 0;

  virtual ~OffsetsEnum();

  virtual std::wstring toString();

  /**
   * Based on a {@link PostingsEnum} -- the typical/standard OE impl.
   */
public:
  class OfPostings;

  /**
   * Empty enumeration
   */
public:
  static const std::shared_ptr<OffsetsEnum> EMPTY;

private:
  class OffsetsEnumAnonymousInnerClass;

  /**
   * A view over several OffsetsEnum instances, merging them in-place
   */
public:
  class MultiOffsetsEnum;
};

} // namespace org::apache::lucene::search::uhighlight
class OffsetsEnum::OfPostings : public OffsetsEnum
{
  GET_CLASS_NAME(OffsetsEnum::OfPostings)
private:
  const std::shared_ptr<BytesRef> term;
  const std::shared_ptr<PostingsEnum> postingsEnum; // with offsets
  // C++ NOTE: Fields cannot have the same name as methods:
  const int freq_;

  int posCounter = -1;

public:
  OfPostings(std::shared_ptr<BytesRef> term, int freq,
             std::shared_ptr<PostingsEnum> postingsEnum) ;

  OfPostings(std::shared_ptr<BytesRef> term,
             std::shared_ptr<PostingsEnum> postingsEnum) ;

  virtual std::shared_ptr<PostingsEnum> getPostingsEnum();

  bool nextPosition()  override;

  std::shared_ptr<BytesRef> getTerm()  override;

  int startOffset()  override;

  int endOffset()  override;

  int freq()  override;

protected:
  std::shared_ptr<OfPostings> shared_from_this()
  {
    return std::static_pointer_cast<OfPostings>(
        OffsetsEnum::shared_from_this());
  }
};
class OffsetsEnum::OffsetsEnumAnonymousInnerClass : public OffsetsEnum
{
  GET_CLASS_NAME(OffsetsEnum::OffsetsEnumAnonymousInnerClass)
public:
  OffsetsEnumAnonymousInnerClass();

  bool nextPosition()  override;

  std::shared_ptr<BytesRef> getTerm()  override;

  int startOffset()  override;

  int endOffset()  override;

  int freq()  override;

protected:
  std::shared_ptr<OffsetsEnumAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<OffsetsEnumAnonymousInnerClass>(
        OffsetsEnum::shared_from_this());
  }
};
class OffsetsEnum::MultiOffsetsEnum : public OffsetsEnum
{
  GET_CLASS_NAME(OffsetsEnum::MultiOffsetsEnum)

private:
  const std::shared_ptr<PriorityQueue<std::shared_ptr<OffsetsEnum>>> queue;
  bool started = false;

public:
  MultiOffsetsEnum(std::deque<std::shared_ptr<OffsetsEnum>> &inner) throw(
      IOException);

  bool nextPosition()  override;

  std::shared_ptr<BytesRef> getTerm()  override;

  int startOffset()  override;

  int endOffset()  override;

  int freq()  override;

  virtual ~MultiOffsetsEnum();

protected:
  std::shared_ptr<MultiOffsetsEnum> shared_from_this()
  {
    return std::static_pointer_cast<MultiOffsetsEnum>(
        OffsetsEnum::shared_from_this());
  }
};
