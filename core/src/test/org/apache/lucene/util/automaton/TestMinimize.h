#pragma once
#include "stringhelper.h"
#include <limits>
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
namespace org::apache::lucene::util::automaton
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * This test builds some randomish NFA/DFA and minimizes them.
 */
class TestMinimize : public LuceneTestCase
{
  GET_CLASS_NAME(TestMinimize)
  /** the minimal and non-minimal are compared to ensure they are the same. */
public:
  virtual void testBasic();

  /** compare minimized against minimized with a slower, simple impl.
   * we check not only that they are the same, but that #states/#transitions
   * are the same. */
  virtual void testAgainstBrzozowski();

  /** n^2 space usage in Hopcroft minimization? */
  virtual void testMinimizeHuge();

protected:
  std::shared_ptr<TestMinimize> shared_from_this()
  {
    return std::static_pointer_cast<TestMinimize>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/automaton/
