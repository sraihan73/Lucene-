#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/LockFactory.h"

#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"

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
 * Native {@link Directory} implementation for Microsoft Windows.
 * <p>
 * Steps:
 * <ol>
 *   <li>Compile the source code to create WindowsDirectory.dll:
 *       <blockquote>
 * c:\mingw\bin\g++ -Wall -D_JNI_IMPLEMENTATION_ -Wl,--kill-at
 * -I"%JAVA_HOME%\include" -I"%JAVA_HOME%\include\win32" -static-libgcc
 * -static-libstdc++ -shared WindowsDirectory.cpp -o WindowsDirectory.dll
 *       </blockquote>
 *       For 64-bit JREs, use mingw64, with the -m64 option.
 *   <li>Put WindowsDirectory.dll into some directory in your windows PATH
 *   <li>Open indexes with WindowsDirectory and use it.
 * </ol>
 * @lucene.experimental
 */
class WindowsDirectory : public FSDirectory
{
  GET_CLASS_NAME(WindowsDirectory)
private:
  static constexpr int DEFAULT_BUFFERSIZE =
      4096; // default pgsize on ia32/amd64

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static WindowsDirectory::StaticConstructor staticConstructor;

  /** Create a new WindowsDirectory for the named location.
   *
   * @param path the path of the directory
   * @param lockFactory the lock factory to use
   * @throws IOException If there is a low-level I/O error
   */
public:
  WindowsDirectory(std::shared_ptr<Path> path,
                   std::shared_ptr<LockFactory> lockFactory) ;

  /** Create a new WindowsDirectory for the named location and {@link
   * FSLockFactory#getDefault()}.
   *
   * @param path the path of the directory
   * @throws IOException If there is a low-level I/O error
   */
  WindowsDirectory(std::shared_ptr<Path> path) ;

  std::shared_ptr<IndexInput>
  openInput(const std::wstring &name,
            std::shared_ptr<IOContext> context)  override;

public:
  class WindowsIndexInput : public BufferedIndexInput
  {
    GET_CLASS_NAME(WindowsIndexInput)
  private:
    const int64_t fd;
    // C++ NOTE: Fields cannot have the same name as methods:
    const int64_t length_;

  public:
    bool isClone = false;
    bool isOpen = false;

    // C++ TODO: There is no native C++ equivalent to 'toString':
    WindowsIndexInput(std::shared_ptr<Path> file,
                      int bufferSize) ;

  protected:
    void readInternal(std::deque<char> &b, int offset,
                      int length)  override;

    void seekInternal(int64_t pos)  override;

  public:
    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual ~WindowsIndexInput();

    int64_t length() override;

    std::shared_ptr<WindowsIndexInput> clone() override;

  protected:
    std::shared_ptr<WindowsIndexInput> shared_from_this()
    {
      return std::static_pointer_cast<WindowsIndexInput>(
          BufferedIndexInput::shared_from_this());
    }
  };

  /** Opens a handle to a file. */
  // C++ NOTE: The following Java 'native' declaration was converted using the
  // Microsoft-specific __declspec(dllimport):
  //      private static native long open(std::wstring filename) throws
  //      java.io.IOException;
private:
  __declspec(dllimport) static int64_t open(
      const std::wstring &filename) ;

  /** Reads data from a file at pos into bytes */
  // C++ NOTE: The following Java 'native' declaration was converted using the
  // Microsoft-specific __declspec(dllimport):
  //      private static native int read(long fd, byte bytes[], int offset, int
  //      length, long pos) throws java.io.IOException;
  __declspec(dllimport) static int read(int64_t fd, std::deque<char> &bytes,
                                        int offset, int length,
                                        int64_t pos) ;

  /** Closes a handle to a file */
  // C++ NOTE: The following Java 'native' declaration was converted using the
  // Microsoft-specific __declspec(dllimport):
  //      private static native void close(long fd) throws java.io.IOException;
  __declspec(dllimport) static void close(int64_t fd) ;

  /** Returns the length of a file */
  // C++ NOTE: The following Java 'native' declaration was converted using the
  // Microsoft-specific __declspec(dllimport):
  //      private static native long length(long fd) throws java.io.IOException;
  __declspec(dllimport) static int64_t length(int64_t fd) throw(
      IOException);

protected:
  std::shared_ptr<WindowsDirectory> shared_from_this()
  {
    return std::static_pointer_cast<WindowsDirectory>(
        FSDirectory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
