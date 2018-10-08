#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
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

using QualityQuery = org::apache::lucene::benchmark::quality::QualityQuery;

/**
 * Read topics of TREC 1MQ track.
 * <p>
 * Expects this topic format -
 * <pre>
 *   qnum:qtext
 * </pre>
 * Comment lines starting with '#' are ignored.
 * <p>
 * All topics will have a single name value pair.
 */
class Trec1MQReader : public std::enable_shared_from_this<Trec1MQReader>
{
  GET_CLASS_NAME(Trec1MQReader)

private:
  std::wstring name;

  /**
   *  Constructor for Trec's 1MQ TopicsReader
   *  @param name name of name-value pair to set for all queries.
   */
public:
  // C++ TODO: No base class can be determined:
  Trec1MQReader(const std::wstring &name); // super();

  /**
   * Read quality queries from trec 1MQ format topics file.
   * @param reader where queries are read from.
   * @return the result quality queries.
   * @throws IOException if cannot read the queries.
   */
  virtual std::deque<std::shared_ptr<QualityQuery>>
  readQueries(std::shared_ptr<BufferedReader> reader) ;
};

} // namespace org::apache::lucene::benchmark::quality::trec
