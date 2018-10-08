#pragma once
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class Codec;
}

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

using Codec = org::apache::lucene::codecs::Codec;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// e.g. run like this: ant test -Dtestcase=Test2BPoints -Dtests.nightly=true
// -Dtests.verbose=true -Dtests.monster=true
//
//   or: python -u /l/util/src/python/repeatLuceneTest.py -heap 6g -once -nolog
//   -tmpDir /b/tmp -logDir /l/logs Test2BPoints.test2D -verbose

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({ "SimpleText", "Memory", "Direct",
// "Compressing" }) @TimeoutSuite(millis = 365 * 24 * TimeUnits.HOUR)
// @Monster("takes at least 4 hours and consumes many GB of temp disk space")
// public class Test2BPoints extends org.apache.lucene.util.LuceneTestCase
class Test2BPoints : public LuceneTestCase
{
public:
  virtual void test1D() ;

  virtual void test2D() ;

private:
  static std::shared_ptr<Codec> getCodec();

protected:
  std::shared_ptr<Test2BPoints> shared_from_this()
  {
    return std::static_pointer_cast<Test2BPoints>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
