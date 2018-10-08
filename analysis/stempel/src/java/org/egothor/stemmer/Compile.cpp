using namespace std;

#include "Compile.h"
#include "Diff.h"
#include "Gener.h"
#include "Lift.h"
#include "MultiTrie2.h"
#include "Optimizer.h"
#include "Optimizer2.h"
#include "Trie.h"

namespace org::egothor::stemmer
{
using org::apache::lucene::util::SuppressForbidden;
bool Compile::backward = false;
bool Compile::multi = false;
shared_ptr<Trie> Compile::trie;

Compile::Compile() {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command line
// tool") public static void main(java.lang.std::wstring[] args) throws Exception
void Compile::main(std::deque<wstring> &args) 
{
  if (args.size() < 1) {
    return;
  }

  args[0].toUpperCase(Locale::ROOT);

  backward = args[0][0] == L'-';
  int qq = (backward) ? 1 : 0;
  bool storeorig = false;

  if (args[0][qq] == L'0') {
    storeorig = true;
    qq++;
  }

  multi = args[0][qq] == L'M';
  if (multi) {
    qq++;
  }

  wstring charset = System::getProperty(L"egothor.stemmer.charset", L"UTF-8");

  std::deque<wchar_t> optimizer(args[0].length() - qq);
  for (int i = 0; i < optimizer.size(); i++) {
    optimizer[i] = args[0][qq + i];
  }

  for (int i = 1; i < args.size(); i++) {
    // System.out.println("[" + args[i] + "]");
    shared_ptr<Diff> diff = make_shared<Diff>();

    allocTrie();

    wcout << args[i] << endl;
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.LineNumberReader in = new
    // java.io.LineNumberReader(java.nio.file.Files.newBufferedReader(java.nio.file.Paths.get(args[i]),
    // java.nio.charset.Charset.forName(charset))))
    {
      java::io::LineNumberReader in_ =
          java::io::LineNumberReader(java::nio::file::Files::newBufferedReader(
              java::nio::file::Paths->get(args[i]),
              java::nio::charset::Charset::forName(charset)));
      for (wstring line = in_.readLine(); line != L""; line = in_.readLine()) {
        try {
          line = line.toLowerCase(Locale::ROOT);
          shared_ptr<StringTokenizer> st = make_shared<StringTokenizer>(line);
          wstring stem = st->nextToken();
          if (storeorig) {
            trie->add(stem, L"-a");
          }
          while (st->hasMoreTokens()) {
            wstring token = st->nextToken();
            if (token == stem == false) {
              trie->add(token, diff->exec(token, stem));
            }
          }
        } catch (const NoSuchElementException &x) {
          // no base token (stem) on a line
        }
      }
    }

    shared_ptr<Optimizer> o = make_shared<Optimizer>();
    shared_ptr<Optimizer2> o2 = make_shared<Optimizer2>();
    shared_ptr<Lift> l = make_shared<Lift>(true);
    shared_ptr<Lift> e = make_shared<Lift>(false);
    shared_ptr<Gener> g = make_shared<Gener>();

    for (int j = 0; j < optimizer.size(); j++) {
      wstring prefix;
      switch (optimizer[j]) {
      case L'G':
        trie = trie->reduce(g);
        prefix = L"G: ";
        break;
      case L'L':
        trie = trie->reduce(l);
        prefix = L"L: ";
        break;
      case L'E':
        trie = trie->reduce(e);
        prefix = L"E: ";
        break;
      case L'2':
        trie = trie->reduce(o2);
        prefix = L"2: ";
        break;
      case L'1':
        trie = trie->reduce(o);
        prefix = L"1: ";
        break;
      default:
        continue;
      }
      trie->printInfo(System::out, prefix + L" ");
    }

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.DataOutputStream os = new
    // java.io.DataOutputStream(new
    // java.io.BufferedOutputStream(java.nio.file.Files.newOutputStream(java.nio.file.Paths.get(args[i]
    // + ".out")))))
    {
      java::io::DataOutputStream os = java::io::DataOutputStream(
          make_shared<java::io::BufferedOutputStream>(
              java::nio::file::Files::newOutputStream(
                  java::nio::file::Paths->get(args[i] + L".out"))));
      os.writeUTF(args[0]);
      trie->store(os);
    }
  }
}

void Compile::allocTrie()
{
  if (multi) {
    trie = make_shared<MultiTrie2>(!backward);
  } else {
    trie = make_shared<Trie>(!backward);
  }
}
} // namespace org::egothor::stemmer