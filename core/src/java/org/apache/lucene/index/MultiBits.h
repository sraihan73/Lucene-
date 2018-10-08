#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class Bits;
}

namespace org::apache::lucene::index
{
class ReaderSlice;
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
namespace org::apache::lucene::index
{

using Bits = org::apache::lucene::util::Bits;

/**
 * Concatenates multiple Bits together, on every lookup.
 *
 * <p><b>NOTE</b>: This is very costly, as every lookup must
 * do a binary search to locate the right sub-reader.
 *
 * @lucene.experimental
 */
class MultiBits final : public std::enable_shared_from_this<MultiBits>,
                        public Bits
{
  GET_CLASS_NAME(MultiBits)
private:
  std::deque<std::shared_ptr<Bits>> const subs;

  // length is 1+subs.length (the last entry has the maxDoc):
  std::deque<int> const starts;

  const bool defaultValue;

public:
  MultiBits(std::deque<std::shared_ptr<Bits>> &subs, std::deque<int> &starts,
            bool defaultValue);

private:
  bool checkLength(int reader, int doc);

public:
  bool get(int doc) override;

  virtual std::wstring toString();

  /**
   * Represents a sub-Bits from
   * {@link MultiBits#getMatchingSub(org.apache.lucene.index.ReaderSlice)
   * getMatchingSub()}.
   */
public:
  class SubResult final : public std::enable_shared_from_this<SubResult>
  {
    GET_CLASS_NAME(SubResult)
  public:
    bool matches = false;
    std::shared_ptr<Bits> result;
  };

  /**
   * Returns a sub-Bits matching the provided <code>slice</code>
   * <p>
   * Because <code>null</code> usually has a special meaning for
   * Bits (e.g. no deleted documents), you must check
   * {@link SubResult#matches} instead to ensure the sub was
   * actually found.
   */
public:
  std::shared_ptr<SubResult> getMatchingSub(std::shared_ptr<ReaderSlice> slice);

  int length() override;
};

} // namespace org::apache::lucene::index
