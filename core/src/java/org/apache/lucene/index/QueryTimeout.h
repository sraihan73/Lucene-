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
namespace org::apache::lucene::index
{

/**
 * Base for query timeout implementations, which will provide a {@code
 * shouldExit()} method, used with {@link ExitableDirectoryReader}.
 */
class QueryTimeout
{
  GET_CLASS_NAME(QueryTimeout)

  /**
   * Called from {@link ExitableDirectoryReader.ExitableTermsEnum#next()}
   * to determine whether to stop processing a query.
   */
public:
  virtual bool shouldExit() = 0;

  /** Returns true if timeouts are enabled for this query (i.e. if shouldExit
   * would ever return true) */
  // C++ TODO: There is no equivalent in C++ to Java default interface methods:
  //      public default bool isTimeoutEnabled()
  //  {
  //    return true;
  //  }
};

} // #include  "core/src/java/org/apache/lucene/index/
