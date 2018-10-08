#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/WeightedPhraseInfo.h"

#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldQuery.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldTermStack.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/Toffs.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/TermInfo.h"

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
namespace org::apache::lucene::search::vectorhighlight
{

using TermInfo =
    org::apache::lucene::search::vectorhighlight::FieldTermStack::TermInfo;

/**
 * FieldPhraseList has a deque of WeightedPhraseInfo that is used by
 * FragListBuilder to create a FieldFragList object.
 */
class FieldPhraseList : public std::enable_shared_from_this<FieldPhraseList>
{
  GET_CLASS_NAME(FieldPhraseList)
  /**
   * List of non-overlapping WeightedPhraseInfo objects.
   */
public:
  std::deque<std::shared_ptr<WeightedPhraseInfo>> phraseList =
      std::deque<std::shared_ptr<WeightedPhraseInfo>>();

  /**
   * create a FieldPhraseList that has no limit on the number of phrases to
   * analyze
   *
   * @param fieldTermStack FieldTermStack object
   * @param fieldQuery FieldQuery object
   */
  FieldPhraseList(std::shared_ptr<FieldTermStack> fieldTermStack,
                  std::shared_ptr<FieldQuery> fieldQuery);

  /**
   * return the deque of WeightedPhraseInfo.
   *
   * @return phraseList.
   */
  virtual std::deque<std::shared_ptr<WeightedPhraseInfo>> getPhraseList();

  /**
   * a constructor.
   *
   * @param fieldTermStack FieldTermStack object
   * @param fieldQuery FieldQuery object
   * @param phraseLimit maximum size of phraseList
   */
  FieldPhraseList(std::shared_ptr<FieldTermStack> fieldTermStack,
                  std::shared_ptr<FieldQuery> fieldQuery, int phraseLimit);

  /**
   * Merging constructor.
   *
   * @param toMerge FieldPhraseLists to merge to build this one
   */
  FieldPhraseList(std::deque<std::shared_ptr<FieldPhraseList>> &toMerge);

  virtual void addIfNoOverlap(std::shared_ptr<WeightedPhraseInfo> wpi);

  /**
   * Represents the deque of term offsets and boost for some text
   */
public:
  class WeightedPhraseInfo
      : public std::enable_shared_from_this<WeightedPhraseInfo>,
        public Comparable<std::shared_ptr<WeightedPhraseInfo>>
  {
    GET_CLASS_NAME(WeightedPhraseInfo)
  private:
    std::deque<std::shared_ptr<Toffs>>
        termsOffsets; // usually termsOffsets.size() == 1,
                      // but if position-gap > 1 and slop > 0 then size() could
                      // be greater than 1
    float boost = 0;  // query boost
    int seqnum = 0;

    std::deque<std::shared_ptr<TermInfo>> termsInfos;

    /**
     * Text of the match, calculated on the fly.  Use for debugging only.
     * @return the text
     */
  public:
    virtual std::wstring getText();

    /**
     * @return the termsOffsets
     */
    virtual std::deque<std::shared_ptr<Toffs>> getTermsOffsets();

    /**
     * @return the boost
     */
    virtual float getBoost();

    /**
     * @return the termInfos
     */
    virtual std::deque<std::shared_ptr<TermInfo>> getTermsInfos();

    WeightedPhraseInfo(std::deque<std::shared_ptr<TermInfo>> &terms,
                       float boost);

    WeightedPhraseInfo(std::deque<std::shared_ptr<TermInfo>> &terms, float boost,
                       int seqnum);

    /**
     * Merging constructor.  Note that this just grabs seqnum from the first
     * info.
     */
    WeightedPhraseInfo(
        std::shared_ptr<std::deque<std::shared_ptr<WeightedPhraseInfo>>>
            toMerge);

    virtual int getStartOffset();

    virtual int getEndOffset();

    virtual bool isOffsetOverlap(std::shared_ptr<WeightedPhraseInfo> other);

    virtual std::wstring toString();

    /**
     * @return the seqnum
     */
    virtual int getSeqnum();

    int compareTo(std::shared_ptr<WeightedPhraseInfo> other) override;

    virtual int hashCode();

    bool equals(std::any obj) override;

    /**
     * Term offsets (start + end)
     */
  public:
    class Toffs : public std::enable_shared_from_this<Toffs>,
                  public Comparable<std::shared_ptr<Toffs>>
    {
      GET_CLASS_NAME(Toffs)
    private:
      int startOffset = 0;
      int endOffset = 0;

    public:
      Toffs(int startOffset, int endOffset);
      virtual void setEndOffset(int endOffset);
      virtual int getStartOffset();
      virtual int getEndOffset();
      int compareTo(std::shared_ptr<Toffs> other) override;
      virtual int hashCode();
      bool equals(std::any obj) override;
      virtual std::wstring toString();
    };
  };
};

} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
