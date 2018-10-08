#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/WeightedFragInfo.h"

#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldPhraseList.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/WeightedPhraseInfo.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/SubInfo.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/Toffs.h"

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

using WeightedPhraseInfo = org::apache::lucene::search::vectorhighlight::
    FieldPhraseList::WeightedPhraseInfo;
using Toffs = org::apache::lucene::search::vectorhighlight::FieldPhraseList::
    WeightedPhraseInfo::Toffs;

/**
 * FieldFragList has a deque of "frag info" that is used by FragmentsBuilder
 * class to create fragments (snippets).
 */
class FieldFragList : public std::enable_shared_from_this<FieldFragList>
{
  GET_CLASS_NAME(FieldFragList)

private:
  std::deque<std::shared_ptr<WeightedFragInfo>> fragInfos =
      std::deque<std::shared_ptr<WeightedFragInfo>>();

  /**
   * a constructor.
   *
   * @param fragCharSize the length (number of chars) of a fragment
   */
public:
  FieldFragList(int fragCharSize);

  /**
   * convert the deque of WeightedPhraseInfo to WeightedFragInfo, then add it to
   * the fragInfos
   *
   * @param startOffset start offset of the fragment
   * @param endOffset end offset of the fragment
   * @param phraseInfoList deque of WeightedPhraseInfo objects
   */
  virtual void
  add(int startOffset, int endOffset,
      std::deque<std::shared_ptr<WeightedPhraseInfo>> &phraseInfoList) = 0;

  /**
   * return the deque of WeightedFragInfos.
   *
   * @return fragInfos.
   */
  virtual std::deque<std::shared_ptr<WeightedFragInfo>> getFragInfos();

  /**
   * List of term offsets + weight for a frag info
   */
public:
  class WeightedFragInfo : public std::enable_shared_from_this<WeightedFragInfo>
  {
    GET_CLASS_NAME(WeightedFragInfo)

  private:
    std::deque<std::shared_ptr<SubInfo>> subInfos;
    float totalBoost = 0;
    int startOffset = 0;
    int endOffset = 0;

  public:
    WeightedFragInfo(int startOffset, int endOffset,
                     std::deque<std::shared_ptr<SubInfo>> &subInfos,
                     float totalBoost);

    virtual std::deque<std::shared_ptr<SubInfo>> getSubInfos();

    virtual float getTotalBoost();

    virtual int getStartOffset();

    virtual int getEndOffset();

    virtual std::wstring toString();

    /**
     * Represents the deque of term offsets for some text
     */
  public:
    class SubInfo : public std::enable_shared_from_this<SubInfo>
    {
      GET_CLASS_NAME(SubInfo)
    private:
      const std::wstring
          text; // unnecessary member, just exists for debugging purpose
      const std::deque<std::shared_ptr<WeightedPhraseInfo::Toffs>>
          termsOffsets; // usually termsOffsets.size() == 1,
                        // but if position-gap > 1 and slop > 0 then size()
                        // could be greater than 1
      const int seqnum;
      const float boost; // used for scoring split WeightedPhraseInfos.

    public:
      SubInfo(
          const std::wstring &text,
          std::deque<std::shared_ptr<WeightedPhraseInfo::Toffs>> &termsOffsets,
          int seqnum, float boost);

      virtual std::deque<std::shared_ptr<WeightedPhraseInfo::Toffs>>
      getTermsOffsets();

      virtual int getSeqnum();

      virtual std::wstring getText();

      virtual float getBoost();

      virtual std::wstring toString();
    };
  };
};

} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
