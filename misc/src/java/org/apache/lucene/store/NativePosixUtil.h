#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

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
 * Provides JNI access to native methods such as madvise() for
 * {@link NativeUnixDirectory}
 */
class NativePosixUtil final
    : public std::enable_shared_from_this<NativePosixUtil>
{
  GET_CLASS_NAME(NativePosixUtil)
public:
  static constexpr int NORMAL = 0;
  static constexpr int SEQUENTIAL = 1;
  static constexpr int RANDOM = 2;
  static constexpr int WILLNEED = 3;
  static constexpr int DONTNEED = 4;
  static constexpr int NOREUSE = 5;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static NativePosixUtil::StaticConstructor staticConstructor;

  // C++ NOTE: The following Java 'native' declaration was converted using the
  // Microsoft-specific __declspec(dllimport):
  //      private static native int posix_fadvise(java.io.FileDescriptor fd,
  //      long offset, long len, int advise) throws java.io.IOException;
  __declspec(dllimport) static int posix_fadvise(
      std::shared_ptr<FileDescriptor> fd, int64_t offset, int64_t len,
      int advise) ;
  // C++ NOTE: The following Java 'native' declaration was converted using the
  // Microsoft-specific __declspec(dllimport):
  //      public static native int posix_madvise(java.nio.ByteBuffer buf, int
  //      advise) throws java.io.IOException;
public:
  __declspec(dllimport) static int posix_madvise(
      std::shared_ptr<ByteBuffer> buf, int advise) ;
  // C++ NOTE: The following Java 'native' declaration was converted using the
  // Microsoft-specific __declspec(dllimport):
  //      public static native int madvise(java.nio.ByteBuffer buf, int advise)
  //      throws java.io.IOException;
  __declspec(dllimport) static int madvise(std::shared_ptr<ByteBuffer> buf,
                                           int advise) ;
  // C++ NOTE: The following Java 'native' declaration was converted using the
  // Microsoft-specific __declspec(dllimport):
  //      public static native java.io.FileDescriptor open_direct(std::wstring
  //      filename, bool read) throws java.io.IOException;
  __declspec(dllimport) static std::shared_ptr<FileDescriptor> open_direct(
      const std::wstring &filename, bool read) ;
  // C++ NOTE: The following Java 'native' declaration was converted using the
  // Microsoft-specific __declspec(dllimport):
  //      public static native long pread(java.io.FileDescriptor fd, long pos,
  //      java.nio.ByteBuffer byteBuf) throws java.io.IOException;
  __declspec(dllimport) static int64_t pread(
      std::shared_ptr<FileDescriptor> fd, int64_t pos,
      std::shared_ptr<ByteBuffer> byteBuf) ;

  static void advise(std::shared_ptr<FileDescriptor> fd, int64_t offset,
                     int64_t len, int advise) ;
};

} // #include  "core/src/java/org/apache/lucene/store/
