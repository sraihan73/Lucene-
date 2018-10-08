#pragma once
#include "stringhelper.h"
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
namespace org::apache::lucene::search::join
{

/**
 * How to aggregate multiple child hit scores into a single parent score.
 */
enum class ScoreMode {
  GET_CLASS_NAME(ScoreMode)

  /**
   * Do no scoring.
   */
  None,

  /**
   * Parent hit's score is the average of all child scores.
   */
  Avg,

  /**
   * Parent hit's score is the max of all child scores.
   */
  Max,

  /**
   * Parent hit's score is the sum of all child scores.
   */
  Total,

  /**
   * Parent hit's score is the min of all child scores.
   */
  Min

};

} // #include  "core/src/java/org/apache/lucene/search/join/
