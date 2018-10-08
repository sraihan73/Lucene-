#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::@ mutable { class MutableValue; }

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
namespace org::apache::lucene::util::mutable_
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Simple test of the basic contract of the various {@link MutableValue}
 * implementaitons.
 */
class TestMutableValues : public LuceneTestCase
{
  GET_CLASS_NAME(TestMutableValues)

public:
  virtual void testStr();

  virtual void testDouble();

  virtual void testInt();

  virtual void testFloat();

  virtual void testLong();

  virtual void testBool();

private:
  void assertSanity(std::shared_ptr<MutableValue> x);

  void assertEquality(std::shared_ptr<MutableValue> x,
                      std::shared_ptr<MutableValue> y);

  void assertInEquality(std::shared_ptr<MutableValue> x,
                        std::shared_ptr<MutableValue> y);

protected:
  std::shared_ptr<TestMutableValues> shared_from_this()
  {
    return std::static_pointer_cast<TestMutableValues>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::mutable_
