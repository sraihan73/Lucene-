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
namespace org::apache::lucene::search::vectorhighlight
{

/**
 * Finds fragment boundaries: pluggable into {@link BaseFragmentsBuilder}
 */
class BoundaryScanner
{
  GET_CLASS_NAME(BoundaryScanner)

  /**
   * Scan backward to find end offset.
   * @param buffer scanned object
   * @param start start offset to begin
   * @return the found start offset
   */
public:
  virtual int findStartOffset(std::shared_ptr<StringBuilder> buffer,
                              int start) = 0;

  /**
   * Scan forward to find start offset.
   * @param buffer scanned object
   * @param start start offset to begin
   * @return the found end offset
   */
  virtual int findEndOffset(std::shared_ptr<StringBuilder> buffer,
                            int start) = 0;
};

} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
