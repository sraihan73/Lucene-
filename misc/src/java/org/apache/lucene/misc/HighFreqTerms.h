#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

#include  "core/src/java/org/apache/lucene/misc/TermStats.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"

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
namespace org::apache::lucene::misc
{

using IndexReader = org::apache::lucene::index::IndexReader;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * <code>HighFreqTerms</code> class extracts the top n most frequent terms
 * (by document frequency) from an existing Lucene index and reports their
 * document frequency.
 * <p>
 * If the -t flag is given, both document frequency and total tf (total
 * number of occurrences) are reported, ordered by descending total tf.
 *
 */
class HighFreqTerms : public std::enable_shared_from_this<HighFreqTerms>
{
  GET_CLASS_NAME(HighFreqTerms)

  // The top numTerms will be displayed
public:
  static constexpr int DEFAULT_NUMTERMS = 100;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command
  // line tool") public static void main(std::wstring[] args) throws Exception
  static void main(std::deque<std::wstring> &args) ;

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command
  // line tool") private static void usage()
  static void usage();

  /**
   * Returns TermStats[] ordered by the specified comparator
   */
public:
  static std::deque<std::shared_ptr<TermStats>>
  getHighFreqTerms(std::shared_ptr<IndexReader> reader, int numTerms,
                   const std::wstring &field,
                   std::shared_ptr<Comparator<std::shared_ptr<TermStats>>>
                       comparator) ;

  /**
   * Compares terms by docTermFreq
   */
public:
  class DocFreqComparator final
      : public std::enable_shared_from_this<DocFreqComparator>,
        public Comparator<std::shared_ptr<TermStats>>
  {
    GET_CLASS_NAME(DocFreqComparator)

  public:
    int compare(std::shared_ptr<TermStats> a,
                std::shared_ptr<TermStats> b) override;
  };

  /**
   * Compares terms by totalTermFreq
   */
public:
  class TotalTermFreqComparator final
      : public std::enable_shared_from_this<TotalTermFreqComparator>,
        public Comparator<std::shared_ptr<TermStats>>
  {
    GET_CLASS_NAME(TotalTermFreqComparator)

  public:
    int compare(std::shared_ptr<TermStats> a,
                std::shared_ptr<TermStats> b) override;
  };

  /**
   * Priority queue for TermStats objects
   **/
public:
  class TermStatsQueue final : public PriorityQueue<std::shared_ptr<TermStats>>
  {
    GET_CLASS_NAME(TermStatsQueue)
  public:
    const std::shared_ptr<Comparator<std::shared_ptr<TermStats>>> comparator;

    TermStatsQueue(
        int size,
        std::shared_ptr<Comparator<std::shared_ptr<TermStats>>> comparator);

  protected:
    bool lessThan(std::shared_ptr<TermStats> termInfoA,
                  std::shared_ptr<TermStats> termInfoB) override;

    void fill(const std::wstring &field,
              std::shared_ptr<TermsEnum> termsEnum) ;

  protected:
    std::shared_ptr<TermStatsQueue> shared_from_this()
    {
      return std::static_pointer_cast<TermStatsQueue>(
          org.apache.lucene.util.PriorityQueue<TermStats>::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/misc/
