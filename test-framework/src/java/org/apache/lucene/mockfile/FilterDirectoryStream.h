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
namespace org::apache::lucene::mockfile
{

/**
 * A {@code FilterDirectoryStream} contains another
 * {@code DirectoryStream}, which it uses as its basic
 * source of data, possibly transforming the data along the
 * way or providing additional functionality.
 */
class FilterDirectoryStream
    : public std::enable_shared_from_this<FilterDirectoryStream>,
      public DirectoryStream<std::shared_ptr<Path>>
{
  GET_CLASS_NAME(FilterDirectoryStream)

  /**
   * The underlying {@code DirectoryStream} instance.
   */
protected:
  const std::shared_ptr<DirectoryStream<std::shared_ptr<Path>>> delegate_;

  /**
   * The underlying {@code FileSystem} instance.
   */
  const std::shared_ptr<FileSystem> fileSystem;

  /**
   * Construct a {@code FilterDirectoryStream} based on
   * the specified base stream.
   * <p>
   * Note that base stream is closed if this stream is closed.
   * @param delegate specified base stream.
   */
public:
  FilterDirectoryStream(
      std::shared_ptr<DirectoryStream<std::shared_ptr<Path>>> delegate_,
      std::shared_ptr<FileSystem> fileSystem);

  virtual ~FilterDirectoryStream();

  std::shared_ptr<Iterator<std::shared_ptr<Path>>> iterator() override;

private:
  class IteratorAnonymousInnerClass
      : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
        public Iterator<std::shared_ptr<Path>>
  {
    GET_CLASS_NAME(IteratorAnonymousInnerClass)
  private:
    std::shared_ptr<FilterDirectoryStream> outerInstance;

    DirectoryStream<std::shared_ptr<Path>>::const_iterator delegateIterator;

  public:
    IteratorAnonymousInnerClass(
        std::shared_ptr<FilterDirectoryStream> outerInstance,
        DirectoryStream<Path>::const_iterator delegateIterator);

    bool hasNext();
    std::shared_ptr<Path> next();
    void remove();
  };
};

} // namespace org::apache::lucene::mockfile
