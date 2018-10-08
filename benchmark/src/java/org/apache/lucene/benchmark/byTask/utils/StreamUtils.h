#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
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
namespace org::apache::lucene::benchmark::byTask::utils
{

/**
 * Stream utilities.
 */
class StreamUtils : public std::enable_shared_from_this<StreamUtils>
{
  GET_CLASS_NAME(StreamUtils)

  /** Buffer size used across the benchmark package */
public:
  static const int BUFFER_SIZE = 1 << 16; // 64K

  /** File format type */
public:
  class Type final
  {
    GET_CLASS_NAME(Type)
    /** BZIP2 is automatically used for <b>.bz2</b> and <b>.bzip2</b>
     * extensions. */
  public:
    static Type BZIP2;
    /** GZIP is automatically used for <b>.gz</b> and <b>.gzip</b> extensions.
     */
    static Type GZIP;
    /** Plain text is used for anything which is not GZIP or BZIP. */
    static Type PLAIN;

  private:
    static std::deque<Type> valueList;

    class StaticConstructor
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

    static StaticConstructor staticConstructor;

  public:
    enum class InnerEnum { GET_CLASS_NAME(InnerEnum) BZIP2, GZIP, PLAIN };

    const InnerEnum innerEnumValue;

  private:
    const std::wstring nameValue;
    const int ordinalValue;
    static int nextOrdinal;

  private:
    const std::wstring csfType;

  public:
    Type(const std::wstring &name, InnerEnum innerEnum,
         std::shared_ptr<StreamUtils> outerInstance,
         const std::wstring &csfType);

  private:
    std::shared_ptr<java::io::InputStream> inputStream(
        std::shared_ptr<java::io::InputStream> in_) ;
    std::shared_ptr<java::io::OutputStream> outputStream(
        std::shared_ptr<java::io::OutputStream> os) ;

  public:
    bool operator==(const Type &other);

    bool operator!=(const Type &other);

    static std::deque<Type> values();

    int ordinal();

    std::wstring toString();

    static Type valueOf(const std::wstring &name);
  };

private:
  static const std::unordered_map<std::wstring, Type> extensionToType;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static StreamUtils::StaticConstructor staticConstructor;

  /**
   * Returns an {@link InputStream} over the requested file. This method
   * attempts to identify the appropriate {@link InputStream} instance to return
   * based on the file name (e.g., if it ends with .bz2 or .bzip, return a
   * 'bzip' {@link InputStream}).
   */
public:
  static std::shared_ptr<InputStream>
  inputStream(std::shared_ptr<Path> file) ;

  /** Return the type of the file, or null if unknown */
private:
  static Type fileType(std::shared_ptr<Path> file);

  /**
   * Returns an {@link OutputStream} over the requested file, identifying
   * the appropriate {@link OutputStream} instance similar to {@link
   * #inputStream(Path)}.
   */
public:
  static std::shared_ptr<OutputStream>
  outputStream(std::shared_ptr<Path> file) ;
};

} // namespace org::apache::lucene::benchmark::byTask::utils
