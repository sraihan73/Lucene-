using namespace std;

#include "DictionaryBuilder.h"
#include "ConnectionCostsBuilder.h"
#include "ConnectionCostsWriter.h"
#include "TokenInfoDictionaryBuilder.h"
#include "TokenInfoDictionaryWriter.h"
#include "UnknownDictionaryBuilder.h"
#include "UnknownDictionaryWriter.h"

namespace org::apache::lucene::analysis::ko::util
{

DictionaryBuilder::DictionaryBuilder() {}

void DictionaryBuilder::build(const wstring &inputDirname,
                              const wstring &outputDirname,
                              const wstring &encoding,
                              bool normalizeEntry) 
{
  wcout << L"building tokeninfo dict..." << endl;
  shared_ptr<TokenInfoDictionaryBuilder> tokenInfoBuilder =
      make_shared<TokenInfoDictionaryBuilder>(encoding, normalizeEntry);
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
  wstring inputDirname = args[0];
  wstring outputDirname = args[1];
  wstring inputEncoding = args[2];
  bool normalizeEntries = StringHelper::fromString<bool>(args[3]);

  wcout << L"dictionary builder" << endl;
  wcout << L"" << endl;
  wcout << L"input directory: " << inputDirname << endl;
  wcout << L"output directory: " << outputDirname << endl;
  wcout << L"input encoding: " << inputEncoding << endl;
  wcout << L"normalize entries: " << normalizeEntries << endl;
  wcout << L"" << endl;
  DictionaryBuilder::build(inputDirname, outputDirname, inputEncoding,
                           normalizeEntries);
}
} // namespace org::apache::lucene::analysis::ko::util