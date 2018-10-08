using namespace std;

#include "GetTermInfo.h"

namespace org::apache::lucene::misc
{
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using org::apache::lucene::util::SuppressForbidden;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;

void GetTermInfo::main(std::deque<wstring> &args) 
{

  shared_ptr<FSDirectory> dir = nullptr;
  wstring inputStr = L"";
  wstring field = L"";

  if (args.size() == 3) {
    dir = FSDirectory::open(Paths->get(args[0]));
    field = args[1];
    inputStr = args[2];
  } else {
    usage();
    exit(1);
  }

  getTermInfo(dir, make_shared<Term>(field, inputStr));
}

void GetTermInfo::getTermInfo(shared_ptr<Directory> dir,
                              shared_ptr<Term> term) 
{
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  printf(Locale::ROOT, L"%s:%s \t totalTF = %,d \t doc freq = %,d \n",
         term->field(), term->text(), reader->totalTermFreq(term),
         reader->docFreq(term));
}

void GetTermInfo::usage()
{
  wcout << L"\n\nusage:\n\t" << L"java " << GetTermInfo::typeid->getName()
        << L" <index dir> field term \n\n"
        << endl;
}
} // namespace org::apache::lucene::misc