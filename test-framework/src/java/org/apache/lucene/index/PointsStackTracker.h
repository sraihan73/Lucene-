#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/Cell.h"

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

namespace org::apache::lucene::index
{

/** Simple utility class to track the current BKD stack based solely on calls to
 * {@link IntersectVisitor#compare}. */
class PointsStackTracker
    : public std::enable_shared_from_this<PointsStackTracker>
{

private:
  const int numDims;
  const int bytesPerDim;

public:
  const std::deque<std::shared_ptr<Cell>> stack =
      std::deque<std::shared_ptr<Cell>>();

public:
  class Cell : public std::enable_shared_from_this<Cell>
  {
    GET_CLASS_NAME(Cell)
  private:
    std::shared_ptr<PointsStackTracker> outerInstance;

  public:
    std::deque<char> const minPackedValue;
    std::deque<char> const maxPackedValue;

    Cell(std::shared_ptr<PointsStackTracker> outerInstance,
         std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue);

    virtual bool contains(std::shared_ptr<Cell> other);
  };

public:
  PointsStackTracker(int numDims, int bytesPerDim);

  virtual void onCompare(std::deque<char> &minPackedValue,
                         std::deque<char> &maxPackedValue);

  // TODO: expose other details about the stack...
};

} // #include  "core/src/java/org/apache/lucene/index/
