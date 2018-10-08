#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
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
namespace org::apache::lucene::benchmark::byTask::programmatic
{

/**
 * Sample performance test written programmatically - no algorithm file is
 * needed here.
 */
class Sample : public std::enable_shared_from_this<Sample>
{
  GET_CLASS_NAME(Sample)

  static void main(std::deque<std::wstring> &args) ;

  // Sample programmatic settings. Could also read from file.
private:
  static std::shared_ptr<Properties> initProps();
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/programmatic/
