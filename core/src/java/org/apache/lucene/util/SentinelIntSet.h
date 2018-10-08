#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::util
{

/**
 * A native int hash-based set where one value is reserved to mean "EMPTY"
 * internally. The space overhead is fairly low as there is only one
 * power-of-two sized int[] to hold the values.  The set is re-hashed when
 * adding a value that would make it &gt;= 75% full.  Consider extending and
 * over-riding {@link #hash(int)} if the values might be poor hash keys; Lucene
 * docids should be fine. The internal fields are exposed publicly to enable
 * more efficient use at the expense of better O-O principles. <p> To iterate
 * over the integers held in this set, simply use code like this: <pre
 * class="prettyprint"> SentinelIntSet set = ... for (int v : set.keys) { if (v
 * == set.emptyVal) continue;
 *   //use v...
 * }</pre>
 *
 * @lucene.internal
 */
class SentinelIntSet : public std::enable_shared_from_this<SentinelIntSet>
{
  GET_CLASS_NAME(SentinelIntSet)
  /** A power-of-2 over-sized array holding the integers in the set along with
   * empty values. */
public:
  std::deque<int> keys;
  int count = 0;
  const int emptyVal;
  /** the count at which a rehash should be done */
  int rehashCount = 0;

  /**
   *
   * @param size  The minimum number of elements this set should be able to hold
   * without rehashing (i.e. the slots are guaranteed not to change)
   * @param emptyVal The integer value to use for EMPTY
   */
  SentinelIntSet(int size, int emptyVal);

  virtual void clear();

  /** (internal) Return the hash for the key. The default implementation just
   * returns the key, which is not appropriate for general purpose use.
   */
  virtual int hash(int key);

  /** The number of integers in this set. */
  virtual int size();

  /** (internal) Returns the slot for this key */
  virtual int getSlot(int key);

  /** (internal) Returns the slot for this key, or -slot-1 if not found */
  virtual int find(int key);

  /** Does this set contain the specified integer? */
  virtual bool exists(int key);

  /** Puts this integer (key) in the set, and returns the slot index it was
   * added to. It rehashes if adding it would make the set more than 75% full.
   */
  virtual int put(int key);

  /** (internal) Rehashes by doubling {@code int[] key} and filling with the old
   * values. */
  virtual void rehash();

  /** Return the memory footprint of this class in bytes. */
  virtual int64_t ramBytesUsed();
};

} // namespace org::apache::lucene::util
