#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs::compressing
{
class CompressionMode;
}

namespace org::apache::lucene::codecs::compressing
{
class Compressor;
}
namespace org::apache::lucene::codecs::compressing
{
class Decompressor;
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
namespace org::apache::lucene::codecs::compressing
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class AbstractTestCompressionMode : public LuceneTestCase
{
  GET_CLASS_NAME(AbstractTestCompressionMode)

public:
  std::shared_ptr<CompressionMode> mode;

  static std::deque<char> randomArray();

  static std::deque<char> randomArray(int length, int max);

  virtual std::deque<char> compress(std::deque<char> &decompressed, int off,
                                     int len) ;

  static std::deque<char> compress(std::shared_ptr<Compressor> compressor,
                                    std::deque<char> &decompressed, int off,
                                    int len) ;

  virtual std::deque<char> decompress(std::deque<char> &compressed,
                                       int originalLength) ;

  static std::deque<char>
  decompress(std::shared_ptr<Decompressor> decompressor,
             std::deque<char> &compressed,
             int originalLength) ;

  virtual std::deque<char> decompress(std::deque<char> &compressed,
                                       int originalLength, int offset,
                                       int length) ;

  virtual void testDecompress() ;

  virtual void testPartialDecompress() ;

  virtual std::deque<char>
  test(std::deque<char> &decompressed) ;

  virtual std::deque<char> test(std::deque<char> &decompressed, int off,
                                 int len) ;

  virtual void testEmptySequence() ;

  virtual void testShortSequence() ;

  virtual void testIncompressible() ;

  virtual void testConstant() ;

  virtual void testLUCENE5201() ;

protected:
  std::shared_ptr<AbstractTestCompressionMode> shared_from_this()
  {
    return std::static_pointer_cast<AbstractTestCompressionMode>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};
} // namespace org::apache::lucene::codecs::compressing
