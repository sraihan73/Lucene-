using namespace std;

#include "IndexUpgrader.h"

namespace org::apache::lucene::index
{
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using CommandLineUtil = org::apache::lucene::util::CommandLineUtil;
using InfoStream = org::apache::lucene::util::InfoStream;
using PrintStreamInfoStream = org::apache::lucene::util::PrintStreamInfoStream;
using org::apache::lucene::util::SuppressForbidden;
using Version = org::apache::lucene::util::Version;
const wstring IndexUpgrader::LOG_PREFIX = L"IndexUpgrader";

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command line
// tool") private static void printUsage()
void IndexUpgrader::printUsage()
{
  System::err::println(L"Upgrades an index so all segments created with a "
                       L"previous Lucene version are rewritten.");
  System::err::println(L"Usage:");
  System::err::println(
      L"  java " + IndexUpgrader::typeid->getName() +
      L" [-delete-prior-commits] [-verbose] [-dir-impl X] indexDir");
  System::err::println(
      L"This tool keeps only the last commit in an index; for this");
  System::err::println(
      L"reason, if the incoming index has more than one commit, the tool");
  System::err::println(
      L"refuses to run by default. Specify -delete-prior-commits to override");
  System::err::println(
      L"this, allowing the tool to delete all but the last commit.");
  System::err::println(L"Specify a " + FSDirectory::typeid->getSimpleName() +
                       L" implementation through the -dir-impl option to force "
                       L"its use. If no package is specified the " +
                       FSDirectory::typeid->getPackage().getName() +
                       L" package will be used.");
  System::err::println(L"WARNING: This tool may reorder document IDs!");
  exit(1);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("deprecation") public static void
// main(std::wstring[] args) throws java.io.IOException
void IndexUpgrader::main(std::deque<wstring> &args) 
{
  parseArgs(args)->upgrade();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command line
// tool") static IndexUpgrader parseArgs(std::wstring[] args) throws
// java.io.IOException
shared_ptr<IndexUpgrader>
IndexUpgrader::parseArgs(std::deque<wstring> &args) 
{
  wstring path = L"";
  bool deletePriorCommits = false;
  shared_ptr<InfoStream> out = nullptr;
  wstring dirImpl = L"";
  int i = 0;
  while (i < args.size()) {
    wstring arg = args[i];
    if (L"-delete-prior-commits" == arg) {
      deletePriorCommits = true;
    } else if (L"-verbose" == arg) {
      out = make_shared<PrintStreamInfoStream>(System::out);
    } else if (L"-dir-impl" == arg) {
      if (i == args.size() - 1) {
        wcout << L"ERROR: missing value for -dir-impl option" << endl;
        exit(1);
      }
      i++;
      dirImpl = args[i];
    } else if (path == L"") {
      path = arg;
    } else {
      printUsage();
    }
    i++;
  }
  if (path == L"") {
    printUsage();
  }

  shared_ptr<Path> p = Paths->get(path);
  shared_ptr<Directory> dir = nullptr;
  if (dirImpl == L"") {
    dir = FSDirectory::open(p);
  } else {
    dir = CommandLineUtil::newFSDirectory(dirImpl, p);
  }
  return make_shared<IndexUpgrader>(dir, out, deletePriorCommits);
}

IndexUpgrader::IndexUpgrader(shared_ptr<Directory> dir)
    : IndexUpgrader(dir, new IndexWriterConfig(nullptr), false)
{
}

IndexUpgrader::IndexUpgrader(shared_ptr<Directory> dir,
                             shared_ptr<InfoStream> infoStream,
                             bool deletePriorCommits)
    : IndexUpgrader(dir, new IndexWriterConfig(nullptr), deletePriorCommits)
{
  if (nullptr != infoStream) {
    this->iwc->setInfoStream(infoStream);
  }
}

IndexUpgrader::IndexUpgrader(shared_ptr<Directory> dir,
                             shared_ptr<IndexWriterConfig> iwc,
                             bool deletePriorCommits)
    : dir(dir), iwc(iwc), deletePriorCommits(deletePriorCommits)
{
}

void IndexUpgrader::upgrade() 
{
  if (!DirectoryReader::indexExists(dir)) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw make_shared<IndexNotFoundException>(dir->toString());
  }

  if (!deletePriorCommits) {
    shared_ptr<deque<std::shared_ptr<IndexCommit>>> *const commits =
        DirectoryReader::listCommits(dir);
    if (commits->size() > 1) {
      throw invalid_argument(
          L"This tool was invoked to not delete prior commit points, but the "
          L"following commits were found: " +
          commits);
    }
  }

  iwc->setMergePolicy(
      make_shared<UpgradeIndexMergePolicy>(iwc->getMergePolicy()));
  iwc->setIndexDeletionPolicy(make_shared<KeepOnlyLastCommitDeletionPolicy>());

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (final IndexWriter w = new IndexWriter(dir,
  // iwc))
  {
    shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, iwc);
    shared_ptr<InfoStream> infoStream = iwc->getInfoStream();
    if (infoStream->isEnabled(LOG_PREFIX)) {
      infoStream->message(LOG_PREFIX, L"Upgrading all pre-" + Version::LATEST +
                                          L" segments of index directory '" +
                                          dir + L"' to version " +
                                          Version::LATEST + L"...");
    }
    w->forceMerge(1);
    if (infoStream->isEnabled(LOG_PREFIX)) {
      infoStream->message(LOG_PREFIX, L"All segments upgraded to version " +
                                          Version::LATEST);
      infoStream->message(LOG_PREFIX,
                          L"Enforcing commit to rewrite all index metadata...");
    }
    w->setLiveCommitData(
        w->getLiveCommitData()); // fake change to enforce a commit (e.g. if
                                 // index has no segments)
    assert(w->hasUncommittedChanges());
    w->commit();
    if (infoStream->isEnabled(LOG_PREFIX)) {
      infoStream->message(LOG_PREFIX, L"Committed upgraded metadata to index.");
    }
  }
}
} // namespace org::apache::lucene::index