#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/PriorityQueue.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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
namespace org::apache::lucene::benchmark::quality::utils
{

using Directory = org::apache::lucene::store::Directory;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Suggest Quality queries based on an index contents.
 * Utility class, used for making quality test benchmarks.
 */
class QualityQueriesFinder
    : public std::enable_shared_from_this<QualityQueriesFinder>
{
  GET_CLASS_NAME(QualityQueriesFinder)

private:
  static const std::wstring newline;
  std::shared_ptr<Directory> dir;

  /**
   * Constructor over a directory containing the index.
   * @param dir directory containing the index we search for the quality test.
   */
  QualityQueriesFinder(std::shared_ptr<Directory> dir);

  /**
   * @param args {index-dir}
   * @throws IOException  if cannot access the index.
   */
  static void main(std::deque<std::wstring> &args) ;

  std::deque<std::wstring> bestQueries(const std::wstring &field,
                                        int numQueries) ;

  static std::wstring formatQueryAsTrecTopic(int qnum,
                                             const std::wstring &title,
                                             const std::wstring &description,
                                             const std::wstring &narrative);

  std::deque<std::wstring> bestTerms(const std::wstring &field,
                                      int numTerms) ;

private:
  class TermDf : public std::enable_shared_from_this<TermDf>
  {
    GET_CLASS_NAME(TermDf)
  public:
    std::wstring word;
    int df = 0;
    TermDf(const std::wstring &word, int freq);
  };

private:
  class TermsDfQueue : public PriorityQueue<std::shared_ptr<TermDf>>
  {
    GET_CLASS_NAME(TermsDfQueue)
  public:
    TermsDfQueue(int maxSize);

  protected:
    bool lessThan(std::shared_ptr<TermDf> tf1,
                  std::shared_ptr<TermDf> tf2) override;

  protected:
    std::shared_ptr<TermsDfQueue> shared_from_this()
    {
      return std::static_pointer_cast<TermsDfQueue>(
          org.apache.lucene.util.PriorityQueue<TermDf>::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/benchmark/quality/utils/
