#pragma once
#include "stringhelper.h"
#include <memory>

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

/** time unit constants for use in annotations. */
class TimeUnits final : public std::enable_shared_from_this<TimeUnits>
{
  GET_CLASS_NAME(TimeUnits)
private:
  TimeUnits();

  /** 1 second in milliseconds */
public:
  static constexpr int SECOND = 1000;
  /** 1 minute in milliseconds */
  static const int MINUTE = 60 * SECOND;
  /** 1 hour in milliseconds */
  static const int HOUR = 60 * MINUTE;
};

} // #include  "core/src/java/org/apache/lucene/util/
