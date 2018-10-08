#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::store
{
class IndexInput;
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
namespace org::apache::lucene::store
{

/**
 * Tests MMapDirectory's MultiMMapIndexInput
 * <p>
 * Because Java's ByteBuffer uses an int to address the
 * values, it's necessary to access a file &gt;
 * Integer.MAX_VALUE in size using multiple byte buffers.
 */
class TestMultiMMap : public BaseDirectoryTestCase
{
  GET_CLASS_NAME(TestMultiMMap)

protected:
  std::shared_ptr<Directory>
  getDirectory(std::shared_ptr<Path> path)  override;

public:
  void setUp()  override;

  virtual void testCloneSafety() ;

  virtual void testCloneClose() ;

  virtual void testCloneSliceSafety() ;

  virtual void testCloneSliceClose() ;

  virtual void testSeekZero() ;

  virtual void testSeekSliceZero() ;

  virtual void testSeekEnd() ;

  virtual void testSeekSliceEnd() ;

  virtual void testSeeking() ;

  // note instead of seeking to offset and reading length, this opens slices at
  // the the various offset+length and just does readBytes.
  virtual void testSlicedSeeking() ;

  void testSliceOfSlice()  override;

private:
  void assertSlice(std::deque<char> &bytes, std::shared_ptr<IndexInput> slicer,
                   int outerSliceStart, int sliceStart,
                   int sliceLength) ;

public:
  virtual void testRandomChunkSizes() ;

private:
  void assertChunking(std::shared_ptr<Random> random,
                      int chunkSize) ;

public:
  virtual void testImplementations() ;

protected:
  std::shared_ptr<TestMultiMMap> shared_from_this()
  {
    return std::static_pointer_cast<TestMultiMMap>(
        BaseDirectoryTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
