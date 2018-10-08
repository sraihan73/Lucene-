#pragma once
#include "stringhelper.h"
#include <cmath>
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
namespace org::apache::lucene::spatial3d::geom
{

/**
 * Static methods globally useful for 3d geometric work.
 *
 * @lucene.experimental
 */
class Tools : public std::enable_shared_from_this<Tools>
{
  GET_CLASS_NAME(Tools)
private:
  Tools();

  /**
   * Java acos yields a NAN if you take an arc-cos of an
   * angle that's just a tiny bit greater than 1.0, so
   * here's a more resilient version.
   */
public:
  static double safeAcos(double value);
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
