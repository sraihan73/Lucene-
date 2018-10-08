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
namespace org::apache::lucene
{

/** Lucene's package information, including version. **/
class LucenePackage final : public std::enable_shared_from_this<LucenePackage>
{
  GET_CLASS_NAME(LucenePackage)

private:
  LucenePackage();

  /** Return Lucene's package, including version information. */
public:
  static std::shared_ptr<Package> get();
};

} // #include  "core/src/java/org/apache/lucene/
