using namespace std;

#include "StempelStemmer.h"
#include "../../../../egothor/stemmer/Diff.h"
#include "../../../../egothor/stemmer/MultiTrie2.h"
#include "../../../../egothor/stemmer/Trie.h"

namespace org::apache::lucene::analysis::stempel
{
using Diff = org::egothor::stemmer::Diff;
using Trie = org::egothor::stemmer::Trie;

StempelStemmer::StempelStemmer(shared_ptr<InputStream> stemmerTable) throw(
    IOException)
    : StempelStemmer(load(stemmerTable))
{
}

StempelStemmer::StempelStemmer(shared_ptr<Trie> stemmer)
{
  this->stemmer = stemmer;
}

shared_ptr<Trie>
StempelStemmer::load(shared_ptr<InputStream> stemmerTable) 
{
  shared_ptr<DataInputStream> in_ = nullptr;
  try {
    in_ = make_shared<DataInputStream>(
        make_shared<BufferedInputStream>(stemmerTable));
    wstring method = in_->readUTF()->toUpperCase(Locale::ROOT);
    if (method.find(L'M') == wstring::npos) {
      return make_shared<Trie>(in_);
    } else {
      return make_shared<org::egothor::stemmer::MultiTrie2>(in_);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    in_->close();
  }
}

shared_ptr<StringBuilder> StempelStemmer::stem(shared_ptr<std::wstring> word)
{
  shared_ptr<std::wstring> cmd = stemmer->getLastOnPath(word);

  if (cmd == nullptr) {
    return nullptr;
  }

  buffer->setLength(0);
  buffer->append(word);

  Diff::apply(buffer, cmd);

  if (buffer->length() > 0) {
    return buffer;
  } else {
    return nullptr;
  }
}
} // namespace org::apache::lucene::analysis::stempel