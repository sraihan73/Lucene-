using namespace std;

#include "SingleDocSource.h"
#include "DocData.h"
#include "NoMoreDataException.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{

const wstring SingleDocSource::DOC_TEXT =
    wstring(L"Well, this is just some plain text we use for creating the ") +
    L"test documents. It used to be a text from an online collection " +
    L"devoted to first aid, but if there was there an (online) lawyers " +
    L"first aid collection with legal advices, \"it\" might have quite " +
    L"probably advised one not to include \"it\"'s text or the text of " +
    L"any other online collection in one's code, unless one has money " +
    L"that one don't need and one is happy to donate for lawyers " +
    L"charity. Anyhow at some point, rechecking the usage of this text, " +
    L"it became uncertain that this text is free to use, because " +
    L"the web site in the disclaimer of he eBook containing that text " +
    L"was not responding anymore, and at the same time, in projGut, " +
    L"searching for first aid no longer found that eBook as well. " +
    L"So here we are, with a perhaps much less interesting " +
    L"text for the test, but oh much much safer. ";

// C++ WARNING: The following method was originally marked 'synchronized':
int SingleDocSource::newdocid() 
{
  if (docID > 0 && !forever) {
    throw make_shared<NoMoreDataException>();
  }
  return docID++;
}

SingleDocSource::~SingleDocSource() {}

shared_ptr<DocData> SingleDocSource::getNextDocData(
    shared_ptr<DocData> docData) 
{
  int id = newdocid();
  addBytes(DOC_TEXT.length());
  docData->clear();
  docData->setName(L"doc" + to_wstring(id));
  docData->setBody(DOC_TEXT);
  return docData;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void SingleDocSource::resetInputs() 
{
  ContentSource::resetInputs();
  docID = 0;
}
} // namespace org::apache::lucene::benchmark::byTask::feeds