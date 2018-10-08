#pragma once
#include "stringhelper.h"
#include "tangible_filesystem.h"
#include <memory>
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
namespace org::apache::lucene::analysis::ko::util
{

class ConnectionCostsWriter final
    : public std::enable_shared_from_this<ConnectionCostsWriter>
{
  GET_CLASS_NAME(ConnectionCostsWriter)

private:
  std::deque<std::deque<short>> const
      costs; // array is backward IDs first since get is called using the same
             // backward ID consecutively. maybe doesn't matter.
  const int forwardSize;
  const int backwardSize;
  /**
   * Constructor for building. TODO: remove write access
   */
public:
  // C++ NOTE: The following call to the 'RectangularVectors' helper class
  // reproduces the rectangular array initialization that is automatic in Java:
  // ORIGINAL LINE: public ConnectionCostsWriter(int forwardSize, int
  // backwardSize)
  ConnectionCostsWriter(int forwardSize, int backwardSize);

  void add(int forwardId, int backwardId, int cost);

  void write(const std::wstring &baseDir) ;
};

} // #include  "core/src/java/org/apache/lucene/analysis/ko/util/
