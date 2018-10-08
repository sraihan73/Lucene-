#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class InetAddressRange;
}

namespace org::apache::lucene::search
{
class Query;
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
namespace org::apache::lucene::search
{

using InetAddressRange = org::apache::lucene::document::InetAddressRange;

/**
 * Random testing for {@link InetAddressRange}
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({"Direct", "Memory"}) public class
// TestInetAddressRangeQueries extends BaseRangeFieldQueryTestCase
class TestInetAddressRangeQueries : public BaseRangeFieldQueryTestCase
{
private:
  static const std::wstring FIELD_NAME;

protected:
  std::shared_ptr<Range>
  nextRange(int dimensions)  override;

  /** return random IPv4 or IPv6 address */
private:
  std::shared_ptr<InetAddress> nextInetaddress() ;

public:
  void testRandomTiny()  override;

  void testMultiValued()  override;

  void testRandomMedium()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly @Override public void testRandomBig() throws
  // Exception
  void testRandomBig()  override;

  /** return random range */
protected:
  std::shared_ptr<InetAddressRange>
  newRangeField(std::shared_ptr<Range> r) override;

  /** return random intersects query */
  std::shared_ptr<Query> newIntersectsQuery(std::shared_ptr<Range> r) override;

  /** return random contains query */
  std::shared_ptr<Query> newContainsQuery(std::shared_ptr<Range> r) override;

  /** return random within query */
  std::shared_ptr<Query> newWithinQuery(std::shared_ptr<Range> r) override;

  /** return random crosses query */
  std::shared_ptr<Query> newCrossesQuery(std::shared_ptr<Range> r) override;

  /** encapsulated IpRange for test validation */
private:
  class IpRange : public Range
  {
    GET_CLASS_NAME(IpRange)
  private:
    std::shared_ptr<TestInetAddressRangeQueries> outerInstance;

  public:
    std::shared_ptr<InetAddress> minAddress;
    std::shared_ptr<InetAddress> maxAddress;
    std::deque<char> min;
    std::deque<char> max;

    IpRange(std::shared_ptr<TestInetAddressRangeQueries> outerInstance,
            std::shared_ptr<InetAddress> min, std::shared_ptr<InetAddress> max);

  protected:
    int numDimensions() override;

    std::shared_ptr<InetAddress> getMin(int dim) override;

    void setMin(int dim, std::any val) override;

    std::shared_ptr<InetAddress> getMax(int dim) override;

    void setMax(int dim, std::any val) override;

    bool isEqual(std::shared_ptr<Range> o) override;

    bool isDisjoint(std::shared_ptr<Range> o) override;

    bool isWithin(std::shared_ptr<Range> o) override;

    bool contains(std::shared_ptr<Range> o) override;

  public:
    virtual std::wstring toString();

  protected:
    std::shared_ptr<IpRange> shared_from_this()
    {
      return std::static_pointer_cast<IpRange>(Range::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestInetAddressRangeQueries> shared_from_this()
  {
    return std::static_pointer_cast<TestInetAddressRangeQueries>(
        BaseRangeFieldQueryTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
