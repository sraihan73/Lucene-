using namespace std;

#include "DictionaryBuilder.h"
#include "ConnectionCostsBuilder.h"
#include "ConnectionCostsWriter.h"
#include "TokenInfoDictionaryBuilder.h"
#include "TokenInfoDictionaryWriter.h"
#include "UnknownDictionaryBuilder.h"
#include "UnknownDictionaryWriter.h"

namespace org::apache::lucene::analysis::ja::util
{

DictionaryBuilder::DictionaryBuilder() {}

void DictionaryBuilder::build(DictionaryFormat format,
                              const wstring &inputDirname,
                              const wstring &outputDirname,
                              const wstring &encoding,
                              bool normalizeEntry) 
{
  wcout << L"building tokeninfo dict..." << endl;
  shared_ptr<TokenInfoDictionaryBuilder> tokenInfoBuilder =
      make_shared<TokenInfoDictionaryBuilder>(format, encoding, normalizeEntry);
  shared_ptr<TokenInfoDictionaryWriter> tokenInfoDictionary =
      tokenInfoBuilder->build(inputDirname);
  tokenInfoDictionary->write(outputDirname);
  tokenInfoDictionary.reset();
  tokenInfoBuilder.reset();
  wcout << L"done" << endl;

  wcout << L"building unknown word dict...";
  shared_ptr<UnknownDictionaryBuilder> unkBuilder =
      make_shared<UnknownDictionaryBuilder>(encoding);
  shared_ptr<UnknownDictionaryWriter> unkDictionary =
      unkBuilder->build(inputDirname);
  unkDictionary->write(outputDirname);
  unkDictionary.reset();
  unkBuilder.reset();
  wcout << L"done" << endl;

  wcout << L"building connection costs...";
  shared_ptr<ConnectionCostsWriter> connectionCosts =
      ConnectionCostsBuilder::build(inputDirname + File::separator +
                                    L"matrix.def");
  connectionCosts->write(outputDirname);
  wcout << L"done" << endl;
}

void DictionaryBuilder::main(std::deque<wstring> &args) 
{
  DictionaryFormat format;
  // C++ TODO: The following Java case-insensitive std::wstring method call is not
  // converted:
  if (args[0].equalsIgnoreCase(L"ipadic")) {
    format = DictionaryFormat::IPADIC;
  }
  // C++ TODO: The following Java case-insensitive std::wstring method call is not
  // converted:
  else if (args[0].equalsIgnoreCase(L"unidic")) {
    format = DictionaryFormat::UNIDIC;
  } else {
    System::err::println(L"Illegal format " + args[0] +
                         L" using unidic instead");
    format = DictionaryFormat::IPADIC;
  }

  wstring inputDirname = args[1];
  wstring outputDirname = args[2];
  wstring inputEncoding = args[3];
  bool normalizeEntries = StringHelper::fromString<bool>(args[4]);

  wcout << L"dictionary builder" << endl;
  wcout << L"" << endl;
  wcout << L"dictionary format: " << format << endl;
  wcout << L"input directory: " << inputDirname << endl;
  wcout << L"output directory: " << outputDirname << endl;
  wcout << L"input encoding: " << inputEncoding << endl;
  wcout << L"normalize entries: " << normalizeEntries << endl;
  wcout << L"" << endl;
  DictionaryBuilder::build(format, inputDirname, outputDirname, inputEncoding,
                           normalizeEntries);
}
} // namespace org::apache::lucene::analysis::ja::util