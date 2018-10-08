#pragma once
#include "stringhelper.h"
#include <algorithm>
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
namespace org::apache::lucene::mockfile
{

/**
 * Gives an unpredictable, but deterministic order to directory listings.
 * <p>
 * This can be useful if for instance, you have build servers on
 * linux but developers are using macs.
 */
class ShuffleFS : public FilterFileSystemProvider
{
  GET_CLASS_NAME(ShuffleFS)
public:
  const int64_t seed;

  /**
   * Create a new instance, wrapping {@code delegate}.
   */
  ShuffleFS(std::shared_ptr<FileSystem> delegate_, int64_t seed);

  template <typename T1>
  // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
  // ORIGINAL LINE: @Override public
  // java.nio.file.DirectoryStream<java.nio.file.Path>
  // newDirectoryStream(java.nio.file.Path dir,
  // java.nio.file.DirectoryStream.Filter<? super java.nio.file.Path> filter)
  // throws java.io.IOException
  std::shared_ptr<DirectoryStream<std::shared_ptr<Path>>> newDirectoryStream(
      std::shared_ptr<Path> dir,
      std::shared_ptr<DirectoryStream::Filter<T1>> filter) ;

private:
  class DirectoryStreamAnonymousInnerClass
      : public DirectoryStream<std::shared_ptr<Path>>
  {
    GET_CLASS_NAME(DirectoryStreamAnonymousInnerClass)
  private:
    std::shared_ptr<ShuffleFS> outerInstance;

    std::deque<std::shared_ptr<Path>> contents;

  public:
    DirectoryStreamAnonymousInnerClass(
        std::shared_ptr<ShuffleFS> outerInstance,
        std::deque<std::shared_ptr<Path>> &contents);

    std::shared_ptr<Iterator<std::shared_ptr<Path>>> iterator() override;
    virtual ~DirectoryStreamAnonymousInnerClass();

  protected:
    std::shared_ptr<DirectoryStreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DirectoryStreamAnonymousInnerClass>(
          java.nio.file
              .DirectoryStream<java.nio.file.Path>::shared_from_this());
    }
  };

protected:
  std::shared_ptr<ShuffleFS> shared_from_this()
  {
    return std::static_pointer_cast<ShuffleFS>(
        FilterFileSystemProvider::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/mockfile/
