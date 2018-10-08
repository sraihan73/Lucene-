#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/ThrottledIndexOutput.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/store/Lock.h"
#include  "core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include  "core/src/java/org/apache/lucene/store/AlreadyClosedException.h"

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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using ThrottledIndexOutput = org::apache::lucene::util::ThrottledIndexOutput;

/**
 * This is a Directory Wrapper that adds methods
 * intended to be used only by unit tests.
 * It also adds a number of features useful for testing:
 * <ul>
 *   <li> Instances created by {@link LuceneTestCase#newDirectory()} are tracked
 *        to ensure they are closed by the test.
 *   <li> When a MockDirectoryWrapper is closed, it will throw an exception if
 *        it has any open files against it (with a stacktrace indicating where
 *        they were opened from).
 *   <li> When a MockDirectoryWrapper is closed, it runs CheckIndex to test if
 *        the index was corrupted.
 *   <li> MockDirectoryWrapper simulates some "features" of Windows, such as
 *        refusing to write/delete to open files.
 * </ul>
 */
class MockDirectoryWrapper : public BaseDirectoryWrapper
{
  GET_CLASS_NAME(MockDirectoryWrapper)
public:
  int64_t maxSize = 0;

  // Max actual bytes used. This is set by MockRAMOutputStream:
  int64_t maxUsedSize = 0;
  double randomIOExceptionRate = 0;
  double randomIOExceptionRateOnOpen = 0;
  std::shared_ptr<Random> randomState;
  bool assertNoDeleteOpenFile = false;
  bool trackDiskUsage = false;
  bool useSlowOpenClosers = LuceneTestCase::TEST_NIGHTLY;
  bool allowRandomFileNotFoundException = true;
  bool allowReadingFilesStillOpenForWrite = false;

private:
  std::shared_ptr<Set<std::wstring>> unSyncedFiles;
  std::shared_ptr<Set<std::wstring>> createdFiles;
  std::shared_ptr<Set<std::wstring>> openFilesForWrite =
      std::unordered_set<std::wstring>();

public:
  std::shared_ptr<ConcurrentMap<std::wstring, std::runtime_error>> openLocks =
      std::make_shared<ConcurrentHashMap<std::wstring, std::runtime_error>>();
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile bool crashed;
  bool crashed = false;

private:
  std::shared_ptr<ThrottledIndexOutput> throttledOutput;
  Throttling throttling =
      LuceneTestCase::TEST_NIGHTLY ? Throttling::SOMETIMES : Throttling::NEVER;

  // for testing
public:
  bool alwaysCorrupt = false;

  const std::shared_ptr<AtomicInteger> inputCloneCount =
      std::make_shared<AtomicInteger>();

  // use this for tracking files for crash.
  // additionally: provides debugging information in case you leave one open
private:
  std::unordered_map<std::shared_ptr<Closeable>, std::runtime_error>
      openFileHandles = Collections::synchronizedMap(
          std::make_shared<IdentityHashMap<std::shared_ptr<Closeable>,
                                           std::runtime_error>>());

  // NOTE: we cannot initialize the Map here due to the
  // order in which our constructor actually does this
  // member initialization vs when it calls super.  It seems
  // like super is called, then our members are initialized:
  std::unordered_map<std::wstring, int> openFiles;

  // Only tracked if noDeleteOpenFile is true: if an attempt
  // is made to delete an open file, we enroll it here.
  std::shared_ptr<Set<std::wstring>> openFilesDeleted;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void init();

public:
  MockDirectoryWrapper(std::shared_ptr<Random> random,
                       std::shared_ptr<Directory> delegate_);

  virtual int getInputCloneCount();

  bool verboseClone = false;

  /**
   * If set to true, we print a fake exception
   * with filename and stacktrace on every indexinput clone()
   */
  virtual void setVerboseClone(bool v);

  virtual void setTrackDiskUsage(bool v);

  /** If set to true (the default), when we throw random
   *  IOException on openInput or createOutput, we may
   *  sometimes throw FileNotFoundException or
   *  NoSuchFileException. */
  virtual void setAllowRandomFileNotFoundException(bool value);

  /** If set to true, you can open an inputstream on a file
   *  that is still open for writes. */
  virtual void setAllowReadingFilesStillOpenForWrite(bool value);

  /**
   * Enum for controlling hard disk throttling.
   * Set via {@link MockDirectoryWrapper #setThrottling(Throttling)}
   * <p>
   * WARNING: can make tests very slow.
   */
public:
  enum class Throttling {
    GET_CLASS_NAME(Throttling)
    /** always emulate a slow hard disk. could be very slow! */
    ALWAYS,
    /** sometimes (0.5% of the time) emulate a slow hard disk. */
    SOMETIMES,
    /** never throttle output */
    NEVER
  };

public:
  virtual void setThrottling(Throttling throttling);

  /**
   * Add a rare small sleep to catch race conditions in open/close
   * <p>
   * You can enable this if you need it.
   */
  virtual void setUseSlowOpenClosers(bool v);

  // C++ WARNING: The following method was originally marked 'synchronized':
  void sync(std::shared_ptr<std::deque<std::wstring>> names) throw(
      IOException) override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void rename(const std::wstring &source,
              const std::wstring &dest)  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void syncMetaData()  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t sizeInBytes() ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void corruptUnknownFiles() ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void corruptFiles(
      std::shared_ptr<std::deque<std::wstring>> files) ;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void _corruptFiles(std::shared_ptr<std::deque<std::wstring>> files) throw(
      IOException);

  /** Simulates a crash of OS or machine by overwriting
   *  unsynced files. */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void crash() ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void clearCrash();

  virtual void setMaxSizeInBytes(int64_t maxSize);
  virtual int64_t getMaxSizeInBytes();

  /**
   * Returns the peek actual storage used (bytes) in this
   * directory.
   */
  virtual int64_t getMaxUsedSizeInBytes();
  virtual void resetMaxUsedSizeInBytes() ;

  /**
   * Trip a test assert if there is an attempt
   * to delete an open file.
   */
  virtual void setAssertNoDeleteOpenFile(bool value);

  virtual bool getAssertNoDeleteOpenFile();

  /**
   * If 0.0, no exceptions will be thrown.  Else this should
   * be a double 0.0 - 1.0.  We will randomly throw an
   * IOException on the first write to an OutputStream based
   * on this probability.
   */
  virtual void setRandomIOExceptionRate(double rate);

  virtual double getRandomIOExceptionRate();

  /**
   * If 0.0, no exceptions will be thrown during openInput
   * and createOutput.  Else this should
   * be a double 0.0 - 1.0 and we will randomly throw an
   * IOException in openInput and createOutput with
   * this probability.
   */
  virtual void setRandomIOExceptionRateOnOpen(double rate);

  virtual double getRandomIOExceptionRateOnOpen();

  virtual void
  maybeThrowIOException(const std::wstring &message) ;

  virtual void
  maybeThrowIOExceptionOnOpen(const std::wstring &name) ;

  /** returns current open file handle count */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int64_t getFileHandleCount();

  // C++ WARNING: The following method was originally marked 'synchronized':
  void deleteFile(const std::wstring &name)  override;

  // sets the cause of the incoming ioe to be the stack
  // trace when the offending file name was opened
private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::runtime_error fillOpenTrace(std::runtime_error t,
                                   const std::wstring &name, bool input);

  void maybeYield();

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<Set<std::wstring>> getOpenDeletedFiles();

private:
  bool failOnCreateOutput = true;

public:
  virtual void setFailOnCreateOutput(bool v);

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<IndexOutput>
  createOutput(const std::wstring &name,
               std::shared_ptr<IOContext> context)  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<IndexOutput> createTempOutput(
      const std::wstring &prefix, const std::wstring &suffix,
      std::shared_ptr<IOContext> context)  override;

private:
  enum class Handle { GET_CLASS_NAME(Handle) Input, Output, Slice };

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void addFileHandle(std::shared_ptr<Closeable> c,
                             const std::wstring &name, Handle handle);

private:
  bool failOnOpenInput = true;

public:
  virtual void setFailOnOpenInput(bool v);

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<IndexInput>
  openInput(const std::wstring &name,
            std::shared_ptr<IOContext> context)  override;

  /** Provided for testing purposes.  Use sizeInBytes() instead. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t getRecomputedSizeInBytes() ;

  /** Like getRecomputedSizeInBytes(), but, uses actual file
   * lengths rather than buffer allocations (which are
   * quantized up to nearest
   * RAMOutputStream.BUFFER_SIZE (now 1024) bytes.
   */

  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t getRecomputedActualSizeInBytes() ;

  // NOTE: This is off by default; see LUCENE-5574
private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool assertNoUnreferencedFilesOnClose;
  bool assertNoUnreferencedFilesOnClose = false;

public:
  virtual void setAssertNoUnrefencedFilesOnClose(bool v);

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual ~MockDirectoryWrapper();

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void removeOpenFile(std::shared_ptr<Closeable> c,
                              const std::wstring &name);

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void removeIndexOutput(std::shared_ptr<IndexOutput> out,
                                 const std::wstring &name);

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void removeIndexInput(std::shared_ptr<IndexInput> in_,
                                const std::wstring &name);

  /**
   * Objects that represent fail-able conditions. Objects of a derived
   * class are created and registered with the mock directory. After
   * register, each object will be invoked once for each first write
   * of a file, giving the object a chance to throw an IOException.
   */
public:
  class Failure : public std::enable_shared_from_this<Failure>
  {
    GET_CLASS_NAME(Failure)
    /**
     * eval is called on the first write of every new file.
     */
  public:
    virtual void
    eval(std::shared_ptr<MockDirectoryWrapper> dir) ;

    /**
     * reset should set the state of the failure to its default
     * (freshly constructed) state. Reset is convenient for tests
     * that want to create one failure object and then reuse it in
     * multiple cases. This, combined with the fact that Failure
     * subclasses are often anonymous classes makes reset difficult to
     * do otherwise.
     *
     * A typical example of use is
     * Failure failure = new Failure() { ... };
     * ...
     * mock.failOn(failure.reset())
     */
    virtual std::shared_ptr<Failure> reset();

  protected:
    bool doFail = false;

  public:
    virtual void setDoFail();

    virtual void clearDoFail();
  };

public:
  std::deque<std::shared_ptr<Failure>> failures;

  /**
   * add a Failure object to the deque of objects to be evaluated
   * at every potential failure point
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void failOn(std::shared_ptr<Failure> fail);

  /**
   * Iterate through the failures deque, giving each object a
   * chance to throw an IOE
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void maybeThrowDeterministicException() ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::deque<std::wstring> listAll()  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t fileLength(const std::wstring &name)  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<Lock>
  obtainLock(const std::wstring &name)  override;

  /** Use this when throwing fake {@code IOException},
   *  e.g. from {@link MockDirectoryWrapper.Failure}. */
public:
  class FakeIOException : public IOException
  {
    GET_CLASS_NAME(FakeIOException)

  protected:
    std::shared_ptr<FakeIOException> shared_from_this()
    {
      return std::static_pointer_cast<FakeIOException>(
          java.io.IOException::shared_from_this());
    }
  };

public:
  virtual std::wstring toString();

  // don't override optional methods like copyFrom: we need the default impl for
  // things like disk full checks. we randomly exercise "raw" directories
  // anyway. We ensure default impls are used:

  std::shared_ptr<ChecksumIndexInput> openChecksumInput(
      const std::wstring &name,
      std::shared_ptr<IOContext> context)  override final;

  void copyFrom(
      std::shared_ptr<Directory> from, const std::wstring &src,
      const std::wstring &dest,
      std::shared_ptr<IOContext> context)  override final;

protected:
  void ensureOpen()  override final;

protected:
  std::shared_ptr<MockDirectoryWrapper> shared_from_this()
  {
    return std::static_pointer_cast<MockDirectoryWrapper>(
        BaseDirectoryWrapper::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
