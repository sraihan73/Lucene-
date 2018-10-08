using namespace std;

#include "IndexMergeTool.h"

namespace org::apache::lucene::misc
{
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using HardlinkCopyDirectoryWrapper =
    org::apache::lucene::store::HardlinkCopyDirectoryWrapper;
using org::apache::lucene::util::SuppressForbidden;

void IndexMergeTool::main(std::deque<wstring> &args) 
{
  if (args.size() < 3) {
    System::err::println(
        L"Usage: IndexMergeTool <mergedIndex> <index1> <index2> [index3] ...");
    exit(1);
  }

  // Try to use hardlinks to source segments, if possible.
  shared_ptr<Directory> mergedIndex = make_shared<HardlinkCopyDirectoryWrapper>(
      FSDirectory::open(Paths->get(args[0])));

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      mergedIndex, (make_shared<IndexWriterConfig>(nullptr))
                       ->setOpenMode(IndexWriterConfig::OpenMode::CREATE));

  std::deque<std::shared_ptr<Directory>> indexes(args.size() - 1);
  for (int i = 1; i < args.size(); i++) {
    indexes[i - 1] = FSDirectory::open(Paths->get(args[i]));
  }

  wcout << L"Merging..." << endl;
  writer->addIndexes(indexes);

  wcout << L"Full merge..." << endl;
  writer->forceMerge(1);
  delete writer;
  wcout << L"Done." << endl;
}
} // namespace org::apache::lucene::misc