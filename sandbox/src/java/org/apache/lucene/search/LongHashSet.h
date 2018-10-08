#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <optional>
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
namespace org::apache::lucene::search
{

class LongHashSet final : public AbstractSet<int64_t>
{
  GET_CLASS_NAME(LongHashSet)

private:
  static const int64_t MISSING = std::numeric_limits<int64_t>::min();

public:
  std::deque<int64_t> const table;
  const int mask;
  const bool hasMissingValue;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int size_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int hashCode_;

  LongHashSet(std::deque<int64_t> &values);

private:
  bool add(int64_t l);

public:
  bool contains(int64_t l);

  int size() override;

  virtual int hashCode();

  bool equals(std::any obj) override;

  bool contains(std::any o) override;

  std::shared_ptr<Iterator<int64_t>> iterator() override;

private:
  class IteratorAnonymousInnerClass
      : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
        public Iterator<int64_t>
  {
    GET_CLASS_NAME(IteratorAnonymousInnerClass)
  private:
    std::shared_ptr<LongHashSet> outerInstance;

  public:
    IteratorAnonymousInnerClass(std::shared_ptr<LongHashSet> outerInstance);

  private:
    bool hasNext = false;
    int i = 0;
    int64_t value = 0;

  public:
    bool hasNext();

    std::optional<int64_t> next();
  };

protected:
  std::shared_ptr<LongHashSet> shared_from_this()
  {
    return std::static_pointer_cast<LongHashSet>(
        java.util.AbstractSet<long>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
