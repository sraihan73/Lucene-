#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class TermsEnumWithSlice;
}

namespace org::apache::lucene::index
{
class TermMergeQueue;
}
namespace org::apache::lucene::index
{
class MultiPostingsEnum;
}
namespace org::apache::lucene::index
{
class EnumWithSlice;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::index
{
class ReaderSlice;
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
namespace org::apache::lucene::index
{

using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Exposes {@link TermsEnum} API, merged from {@link TermsEnum} API of
 * sub-segments. This does a merge sort, by term text, of the sub-readers.
 *
 * @lucene.experimental
 */
class MultiTermsEnum final : public TermsEnum
{
  GET_CLASS_NAME(MultiTermsEnum)

private:
  static const std::shared_ptr<Comparator<std::shared_ptr<TermsEnumWithSlice>>>
      INDEX_COMPARATOR;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<TermsEnumWithSlice>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::shared_ptr<TermsEnumWithSlice> o1,
                std::shared_ptr<TermsEnumWithSlice> o2) override;
  };

private:
  const std::shared_ptr<TermMergeQueue> queue;
  std::deque<std::shared_ptr<TermsEnumWithSlice>> const
      subs; // all of our subs (one per sub-reader)
  std::deque<std::shared_ptr<TermsEnumWithSlice>> const
      currentSubs; // current subs that have at least one term for this field
  std::deque<std::shared_ptr<TermsEnumWithSlice>> const top;
  std::deque<std::shared_ptr<MultiPostingsEnum::EnumWithSlice>> const subDocs;

  std::shared_ptr<BytesRef> lastSeek;
  bool lastSeekExact = false;
  const std::shared_ptr<BytesRefBuilder> lastSeekScratch =
      std::make_shared<BytesRefBuilder>();

  int numTop = 0;
  int numSubs = 0;
  std::shared_ptr<BytesRef> current;

public:
  class TermsEnumIndex : public std::enable_shared_from_this<TermsEnumIndex>
  {
    GET_CLASS_NAME(TermsEnumIndex)
  public:
    static std::deque<std::shared_ptr<TermsEnumIndex>> const EMPTY_ARRAY;
    const int subIndex;
    const std::shared_ptr<TermsEnum> termsEnum;

    TermsEnumIndex(std::shared_ptr<TermsEnum> termsEnum, int subIndex);
  };

  /** Returns how many sub-reader slices contain the current
   *  term.  @see #getMatchArray */
public:
  int getMatchCount();

  /** Returns sub-reader slices positioned to the current term. */
  std::deque<std::shared_ptr<TermsEnumWithSlice>> getMatchArray();

  /** Sole constructor.
   *  @param slices Which sub-reader slices we should
   *  merge. */
  MultiTermsEnum(std::deque<std::shared_ptr<ReaderSlice>> &slices);

  std::shared_ptr<BytesRef> term() override;

  /** The terms array must be newly created TermsEnum, ie
   *  {@link TermsEnum#next} has not yet been called. */
  std::shared_ptr<TermsEnum> reset(std::deque<std::shared_ptr<TermsEnumIndex>>
                                       &termsEnumsIndex) ;

  bool seekExact(std::shared_ptr<BytesRef> term)  override;

  SeekStatus
  seekCeil(std::shared_ptr<BytesRef> term)  override;

  void seekExact(int64_t ord) override;

  int64_t ord() override;

private:
  void pullTop();

  void pushTop() ;

public:
  std::shared_ptr<BytesRef> next()  override;

  int docFreq()  override;

  int64_t totalTermFreq()  override;

  std::shared_ptr<PostingsEnum> postings(std::shared_ptr<PostingsEnum> reuse,
                                         int flags)  override;

public:
  class TermsEnumWithSlice final
      : public std::enable_shared_from_this<TermsEnumWithSlice>
  {
    GET_CLASS_NAME(TermsEnumWithSlice)
  private:
    const std::shared_ptr<ReaderSlice> subSlice;

  public:
    std::shared_ptr<TermsEnum> terms;
    std::shared_ptr<BytesRef> current;
    const int index;

    TermsEnumWithSlice(int index, std::shared_ptr<ReaderSlice> subSlice);

    void reset(std::shared_ptr<TermsEnum> terms,
               std::shared_ptr<BytesRef> term);

    virtual std::wstring toString();
  };

private:
  class TermMergeQueue final
      : public PriorityQueue<std::shared_ptr<TermsEnumWithSlice>>
  {
    GET_CLASS_NAME(TermMergeQueue)

  public:
    std::deque<int> const stack;

    TermMergeQueue(int size);

  protected:
    bool lessThan(std::shared_ptr<TermsEnumWithSlice> termsA,
                  std::shared_ptr<TermsEnumWithSlice> termsB) override;

    /** Add the {@link #top()} slice as well as all slices that are positionned
     *  on the same term to {@code tops} and return how many of them there are.
     */
  public:
    int fillTop(std::deque<std::shared_ptr<TermsEnumWithSlice>> &tops);

  private:
    std::shared_ptr<TermsEnumWithSlice> get(int i);

  protected:
    std::shared_ptr<TermMergeQueue> shared_from_this()
    {
      return std::static_pointer_cast<TermMergeQueue>(
          org.apache.lucene.util
              .PriorityQueue<TermsEnumWithSlice>::shared_from_this());
    }
  };

public:
  virtual std::wstring toString();

protected:
  std::shared_ptr<MultiTermsEnum> shared_from_this()
  {
    return std::static_pointer_cast<MultiTermsEnum>(
        TermsEnum::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
