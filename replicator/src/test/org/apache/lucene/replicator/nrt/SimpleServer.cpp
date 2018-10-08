using namespace std;

#include "SimpleServer.h"

namespace org::apache::lucene::replicator::nrt
{
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using InputStreamDataInput = org::apache::lucene::store::InputStreamDataInput;
using OutputStreamDataOutput =
    org::apache::lucene::store::OutputStreamDataOutput;
using Constants = org::apache::lucene::util::Constants;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using org::apache::lucene::util::LuceneTestCase::SuppressSysoutChecks;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::SuppressForbidden;
using TestUtil = org::apache::lucene::util::TestUtil;
const shared_ptr<java::util::Set<std::shared_ptr<Thread>>>
    SimpleServer::clientThreads = java::util::Collections::synchronizedSet(
        unordered_set<std::shared_ptr<Thread>>());
const shared_ptr<java::util::concurrent::atomic::AtomicBoolean>
    SimpleServer::stop =
        make_shared<java::util::concurrent::atomic::AtomicBoolean>();

SimpleServer::ClientHandler::ClientHandler(shared_ptr<ServerSocket> ss,
                                           shared_ptr<Node> node,
                                           shared_ptr<Socket> socket)
    : ss(ss), socket(socket), node(node),
      bufferSize(TestUtil::nextInt(LuceneTestCase::random(), 128, 65536))
{
  if (Node::VERBOSE_CONNECTIONS) {
    node->message(L"new connection socket=" + socket);
  }
}

void SimpleServer::ClientHandler::run()
{
  bool success = false;
  try {
    // node.message("using stream buffer size=" + bufferSize);
    shared_ptr<InputStream> is =
        make_shared<BufferedInputStream>(socket->getInputStream(), bufferSize);
    shared_ptr<DataInput> in_ = make_shared<InputStreamDataInput>(is);
    shared_ptr<BufferedOutputStream> bos = make_shared<BufferedOutputStream>(
        socket->getOutputStream(), bufferSize);
    shared_ptr<DataOutput> out = make_shared<OutputStreamDataOutput>(bos);

    if (std::dynamic_pointer_cast<SimplePrimaryNode>(node) != nullptr) {
      (std::static_pointer_cast<SimplePrimaryNode>(node))
          ->handleOneConnection(LuceneTestCase::random(), ss, stop, is, socket,
                                in_, out, bos);
    } else {
      (std::static_pointer_cast<SimpleReplicaNode>(node))
          ->handleOneConnection(ss, stop, is, socket, in_, out, bos);
    }

    bos->flush();
    if (Node::VERBOSE_CONNECTIONS) {
      node->message(L"bos.flush done");
    }

    success = true;
  } catch (const runtime_error &t) {
    if (std::dynamic_pointer_cast<SocketException>(t) != nullptr == false &&
        std::dynamic_pointer_cast<NodeCommunicationException>(t) != nullptr ==
            false) {
      node->message(L"unexpected exception handling client connection; now "
                    L"failing test:");
      t.printStackTrace(System::out);
      IOUtils::closeWhileHandlingException({ss});
      // Test should fail with this:
      throw runtime_error(t);
    } else {
      node->message(L"exception handling client connection; ignoring:");
      t.printStackTrace(System::out);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      try {
        IOUtils::close({socket});
      } catch (const IOException &ioe) {
        throw runtime_error(ioe);
      }
    } else {
      IOUtils::closeWhileHandlingException({socket});
    }
  }
  if (Node::VERBOSE_CONNECTIONS) {
    node->message(L"socket.close done");
  }
}

void SimpleServer::crashJRE()
{
  const wstring vendor = Constants::JAVA_VENDOR;
  constexpr bool supportsUnsafeNpeDereference =
      StringHelper::startsWith(vendor, L"Oracle") ||
      StringHelper::startsWith(vendor, L"Sun") ||
      StringHelper::startsWith(vendor, L"Apple");

  try {
    if (supportsUnsafeNpeDereference) {
      try {
        type_info clazz = type_info::forName(L"sun.misc.Unsafe");
        shared_ptr<java::lang::reflect::Field> field =
            clazz.getDeclaredField(L"theUnsafe");
        field->setAccessible(true);
        any o = field->get(nullptr);
        shared_ptr<Method> m = clazz.getMethod(
            L"putAddress", int64_t ::typeid, int64_t ::typeid);
        m->invoke(o, 0LL, 0LL);
      } catch (const runtime_error &e) {
        wcout << L"Couldn't kill the JVM via Unsafe." << endl;
        e.printStackTrace(System::out);
      }
    }

    // Fallback attempt to Runtime.halt();
    Runtime::getRuntime().halt(-1);
  } catch (const runtime_error &e) {
    wcout << L"Couldn't kill the JVM." << endl;
    e.printStackTrace(System::out);
  }

  // We couldn't get the JVM to crash for some reason.
  throw runtime_error(L"JVM refuses to die!");
}

void SimpleServer::writeFilesMetaData(
    shared_ptr<DataOutput> out,
    unordered_map<wstring, std::shared_ptr<FileMetaData>>
        &files) 
{
  out->writeVInt(files.size());
  for (auto ent : files) {
    out->writeString(ent.first);

    shared_ptr<FileMetaData> fmd = ent.second;
    out->writeVLong(fmd->length);
    out->writeVLong(fmd->checksum);
    out->writeVInt(fmd->header.size());
    out->writeBytes(fmd->header, 0, fmd->header.size());
    out->writeVInt(fmd->footer.size());
    out->writeBytes(fmd->footer, 0, fmd->footer.size());
  }
}

unordered_map<wstring, std::shared_ptr<FileMetaData>>
SimpleServer::readFilesMetaData(shared_ptr<DataInput> in_) 
{
  int fileCount = in_->readVInt();
  // System.out.println("readFilesMetaData: fileCount=" + fileCount);
  unordered_map<wstring, std::shared_ptr<FileMetaData>> files =
      unordered_map<wstring, std::shared_ptr<FileMetaData>>();
  for (int i = 0; i < fileCount; i++) {
    wstring fileName = in_->readString();
    // System.out.println("readFilesMetaData: fileName=" + fileName);
    int64_t length = in_->readVLong();
    int64_t checksum = in_->readVLong();
    std::deque<char> header(in_->readVInt());
    in_->readBytes(header, 0, header.size());
    std::deque<char> footer(in_->readVInt());
    in_->readBytes(footer, 0, footer.size());
    files.emplace(fileName,
                  make_shared<FileMetaData>(header, footer, length, checksum));
  }
  return files;
}

shared_ptr<CopyState>
SimpleServer::readCopyState(shared_ptr<DataInput> in_) 
{

  // Decode a new CopyState
  std::deque<char> infosBytes(in_->readVInt());
  in_->readBytes(infosBytes, 0, infosBytes.size());

  int64_t gen = in_->readVLong();
  int64_t version = in_->readVLong();
  unordered_map<wstring, std::shared_ptr<FileMetaData>> files =
      readFilesMetaData(in_);

  int count = in_->readVInt();
  shared_ptr<Set<wstring>> completedMergeFiles = unordered_set<wstring>();
  for (int i = 0; i < count; i++) {
    completedMergeFiles->add(in_->readString());
  }
  int64_t primaryGen = in_->readVLong();

  return make_shared<CopyState>(files, version, gen, infosBytes,
                                completedMergeFiles, primaryGen, nullptr);
}

void SimpleServer::test() 
{

  int id =
      static_cast<Integer>(System::getProperty(L"tests.nrtreplication.nodeid"));
  Thread::currentThread().setName(L"main child " + to_wstring(id));
  shared_ptr<Path> indexPath =
      Paths->get(System::getProperty(L"tests.nrtreplication.indexpath"));
  bool isPrimary =
      System::getProperty(L"tests.nrtreplication.isPrimary") != nullptr;
  int primaryTCPPort;
  int64_t forcePrimaryVersion;
  if (isPrimary == false) {
    forcePrimaryVersion = -1;
    primaryTCPPort = static_cast<Integer>(
        System::getProperty(L"tests.nrtreplication.primaryTCPPort"));
  } else {
    primaryTCPPort = -1;
    forcePrimaryVersion = static_cast<int64_t>(
        System::getProperty(L"tests.nrtreplication.forcePrimaryVersion"));
  }
  int64_t primaryGen = static_cast<int64_t>(
      System::getProperty(L"tests.nrtreplication.primaryGen"));
  Node::globalStartNS = static_cast<int64_t>(
      System::getProperty(L"tests.nrtreplication.startNS"));

  bool doRandomCrash =
      L"true" == System::getProperty(L"tests.nrtreplication.doRandomCrash");
  bool doRandomClose =
      L"true" == System::getProperty(L"tests.nrtreplication.doRandomClose");
  bool doFlipBitsDuringCopy =
      L"true" ==
      System::getProperty(L"tests.nrtreplication.doFlipBitsDuringCopy");
  bool doCheckIndexOnClose =
      L"true" == System::getProperty(L"tests.nrtreplication.checkonclose");

  // Create server socket that we listen for incoming requests on:
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (final java.net.ServerSocket ss = new
  // java.net.ServerSocket(0, 0, java.net.InetAddress.getLoopbackAddress()))
  {
    shared_ptr<java::net::ServerSocket> *const ss =
        make_shared<java.net::ServerSocket>(
            0, 0, java.net::InetAddress::getLoopbackAddress());

    int tcpPort = (std::static_pointer_cast<InetSocketAddress>(
                       ss->getLocalSocketAddress()))
                      ->getPort();
    wcout << L"\nPORT: " << tcpPort << endl;
    shared_ptr<Node> *const node;
    if (isPrimary) {
      node = make_shared<SimplePrimaryNode>(
          random(), indexPath, id, tcpPort, primaryGen, forcePrimaryVersion,
          nullptr, doFlipBitsDuringCopy, doCheckIndexOnClose);
      wcout << L"\nCOMMIT VERSION: "
            << (std::static_pointer_cast<PrimaryNode>(node))
                   ->getLastCommitVersion()
            << endl;
    } else {
      try {
        node = make_shared<SimpleReplicaNode>(random(), id, tcpPort, indexPath,
                                              primaryGen, primaryTCPPort,
                                              nullptr, doCheckIndexOnClose);
      } catch (const runtime_error &re) {
        if (re.what()->startsWith(L"replica cannot start")) {
          // this is "OK": it means MDW's refusal to delete a segments_N commit
          // point means we cannot start:
          assumeTrue(re.what(), false);
        }
        throw re;
      }
    }
    wcout << L"\nINFOS VERSION: " << node->getCurrentSearchingVersion() << endl;

    if (doRandomClose || doRandomCrash) {
      constexpr int waitForMS;
      if (isPrimary) {
        waitForMS = TestUtil::nextInt(random(), 20000, 60000);
      } else {
        waitForMS = TestUtil::nextInt(random(), 5000, 60000);
      }

      bool doClose;
      if (doRandomCrash == false) {
        doClose = true;
      } else if (doRandomClose) {
        doClose = random()->nextBoolean();
      } else {
        doClose = false;
      }

      if (doClose) {
        node->message(L"top: will close after " +
                      to_wstring(waitForMS / 1000.0) + L" seconds");
      } else {
        node->message(L"top: will crash after " +
                      to_wstring(waitForMS / 1000.0) + L" seconds");
      }

      shared_ptr<Thread> t = make_shared<ThreadAnonymousInnerClass>(
          shared_from_this(), ss, node, waitForMS, doClose);

      if (isPrimary) {
        t->setName(L"crasher P" + to_wstring(id));
      } else {
        t->setName(L"crasher R" + to_wstring(id));
      }

      // So that if node exits naturally, this thread won't prevent process
      // exit:
      t->setDaemon(true);
      t->start();
    }
    wcout << L"\nNODE STARTED" << endl;

    // List<Thread> clientThreads = new ArrayList<>();

    // Naive thread-per-connection server:
    while (true) {
      shared_ptr<Socket> socket;
      try {
        socket = ss->accept();
      } catch (const SocketException &se) {
        // when ClientHandler closes our ss we will hit this
        node->message(L"top: server socket exc; now exit");
        break;
      }
      shared_ptr<Thread> thread = make_shared<ClientHandler>(ss, node, socket);
      thread->setDaemon(true);
      thread->start();

      clientThreads->add(thread);

      // Prune finished client threads:
      Set<std::shared_ptr<Thread>>::const_iterator it = clientThreads->begin();
      while (it != clientThreads->end()) {
        shared_ptr<Thread> t = *it;
        if (t->isAlive() == false) {
          (*it)->remove();
        }
        it++;
      }
      // node.message(clientThreads.size() + " client threads are still alive");
    }

    stop->set(true);

    // Make sure all client threads are done, else we get annoying (yet
    // ultimately "harmless") messages about threads still running / lingering
    // for them to finish from the child processes:
    for (auto clientThread : clientThreads) {
      node->message(L"top: join clientThread=" + clientThread);
      clientThread->join();
      node->message(L"top: done join clientThread=" + clientThread);
    }
    node->message(L"done join all client threads; now close node");
    delete node;
  }
}

SimpleServer::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<SimpleServer> outerInstance, shared_ptr<ServerSocket> ss,
    shared_ptr<org::apache::lucene::replicator::nrt::Node> node, int waitForMS,
    bool doClose)
{
  this->outerInstance = outerInstance;
  this->ss = ss;
  this->node = node;
  this->waitForMS = waitForMS;
  this->doClose = doClose;
}

void SimpleServer::ThreadAnonymousInnerClass::run()
{
  int64_t endTime = System::nanoTime() + waitForMS * 1000000LL;
  while (System::nanoTime() < endTime) {
    try {
      delay(10);
    } catch (const InterruptedException &e) {
    }
    if (stop->get()) {
      break;
    }
  }

  if (stop->get() == false) {
    if (doClose) {
      try {
        node->message(L"top: now force close server socket after " +
                      to_wstring(waitForMS / 1000.0) + L" seconds");
        node->state = L"top-closing";
        ss->close();
      } catch (const IOException &ioe) {
        throw runtime_error(ioe);
      }
    } else {
      node->message(L"top: now crash JVM after " +
                    to_wstring(waitForMS / 1000.0) + L" seconds");
      crashJRE();
    }
  }
}
} // namespace org::apache::lucene::replicator::nrt