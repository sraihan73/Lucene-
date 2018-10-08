#pragma once
#include "stringhelper.h"
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
namespace org::apache::lucene::benchmark
{

/**
 * Various benchmarking constants (mostly defaults)
 **/
class Constants : public std::enable_shared_from_this<Constants>
{
  GET_CLASS_NAME(Constants)

public:
  static constexpr int DEFAULT_RUN_COUNT = 5;
  static constexpr int DEFAULT_SCALE_UP = 5;
  static constexpr int DEFAULT_LOG_STEP = 1000;

  static std::deque<std::optional<bool>> BOOLEANS;

  static const int DEFAULT_MAXIMUM_DOCUMENTS = std::numeric_limits<int>::max();
};

} // namespace org::apache::lucene::benchmark
