#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"

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

/**
 * <p>
 * Merge policy for testing, it is like an alcoholic.
 * It drinks (merges) at night, and randomly decides what to drink.
 * During the daytime it sleeps.
 * </p>
 * <p>
 * if tests pass with this, then they are likely to pass with any
 * bizarro merge policy users might write.
 * </p>
 * <p>
 * It is a fine bottle of champagne (Ordered by Martijn).
 * </p>
 */
class AlcoholicMergePolicy : public LogMergePolicy
{
  GET_CLASS_NAME(AlcoholicMergePolicy)

private:
  const std::shared_ptr<Random> random;
  const std::shared_ptr<Calendar> calendar;

public:
  AlcoholicMergePolicy(std::shared_ptr<TimeZone> tz,
                       std::shared_ptr<Random> random);

protected:
  int64_t
  size(std::shared_ptr<SegmentCommitInfo> info,
       std::shared_ptr<MergeContext> mergeContext)  override;

private:
  class Drink final
  {
    GET_CLASS_NAME(Drink)

  public:
    static Drink Beer;
    static Drink Wine;
    static Drink Champagne;
    static Drink WhiteRussian;
    static Drink SingleMalt;

  private:
    static std::deque<Drink> valueList;

    class StaticConstructor
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

    static StaticConstructor staticConstructor;

  public:
    enum class InnerEnum {
      GET_CLASS_NAME(InnerEnum) Beer,
      Wine,
      Champagne,
      WhiteRussian,
      SingleMalt
    };

    const InnerEnum innerEnumValue;

  private:
    const std::wstring nameValue;
    const int ordinalValue;
    static int nextOrdinal;

  public:
    int64_t drunkFactor = 0;

    Drink(const std::wstring &name, InnerEnum innerEnum,
          std::shared_ptr<AlcoholicMergePolicy> outerInstance,
          int64_t drunkFactor);

  public:
    bool operator==(const Drink &other);

    bool operator!=(const Drink &other);

    static std::deque<Drink> values();

    int ordinal();

    std::wstring toString();

    static Drink valueOf(const std::wstring &name);
  };

protected:
  std::shared_ptr<AlcoholicMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<AlcoholicMergePolicy>(
        LogMergePolicy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
