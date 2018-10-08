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
namespace org::apache::lucene::util
{

/**
 * An object whose RAM usage can be computed.
 *
 * @lucene.internal
 */
class Accountable
{
  GET_CLASS_NAME(Accountable)

  /**
   * Return the memory usage of this object in bytes. Negative values are
   * illegal.
   */
public:
  virtual int64_t ramBytesUsed() = 0;

  /**
   * Returns nested resources of this class.
   * The result should be a point-in-time snapshot (to avoid race conditions).
   * @see Accountables
   */
  // C++ TODO: There is no equivalent in C++ to Java default interface methods:
  //      default java.util.std::deque<Accountable> getChildResources()
  //  {
  //    return Collections.emptyList();
  //  }
};

} // namespace org::apache::lucene::util
