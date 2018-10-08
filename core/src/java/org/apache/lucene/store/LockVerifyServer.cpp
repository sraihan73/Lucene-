using namespace std;

#include "LockVerifyServer.h"

namespace org::apache::lucene::store
{
using IOUtils = org::apache::lucene::util::IOUtils;
using org::apache::lucene::util::SuppressForbidden;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command line
// tool") public static void main(std::wstring[] args) throws Exception
void LockVerifyServer::main(std::deque<wstring> &args) 
{

  if (args.size() != 2) {
    wcout << L"Usage: java org.apache.lucene.store.LockVerifyServer bindToIp "
             L"clients\n"
          << endl;
    exit(1);
  }

  int arg = 0;
  const wstring hostname = args[arg++];
  constexpr int maxClients = stoi(args[arg++]);

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (final java.net.ServerSocket s = new
  // java.net.ServerSocket())
  {
    shared_ptr<java::net::ServerSocket> *const s =
        make_shared<java.net::ServerSocket>();
    s->setReuseAddress(true);
    s->setSoTimeout(
        30000); // initially 30 secs to give clients enough time to startup
    s->bind(make_shared<java.net::InetSocketAddress>(hostname, 0));
    shared_ptr<InetSocketAddress> *const localAddr =
        std::static_pointer_cast<InetSocketAddress>(s->getLocalSocketAddress());
    wcout << L"Listening on " << localAddr << L"..." << endl;

    // we set the port as a sysprop, so the ANT task can read it. For that to
    // work, this server must run in-process:
    // C++ TODO: There is no native C++ equivalent to 'toString':
    System::setProperty(L"lockverifyserver.port",
                        Integer::toString(localAddr->getPort()));

    constexpr any localLock;
    const std::deque<int> lockedID = std::deque<int>(1);
    lockedID[0] = -1;
    shared_ptr<CountDownLatch> *const startingGun =
        make_shared<java.util::concurrent::CountDownLatch>(1);
    std::deque<std::shared_ptr<Thread>> threads(maxClients);

    for (int count = 0; count < maxClients; count++) {
      shared_ptr<Socket> *const cs = s->accept();
      threads[count] = make_shared<ThreadAnonymousInnerClass>(
          localLock, lockedID, startingGun, cs);
      threads[count]->start();
    }

    // start
    wcout << L"All clients started, fire gun..." << endl;
    startingGun->countDown();

    // wait for all threads to finish
    for (auto t : threads) {
      t->join();
    }

    // cleanup sysprop
    System::clearProperty(L"lockverifyserver.port");

    wcout << L"Server terminated." << endl;
  }
}

LockVerifyServer::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    any localLock, deque<int> &lockedID,
    shared_ptr<CountDownLatch> startingGun, shared_ptr<Socket> cs)
{
  this->localLock = localLock;
  this->lockedID = lockedID;
  this->startingGun = startingGun;
  this->cs = cs;
}

void LockVerifyServer::ThreadAnonymousInnerClass::run()
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream in =
  // cs.getInputStream(); java.io.OutputStream os = cs.getOutputStream())
  {
    java::io::InputStream in_ = cs->getInputStream();
    java::io::OutputStream os = cs->getOutputStream();
    try {
      constexpr int id = in_.read();
      if (id < 0) {
        // C++ TODO: The following line could not be converted:
        throw java.io.IOException(
            L"Client closed connection before communication started.");
      }

      startingGun->await();
      os.write(43);
      os.flush();

      while (true) {
        constexpr int command = in_.read();
        if (command < 0) {
          return; // closed
        }

        {
          lock_guard<mutex> lock(localLock);
          constexpr int currentLock = lockedID[0];
          if (currentLock == -2) {
            return; // another thread got error, so we exit, too!
          }
          switch (command) {
          case 1:
            // Locked
            if (currentLock != -1) {
              lockedID[0] = -2;
              throw make_shared<IllegalStateException>(
                  L"id " + to_wstring(id) + L" got lock, but " +
                  to_wstring(currentLock) + L" already holds the lock");
            }
            lockedID[0] = id;
            break;
          case 0:
            // Unlocked
            if (currentLock != id) {
              lockedID[0] = -2;
              throw make_shared<IllegalStateException>(
                  L"id " + to_wstring(id) + L" released the lock, but " +
                  to_wstring(currentLock) + L" is the one holding the lock");
            }
            lockedID[0] = -1;
            break;
          default:
            throw runtime_error(L"Unrecognized command: " +
                                to_wstring(command));
          }
          os.write(command);
          os.flush();
        }
      }
    }
    // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
    catch (RuntimeException | Error e) {
      throw e;
    } catch (const runtime_error &ioe) {
      throw runtime_error(ioe);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      IOUtils::closeWhileHandlingException({cs});
    }
  }
}
} // namespace org::apache::lucene::store