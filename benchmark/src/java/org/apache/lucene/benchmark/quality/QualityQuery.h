#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
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
 * A QualityQuery has an ID and some name-value pairs.
 * <p>
 * The ID allows to map_obj the quality query with its judgements.
 * <p>
 * The name-value pairs are used by a
 * {@link org.apache.lucene.benchmark.quality.QualityQueryParser}
 * to create a Lucene {@link org.apache.lucene.search.Query}.
 * <p>
 * It is very likely that name-value-pairs would be mapped into fields in a
 * Lucene query, but it is up to the QualityQueryParser how to map_obj - e.g. all
 * values in a single field, or each pair as its own field, etc., - and this of
 * course must match the way the searched index was constructed.
 */
class QualityQuery : public std::enable_shared_from_this<QualityQuery>,
                     public Comparable<std::shared_ptr<QualityQuery>>
{
  GET_CLASS_NAME(QualityQuery)
private:
  std::wstring queryID;
  std::unordered_map<std::wstring, std::wstring> nameValPairs;

  /**
   * Create a QualityQuery with given ID and name-value pairs.
   * @param queryID ID of this quality query.
   * @param nameValPairs the contents of this quality query.
   */
public:
  QualityQuery(const std::wstring &queryID,
               std::unordered_map<std::wstring, std::wstring> &nameValPairs);

  /**
   * Return all the names of name-value-pairs in this QualityQuery.
   */
  virtual std::deque<std::wstring> getNames();

  /**
   * Return the value of a certain name-value pair.
   * @param name the name whose value should be returned.
   */
  virtual std::wstring getValue(const std::wstring &name);

  /**
   * Return the ID of this query.
   * The ID allows to map_obj the quality query with its judgements.
   */
  virtual std::wstring getQueryID();

  /* for a nicer sort of input queries before running them.
   * Try first as ints, fall back to string if not int. */
  int compareTo(std::shared_ptr<QualityQuery> other) override;
};

} // #include  "core/src/java/org/apache/lucene/benchmark/quality/
