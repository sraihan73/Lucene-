#pragma once
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
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
namespace org::apache::lucene::util
{

class TestSmallFloat : public LuceneTestCase
{
  GET_CLASS_NAME(TestSmallFloat)

  // original lucene byteToFloat
public:
  static float orig_byteToFloat(char b);

  // original lucene floatToByte (since lucene 1.3)
  static char orig_floatToByte_v13(float f);

  // This is the original lucene floatToBytes (from v1.3)
  // except with the underflow detection bug fixed for values
  // like 5.8123817E-10f
  static char orig_floatToByte(float f);

  virtual void testByteToFloat();

  virtual void testFloatToByte();

  virtual void testInt4();

  virtual void testByte4();

  /***
  // Do an exhaustive test of all possible floating point values
  // for the 315 float against the original norm encoding in Similarity.
  // Takes 75 seconds on my Pentium4 3GHz, with Java5 -server
  public void testAllFloats() {
    for(int i = Integer.MIN_VALUE;;i++) {
      float f = Float.intBitsToFloat(i);
      if (f==f) { // skip non-numbers
        byte b1 = orig_floatToByte(f);
        byte b2 = SmallFloat.floatToByte315(f);
        if (b1!=b2 || b2==0 && f>0) {
          fail("Failed floatToByte315 for float " + f + " source
  bits="+Integer.toHexString(i) + " float raw bits=" +
  Integer.toHexString(Float.floatToRawIntBits(i)));
        }
      }
      if (i==Integer.MAX_VALUE) break;
    }
  }
  ***/

protected:
  std::shared_ptr<TestSmallFloat> shared_from_this()
  {
    return std::static_pointer_cast<TestSmallFloat>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
