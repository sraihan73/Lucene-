#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldFragList.h"

#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldPhraseList.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/WeightedPhraseInfo.h"

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

/**
 * A abstract implementation of {@link FragListBuilder}.
 */
class BaseFragListBuilder
    : public std::enable_shared_from_this<BaseFragListBuilder>,
      public FragListBuilder
{
  GET_CLASS_NAME(BaseFragListBuilder)

public:
  static constexpr int MARGIN_DEFAULT = 6;
  static constexpr int MIN_FRAG_CHAR_SIZE_FACTOR = 3;

  const int margin;
  const int minFragCharSize;

  BaseFragListBuilder(int margin);

  BaseFragListBuilder();

protected:
  virtual std::shared_ptr<FieldFragList>
  createFieldFragList(std::shared_ptr<FieldPhraseList> fieldPhraseList,
                      std::shared_ptr<FieldFragList> fieldFragList,
                      int fragCharSize);

  /**
   * A predicate to decide if the given {@link WeightedPhraseInfo} should be
   * accepted as a highlighted phrase or if it should be discarded.
   * <p>
   * The default implementation discards phrases that are composed of more than
   * one term and where the matchLength exceeds the fragment character size.
   *
   * @param info the phrase info to accept
   * @param matchLength the match length of the current phrase
   * @param fragCharSize the configured fragment character size
   * @return <code>true</code> if this phrase info should be accepted as a
   * highligh phrase
   */
  virtual bool acceptPhrase(std::shared_ptr<WeightedPhraseInfo> info,
                            int matchLength, int fragCharSize);

private:
  template <typename T>
  class IteratorQueue final : public std::enable_shared_from_this<IteratorQueue>
  {
    GET_CLASS_NAME(IteratorQueue)
  private:
    const std::shared_ptr<Iterator<T>> iter;
    // C++ NOTE: Fields cannot have the same name as methods:
    T top_;

  public:
    IteratorQueue(std::shared_ptr<Iterator<T>> iter) : iter(iter)
    {
      T removeTop = this->removeTop();
      assert(removeTop == nullptr);
    }

    T top() { return top_; }

    T removeTop()
    {
      T currentTop = top_;
      if (iter->hasNext()) {
        top_ = iter->next();
      } else {
        top_ = nullptr;
      }
      return currentTop;
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
