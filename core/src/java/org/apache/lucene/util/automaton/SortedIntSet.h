#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <map_obj>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::automaton
{
class FrozenIntSet;
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
namespace org::apache::lucene::util::automaton
{

// Just holds a set of int[] states, plus a corresponding
// int[] count per state.  Used by
// BasicOperations.determinize
class SortedIntSet final : public std::enable_shared_from_this<SortedIntSet>
{
  GET_CLASS_NAME(SortedIntSet)
public:
  std::deque<int> values;
  std::deque<int> counts;
  int upto = 0;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  int hashCode_ = 0;

  // If we hold more than this many states, we switch from
  // O(N^2) linear ops to O(N log(N)) TreeMap
  static constexpr int TREE_MAP_CUTOVER = 30;

  const std::unordered_map<int, int> map_obj = std::map_obj<int, int>();

  bool useTreeMap = false;

public:
  int state = 0;

  SortedIntSet(int capacity);

  // Adds this state to the set
  void incr(int num);

  // Removes this state from the set, if count decrs to 0
  void decr(int num);

  void computeHash();

  std::shared_ptr<FrozenIntSet> freeze(int state);

  virtual int hashCode();

  bool equals(std::any _other) override;

  virtual std::wstring toString();

public:
  class FrozenIntSet final : public std::enable_shared_from_this<FrozenIntSet>
  {
    GET_CLASS_NAME(FrozenIntSet)
  public:
    std::deque<int> const values;
    // C++ NOTE: Fields cannot have the same name as methods:
    const int hashCode_;
    const int state;

    FrozenIntSet(std::deque<int> &values, int hashCode, int state);

    FrozenIntSet(int num, int state);

    virtual int hashCode();

    bool equals(std::any _other) override;

    virtual std::wstring toString();
  };
};

} // namespace org::apache::lucene::util::automaton
