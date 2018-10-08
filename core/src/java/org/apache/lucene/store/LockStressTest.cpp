using namespace std;

#include "LockStressTest.h"

namespace org::apache::lucene::store
{
using org::apache::lucene::util::SuppressForbidden;
const wstring LockStressTest::LOCK_FILE_NAME = L"test.lock";

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command line
// tool") @SuppressWarnings("try") public static void main(std::wstring[] args) throws
// Exception
void LockStressTest::main(std::deque<wstring> &args) 
{
  if (args.size() != 7) {
    wcout
        << L"Usage: java org.apache.lucene.store.LockStressTest myID "
           L"verifierHost verifierPort lockFactoryClassName lockDirName "
           L"sleepTimeMS count\n"
        << L"\n"
        << L"  myID = int from 0 .. 255 (should be unique for test process)\n"
        << L"  verifierHost = hostname that LockVerifyServer is listening on\n"
        << L"  verifierPort = port that LockVerifyServer is listening on\n"
        << L"  lockFactoryClassName = primary FSLockFactory class that we will "
           L"use\n"
        << L"  lockDirName = path to the lock directory\n"
        << L"  sleepTimeMS = milliseconds to pause betweeen each lock "
           L"obtain/release\n"
        << L"  count = number of locking tries\n"
        << L"\n"
        << L"You should run multiple instances of this process, each with its "
           L"own\n"
        << L"unique ID, and each pointing to the same lock directory, to "
           L"verify\n"
        << L"that locking is working correctly.\n"
        << L"\n"
        << L"Make sure you are first running LockVerifyServer." << endl;
    exit(1);
  }

  int arg = 0;
  constexpr int myID = stoi(args[arg++]);

  if (myID < 0 || myID > 255) {
    wcout << L"myID must be a unique int 0..255" << endl;
    exit(1);
  }

  const wstring verifierHost = args[arg++];
  constexpr int verifierPort = stoi(args[arg++]);
  const wstring lockFactoryClassName = args[arg++];
  shared_ptr<Path> *const lockDirPath = Paths->get(args[arg++]);
  constexpr int sleepTimeMS = stoi(args[arg++]);
  constexpr int count = stoi(args[arg++]);

  shared_ptr<LockFactory> *const lockFactory =
      getNewLockFactory(lockFactoryClassName);
  // we test the lock factory directly, so we don't need it on the directory
  // itsself (the directory is just for testing)
  shared_ptr<FSDirectory> *const lockDir =
      make_shared<SimpleFSDirectory>(lockDirPath, NoLockFactory::INSTANCE);
  shared_ptr<InetSocketAddress> *const addr =
      make_shared<InetSocketAddress>(verifierHost, verifierPort);
  wcout << L"Connecting to server " << addr << L" and registering as client "
        << myID << L"..." << endl;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.net.Socket socket = new
  // java.net.Socket())
  {
    java::net::Socket socket = java::net::Socket();
    socket.setReuseAddress(true);
    socket.connect(addr, 500);
    shared_ptr<OutputStream> *const out = socket.getOutputStream();
    shared_ptr<InputStream> *const in_ = socket.getInputStream();

    out->write(myID);
    out->flush();
    shared_ptr<LockFactory> verifyLF =
        make_shared<VerifyingLockFactory>(lockFactory, in_, out);
    shared_ptr<Random> *const rnd = make_shared<Random>();

    // wait for starting gun
    if (in_->read() != 43) {
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(L"Protocol violation");
    }

    for (int i = 0; i < count; i++) {
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (final Lock l =
      // verifyLF.obtainLock(lockDir, LOCK_FILE_NAME))
      {
        shared_ptr<Lock> *const l =
            verifyLF->obtainLock(lockDir, LOCK_FILE_NAME);
        try {
          if (rnd->nextInt(10) == 0) {
            if (rnd->nextBoolean()) {
              verifyLF = make_shared<VerifyingLockFactory>(
                  getNewLockFactory(lockFactoryClassName), in_, out);
            }
            // C++ NOTE: The following 'try with resources' block is replaced by
            // its C++ equivalent: ORIGINAL LINE: try (final Lock secondLock =
            // verifyLF.obtainLock(lockDir, LOCK_FILE_NAME))
            {
              shared_ptr<Lock> *const secondLock =
                  verifyLF->obtainLock(lockDir, LOCK_FILE_NAME);
              try {
                // C++ TODO: The following line could not be converted:
                throw java.io.IOException(L"Double obtain");
              } catch (const LockObtainFailedException &loe) {
                // pass
              }
            }
          }
          delay(sleepTimeMS);
        } catch (const LockObtainFailedException &loe) {
          // obtain failed
        }
      }

      if (i % 500 == 0) {
        wcout << (i * 100.0 / count) << L"% done." << endl;
      }

      delay(sleepTimeMS);
    }
  }

  wcout << L"Finished " << count << L" tries." << endl;
}

shared_ptr<FSLockFactory> LockStressTest::getNewLockFactory(
    const wstring &lockFactoryClassName) 
{
  // try to get static INSTANCE field of class
  try {
    return std::static_pointer_cast<FSLockFactory>(
        type_info::forName(lockFactoryClassName)
            .getField(L"INSTANCE")
            ->get(nullptr));
  } catch (const ReflectiveOperationException &e) {
    // fall-through
  }

  // try to create a new instance
  try {
    return type_info::forName(lockFactoryClassName)
        .asSubclass(FSLockFactory::typeid)
        .newInstance();
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (ReflectiveOperationException | ClassCastException e) {
    // fall-through
  }

  // C++ TODO: The following line could not be converted:
  throw java.io.IOException(L"Cannot get lock factory singleton of " +
                            lockFactoryClassName);
}
} // namespace org::apache::lucene::store