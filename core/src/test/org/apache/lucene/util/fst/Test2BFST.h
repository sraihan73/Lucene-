#pragma once
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::util::fst
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore("Requires tons of heap to run (30 GB hits OOME but 35
// GB passes after ~4.5 hours)") @TimeoutSuite(millis = 100 * TimeUnits.HOUR)
// public class Test2BFST extends org.apache.lucene.util.LuceneTestCase
class Test2BFST : public LuceneTestCase
{

private:
  static int64_t LIMIT;

public:
  virtual void test() ;

private:
  void nextInput(std::shared_ptr<Random> r, std::deque<int> &ints);

protected:
  std::shared_ptr<Test2BFST> shared_from_this()
  {
    return std::static_pointer_cast<Test2BFST>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::fst
