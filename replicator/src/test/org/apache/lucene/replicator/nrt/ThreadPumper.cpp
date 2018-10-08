using namespace std;

#include "ThreadPumper.h"

namespace org::apache::lucene::replicator::nrt
{

shared_ptr<Thread> ThreadPumper::start(Runnable const onExit,
                                       shared_ptr<BufferedReader> from,
                                       shared_ptr<PrintStream> to,
                                       shared_ptr<Writer> toFile,
                                       shared_ptr<AtomicBoolean> nodeClosing)
{
  shared_ptr<Thread> t =
      make_shared<ThreadAnonymousInnerClass>(onExit, from, toFile, nodeClosing);
  t->start();
  return t;
}

ThreadPumper::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    Runnable onExit, shared_ptr<BufferedReader> from, shared_ptr<Writer> toFile,
    shared_ptr<AtomicBoolean> nodeClosing)
{
  this->onExit = onExit;
  this->from = from;
  this->toFile = toFile;
  this->nodeClosing = nodeClosing;
}

void ThreadPumper::ThreadAnonymousInnerClass::run()
{
  try {
    int64_t startTimeNS = System::nanoTime();
    shared_ptr<Pattern> logTimeStart = Pattern::compile(L"^[0-9\\.]+s .*");
    wstring line;
    while ((line = from->readLine()) != L"") {
      if (toFile != nullptr) {
        toFile->write(line);
        toFile->write(L"\n");
        toFile->flush();
      } else if (logTimeStart->matcher(line).matches()) {
        // Already a well-formed log output:
        wcout << line << endl;
      } else {
        TestStressNRTReplication::message(line, startTimeNS);
      }
      if (line.find(L"now force close server socket after") != wstring::npos) {
        nodeClosing->set(true);
      }
    }
    // Sub-process finished
  } catch (const IOException &e) {
    System::err::println(L"ignore IOExc reading from forked process pipe: " +
                         e);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    onExit();
  }
}
} // namespace org::apache::lucene::replicator::nrt