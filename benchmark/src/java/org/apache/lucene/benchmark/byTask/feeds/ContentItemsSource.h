#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask::utils
{
class Config;
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
namespace org::apache::lucene::benchmark::byTask::feeds
{

using Config = org::apache::lucene::benchmark::byTask::utils::Config;

/**
 * Base class for source of data for benchmarking
 * <p>
 * Keeps track of various statistics, such as how many data items were
 * generated, size in bytes etc. <p> Supports the following configuration
 * parameters: <ul> <li><b>content.source.forever</b> - specifies whether to
 * generate items forever (<b>default=true</b>).
 * <li><b>content.source.verbose</b> - specifies whether messages should be
 * output by the content source (<b>default=false</b>).
 * <li><b>content.source.encoding</b> - specifies which encoding to use when
 * reading the files of that content source. Certain implementations may define
 * a default value if this parameter is not specified. (<b>default=null</b>).
 * <li><b>content.source.log.step</b> - specifies for how many items a
 * message should be logged. If set to 0 it means no logging should occur.
 * <b>NOTE:</b> if verbose is set to false, logging should not occur even if
 * logStep is not 0 (<b>default=0</b>).
 * </ul>
 */
class ContentItemsSource
    : public std::enable_shared_from_this<ContentItemsSource>
{
  GET_CLASS_NAME(ContentItemsSource)

private:
  int64_t bytesCount = 0;
  int64_t totalBytesCount = 0;
  int itemCount = 0;
  int totalItemCount = 0;
  std::shared_ptr<Config> config;

  int lastPrintedNumUniqueTexts = 0;
  int64_t lastPrintedNumUniqueBytes = 0;
  int printNum = 0;

protected:
  bool forever = false;
  int logStep = 0;
  bool verbose = false;
  std::wstring encoding;

  /** update count of bytes generated by this source */
  // C++ WARNING: The following method was originally marked 'synchronized':
  void addBytes(int64_t numBytes);

  /** update count of items generated by this source */
  // C++ WARNING: The following method was originally marked 'synchronized':
  void addItem();

  /**
   * A convenience method for collecting all the files of a content source from
   * a given directory. The collected {@link Path} instances are stored in the
   * given <code>files</code>.
   */
  void
  collectFiles(std::shared_ptr<Path> dir,
               std::deque<std::shared_ptr<Path>> &files) ;

private:
  class SimpleFileVisitorAnonymousInnerClass
      : public SimpleFileVisitor<std::shared_ptr<Path>>
  {
    GET_CLASS_NAME(SimpleFileVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<ContentItemsSource> outerInstance;

    std::deque<std::shared_ptr<Path>> files;

  public:
    SimpleFileVisitorAnonymousInnerClass(
        std::shared_ptr<ContentItemsSource> outerInstance,
        std::deque<std::shared_ptr<Path>> &files);

    std::shared_ptr<FileVisitResult> visitFile(
        std::shared_ptr<Path> file,
        std::shared_ptr<BasicFileAttributes> attrs)  override;

  protected:
    std::shared_ptr<SimpleFileVisitorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleFileVisitorAnonymousInnerClass>(
          java.nio.file
              .SimpleFileVisitor<java.nio.file.Path>::shared_from_this());
    }
  };

  /**
   * Returns true whether it's time to log a message (depending on verbose and
   * the number of items generated).
   */
protected:
  bool shouldLog();

  /** Called when reading from this content source is no longer required. */
public:
  void close() = 0;
  override

      /** Returns the number of bytes generated since last reset. */
      int64_t
      getBytesCount();

  /** Returns the number of generated items since last reset. */
  int getItemsCount();

  std::shared_ptr<Config> getConfig();

  /** Returns the total number of bytes that were generated by this source. */
  int64_t getTotalBytesCount();

  /** Returns the total number of generated items. */
  int getTotalItemsCount();

  /**
   * Resets the input for this content source, so that the test would behave as
   * if it was just started, input-wise.
   * <p>
   * <b>NOTE:</b> the default implementation resets the number of bytes and
   * items generated since the last reset, so it's important to call
   * super.resetInputs in case you override this method.
   */
  virtual void resetInputs() ;

  /**
   * Sets the {@link Config} for this content source. If you override this
   * method, you must call super.setConfig.
   */
  virtual void setConfig(std::shared_ptr<Config> config);

  virtual void printStatistics(const std::wstring &itemsName);
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
