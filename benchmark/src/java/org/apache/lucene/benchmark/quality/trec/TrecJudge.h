#pragma once
#include "../Judge.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::quality::trec
{
class QRelJudgement;
}

namespace org::apache::lucene::benchmark::quality
{
class QualityQuery;
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
namespace org::apache::lucene::benchmark::quality::trec
{

using Judge = org::apache::lucene::benchmark::quality::Judge;
using QualityQuery = org::apache::lucene::benchmark::quality::QualityQuery;

/**
 * Judge if given document is relevant to given quality query, based on Trec
 * format for judgements.
 */
class TrecJudge : public std::enable_shared_from_this<TrecJudge>, public Judge
{
  GET_CLASS_NAME(TrecJudge)

public:
  std::unordered_map<std::wstring, std::shared_ptr<QRelJudgement>> judgements;

  /**
   * Constructor from a reader.
   * <p>
   * Expected input format:
   * <pre>
   *     qnum  0   doc-name     is-relevant
   * </pre>
   * Two sample lines:
   * <pre>
   *     19    0   doc303       1
   *     19    0   doc7295      0
   * </pre>
   * @param reader where judgments are read from.
   * @throws IOException If there is a low-level I/O error.
   */
  TrecJudge(std::shared_ptr<BufferedReader> reader) ;

  // inherit javadocs
  bool isRelevant(const std::wstring &docName,
                  std::shared_ptr<QualityQuery> query) override;

  /** single Judgement of a trec quality query */
private:
  class QRelJudgement : public std::enable_shared_from_this<QRelJudgement>
  {
    GET_CLASS_NAME(QRelJudgement)
  private:
    std::wstring queryID;
    std::unordered_map<std::wstring, std::wstring> relevantDocs;

  public:
    QRelJudgement(const std::wstring &queryID);

    virtual void addRelevandDoc(const std::wstring &docName);

    virtual bool isRelevant(const std::wstring &docName);

    virtual int maxRecall();
  };

  // inherit javadocs
public:
  bool validateData(std::deque<std::shared_ptr<QualityQuery>> &qq,
                    std::shared_ptr<PrintWriter> logger) override;

  // inherit javadocs
  int maxRecall(std::shared_ptr<QualityQuery> query) override;
};

} // namespace org::apache::lucene::benchmark::quality::trec
