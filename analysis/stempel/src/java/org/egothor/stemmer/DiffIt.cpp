using namespace std;

#include "DiffIt.h"
#include "Diff.h"

namespace org::egothor::stemmer
{
using org::apache::lucene::util::SuppressForbidden;

DiffIt::DiffIt() {}

int DiffIt::get(int i, const wstring &s)
{
  try {
    return stoi(s.substr(i, 1));
  } catch (const runtime_error &x) {
    return 1;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command line
// tool") public static void main(java.lang.std::wstring[] args) throws Exception
void DiffIt::main(std::deque<wstring> &args) 
{

  int ins = get(0, args[0]);
  int del = get(1, args[0]);
  int rep = get(2, args[0]);
  int nop = get(3, args[0]);

  for (int i = 1; i < args.size(); i++) {
    // System.out.println("[" + args[i] + "]");
    shared_ptr<Diff> diff = make_shared<Diff>(ins, del, rep, nop);
    wstring charset = System::getProperty(L"egothor.stemmer.charset", L"UTF-8");
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
          wcout << stem << L" -a" << endl;
          while (st->hasMoreTokens()) {
            wstring token = st->nextToken();
            if (token == stem == false) {
              wcout << stem << L" " << diff->exec(token, stem) << endl;
            }
          }
        } catch (const NoSuchElementException &x) {
          // no base token (stem) on a line
        }
      }
    }
  }
}
} // namespace org::egothor::stemmer