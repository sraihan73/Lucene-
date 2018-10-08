using namespace std;

#include "TestCompile.h"
#include "../../../../java/org/egothor/stemmer/Compile.h"
#include "../../../../java/org/egothor/stemmer/Diff.h"
#include "../../../../java/org/egothor/stemmer/MultiTrie.h"
#include "../../../../java/org/egothor/stemmer/Trie.h"

namespace org::egothor::stemmer
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestCompile::testCompile() 
{
  shared_ptr<Path> dir = createTempDir(L"testCompile");
  shared_ptr<Path> output = dir->resolve(L"testRules.txt");
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream input =
  // getClass().getResourceAsStream("testRules.txt"))
  {
    java::io::InputStream input =
        getClass().getResourceAsStream(L"testRules.txt");
    Files::copy(input, output);
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring path = output->toAbsolutePath()->toString();
  Compile::main(std::deque<wstring>{L"test", path});
  shared_ptr<Path> compiled = dir->resolve(L"testRules.txt.out");
  shared_ptr<Trie> trie = loadTrie(compiled);
  assertTrie(trie, output, true, true);
  assertTrie(trie, output, false, true);
}

void TestCompile::testCompileBackwards() 
{
  shared_ptr<Path> dir = createTempDir(L"testCompile");
  shared_ptr<Path> output = dir->resolve(L"testRules.txt");
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream input =
  // getClass().getResourceAsStream("testRules.txt"))
  {
    java::io::InputStream input =
        getClass().getResourceAsStream(L"testRules.txt");
    Files::copy(input, output);
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring path = output->toAbsolutePath()->toString();
  Compile::main(std::deque<wstring>{L"-test", path});
  shared_ptr<Path> compiled = dir->resolve(L"testRules.txt.out");
  shared_ptr<Trie> trie = loadTrie(compiled);
  assertTrie(trie, output, true, true);
  assertTrie(trie, output, false, true);
}

void TestCompile::testCompileMulti() 
{
  shared_ptr<Path> dir = createTempDir(L"testCompile");
  shared_ptr<Path> output = dir->resolve(L"testRules.txt");
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream input =
  // getClass().getResourceAsStream("testRules.txt"))
  {
    java::io::InputStream input =
        getClass().getResourceAsStream(L"testRules.txt");
    Files::copy(input, output);
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring path = output->toAbsolutePath()->toString();
  Compile::main(std::deque<wstring>{L"Mtest", path});
  shared_ptr<Path> compiled = dir->resolve(L"testRules.txt.out");
  shared_ptr<Trie> trie = loadTrie(compiled);
  assertTrie(trie, output, true, true);
  assertTrie(trie, output, false, true);
}

shared_ptr<Trie> TestCompile::loadTrie(shared_ptr<Path> path) 
{
  shared_ptr<Trie> trie;
  shared_ptr<DataInputStream> is = make_shared<DataInputStream>(
      make_shared<BufferedInputStream>(Files::newInputStream(path)));
  wstring method = is->readUTF()->toUpperCase(Locale::ROOT);
  if (method.find(L'M') == wstring::npos) {
    trie = make_shared<Trie>(is);
  } else {
    trie = make_shared<MultiTrie>(is);
  }
  is->close();
  return trie;
}

void TestCompile::assertTrie(shared_ptr<Trie> trie, shared_ptr<Path> file,
                             bool usefull, bool storeorig) 
{
  shared_ptr<LineNumberReader> in_ = make_shared<LineNumberReader>(
      Files::newBufferedReader(file, StandardCharsets::UTF_8));

  for (wstring line = in_->readLine(); line != L""; line = in_->readLine()) {
    try {
      line = line.toLowerCase(Locale::ROOT);
      shared_ptr<StringTokenizer> st = make_shared<StringTokenizer>(line);
      wstring stem = st->nextToken();
      if (storeorig) {
        shared_ptr<std::wstring> cmd =
            (usefull) ? trie->getFully(stem) : trie->getLastOnPath(stem);
        shared_ptr<StringBuilder> stm = make_shared<StringBuilder>(stem);
        Diff::apply(stm, cmd);
        assertEquals(stem.toLowerCase(Locale::ROOT),
                     stm->toString()->toLowerCase(Locale::ROOT));
      }
      while (st->hasMoreTokens()) {
        wstring token = st->nextToken();
        if (token == stem) {
          continue;
        }
        shared_ptr<std::wstring> cmd =
            (usefull) ? trie->getFully(token) : trie->getLastOnPath(token);
        shared_ptr<StringBuilder> stm = make_shared<StringBuilder>(token);
        Diff::apply(stm, cmd);
        assertEquals(stem.toLowerCase(Locale::ROOT),
                     stm->toString()->toLowerCase(Locale::ROOT));
      }
    } catch (const NoSuchElementException &x) {
      // no base token (stem) on a line
    }
  }

  in_->close();
}
} // namespace org::egothor::stemmer