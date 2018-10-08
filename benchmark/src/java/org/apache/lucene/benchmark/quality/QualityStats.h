#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
namespace org::apache::lucene::benchmark::quality
{

/**
 * Results of quality benchmark run for a single query or for a set of queries.
 */
class QualityStats : public std::enable_shared_from_this<QualityStats>
{
  GET_CLASS_NAME(QualityStats)

  /** Number of points for which precision is computed. */
public:
  static constexpr int MAX_POINTS = 20;

private:
  double maxGoodPoints = 0;
  double recall = 0;
  std::deque<double> pAt;
  double pReleventSum = 0;
  double numPoints = 0;
  double numGoodPoints = 0;
  double mrr = 0;
  int64_t searchTime = 0;
  int64_t docNamesExtractTime = 0;

  /**
   * A certain rank in which a relevant doc was found.
   */
public:
  class RecallPoint : public std::enable_shared_from_this<RecallPoint>
  {
    GET_CLASS_NAME(RecallPoint)
  private:
    int rank = 0;
    double recall = 0;
    RecallPoint(int rank, double recall);
    /** Returns the rank: where on the deque of returned docs this relevant doc
     * appeared. */
  public:
    virtual int getRank();
    /** Returns the recall: how many relevant docs were returned up to this
     * point, inclusive. */
    virtual double getRecall();
  };

private:
  std::deque<std::shared_ptr<RecallPoint>> recallPoints;

  /**
   * Construct a QualityStats object with anticipated maximal number of relevant
   * hits.
   * @param maxGoodPoints maximal possible relevant hits.
   */
public:
  QualityStats(double maxGoodPoints, int64_t searchTime);

  /**
   * Add a (possibly relevant) doc.
   * @param n rank of the added doc (its ordinal position within the query
   * results).
   * @param isRelevant true if the added doc is relevant, false otherwise.
   */
  virtual void addResult(int n, bool isRelevant, int64_t docNameExtractTime);

  /**
   * Return the precision at rank n:
   * |{relevant hits within first <code>n</code> hits}| / <code>n</code>.
   * @param n requested precision point, must be at least 1 and at most {@link
   * #MAX_POINTS}.
   */
  virtual double getPrecisionAt(int n);

  /**
   * Return the average precision at recall points.
   */
  virtual double getAvp();

  /**
   * Return the recall: |{relevant hits found}| / |{relevant hits existing}|.
   */
  virtual double getRecall();

  /**
   * Log information on this QualityStats object.
   * @param logger Logger.
   * @param prefix prefix before each log line.
   */
  virtual void log(const std::wstring &title, int paddLines,
                   std::shared_ptr<PrintWriter> logger,
                   const std::wstring &prefix);

private:
  static std::wstring padd;
  std::wstring format(const std::wstring &s, int minLen);
  std::wstring fracFormat(const std::wstring &frac);

  /**
   * Create a QualityStats object that is the average of the input QualityStats
   * objects.
   * @param stats array of input stats to be averaged.
   * @return an average over the input stats.
   */
public:
  static std::shared_ptr<QualityStats>
  average(std::deque<std::shared_ptr<QualityStats>> &stats);

  /**
   * Returns the time it took to extract doc names for judging the measured
   * query, in milliseconds.
   */
  virtual int64_t getDocNamesExtractTime();

  /**
   * Returns the maximal number of good points.
   * This is the number of relevant docs known by the judge for the measured
   * query.
   */
  virtual double getMaxGoodPoints();

  /**
   * Returns the number of good points (only relevant points).
   */
  virtual double getNumGoodPoints();

  /**
   * Returns the number of points (both relevant and irrelevant points).
   */
  virtual double getNumPoints();

  /**
   * Returns the recallPoints.
   */
  virtual std::deque<std::shared_ptr<RecallPoint>> getRecallPoints();

  /**
   * Returns the Mean reciprocal rank over the queries or RR for a single query.
   * <p>
   * Reciprocal rank is defined as <code>1/r</code> where <code>r</code> is the
   * rank of the first correct result, or <code>0</code> if there are no correct
   * results within the top 5 results.
   * <p>
   * This follows the definition in
   * <a href="http://www.cnlp.org/publications/02cnlptrec10.pdf">
   * Question Answering - CNLP at the TREC-10 Question Answering Track</a>.
   */
  virtual double getMRR();

  /**
   * Returns the search time in milliseconds for the measured query.
   */
  virtual int64_t getSearchTime();
};

} // namespace org::apache::lucene::benchmark::quality
