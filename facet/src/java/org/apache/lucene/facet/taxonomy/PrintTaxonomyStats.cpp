using namespace std;

#include "PrintTaxonomyStats.h"

namespace org::apache::lucene::facet::taxonomy
{
using ChildrenIterator =
    org::apache::lucene::facet::taxonomy::TaxonomyReader::ChildrenIterator;
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using org::apache::lucene::util::SuppressForbidden;

PrintTaxonomyStats::PrintTaxonomyStats() {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command line
// tool") public static void main(std::wstring[] args) throws java.io.IOException
void PrintTaxonomyStats::main(std::deque<wstring> &args) 
{
  bool printTree = false;
  wstring path = L"";
  for (int i = 0; i < args.size(); i++) {
    if (args[i] == L"-printTree") {
      printTree = true;
    } else {
      path = args[i];
    }
  }
  if (args.size() != (printTree ? 2 : 1)) {
    wcout << L"\nUsage: java -classpath ... "
             L"org.apache.lucene.facet.util.PrintTaxonomyStats [-printTree] "
             L"/path/to/taxononmy/index\n"
          << endl;
    exit(1);
  }
  shared_ptr<Directory> dir = FSDirectory::open(Paths->get(path));
  shared_ptr<TaxonomyReader> r = make_shared<DirectoryTaxonomyReader>(dir);
  printStats(r, System::out, printTree);
  delete r;
  delete dir;
}

void PrintTaxonomyStats::printStats(shared_ptr<TaxonomyReader> r,
                                    shared_ptr<PrintStream> out,
                                    bool printTree) 
{
  out->println(to_wstring(r->getSize()) + L" total categories.");

  shared_ptr<ChildrenIterator> it =
      r->getChildren(TaxonomyReader::ROOT_ORDINAL);
  int child;
  while ((child = it->next()) != TaxonomyReader::INVALID_ORDINAL) {
    shared_ptr<ChildrenIterator> chilrenIt = r->getChildren(child);
    int numImmediateChildren = 0;
    while (chilrenIt->next() != TaxonomyReader::INVALID_ORDINAL) {
      numImmediateChildren++;
    }
    shared_ptr<FacetLabel> cp = r->getPath(child);
    out->println(L"/" + cp->components[0] + L": " +
                 to_wstring(numImmediateChildren) + L" immediate children; " +
                 to_wstring(1 + countAllChildren(r, child)) +
                 L" total categories");
    if (printTree) {
      printAllChildren(out, r, child, L"  ", 1);
    }
  }
}

int PrintTaxonomyStats::countAllChildren(shared_ptr<TaxonomyReader> r,
                                         int ord) 
{
  int count = 0;
  shared_ptr<ChildrenIterator> it = r->getChildren(ord);
  int child;
  while ((child = it->next()) != TaxonomyReader::INVALID_ORDINAL) {
    count += 1 + countAllChildren(r, child);
  }
  return count;
}

void PrintTaxonomyStats::printAllChildren(shared_ptr<PrintStream> out,
                                          shared_ptr<TaxonomyReader> r, int ord,
                                          const wstring &indent,
                                          int depth) 
{
  shared_ptr<ChildrenIterator> it = r->getChildren(ord);
  int child;
  while ((child = it->next()) != TaxonomyReader::INVALID_ORDINAL) {
    out->println(indent + L"/" + r->getPath(child)->components[depth]);
    printAllChildren(out, r, child, indent + L"  ", depth + 1);
  }
}
} // namespace org::apache::lucene::facet::taxonomy