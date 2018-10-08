#pragma once
#include "stringhelper.h"
#include <memory>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/OrdAndValue.h"

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
namespace org::apache::lucene::facet
{

using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/** Keeps highest results, first by largest float value,
 *  then tie break by smallest ord. */
class TopOrdAndFloatQueue
    : public PriorityQueue<std::shared_ptr<TopOrdAndFloatQueue::OrdAndValue>>
{
  GET_CLASS_NAME(TopOrdAndFloatQueue)

  /** Holds a single entry. */
public:
  class OrdAndValue final : public std::enable_shared_from_this<OrdAndValue>
  {
    GET_CLASS_NAME(OrdAndValue)

    /** Ordinal of the entry. */
  public:
    int ord = 0;

    /** Value associated with the ordinal. */
    float value = 0;

    /** Default constructor. */
    OrdAndValue();
  };

  /** Sole constructor. */
public:
  TopOrdAndFloatQueue(int topN);

protected:
  bool lessThan(std::shared_ptr<OrdAndValue> a,
                std::shared_ptr<OrdAndValue> b) override;

protected:
  std::shared_ptr<TopOrdAndFloatQueue> shared_from_this()
  {
    return std::static_pointer_cast<TopOrdAndFloatQueue>(
        org.apache.lucene.util.PriorityQueue<
            TopOrdAndFloatQueue.OrdAndValue>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
