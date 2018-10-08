using namespace std;

#include "TestReaderWrapperDVTypeCheck.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestReaderWrapperDVTypeCheck::testNoDVFieldOnSegment() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> cfg =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setCodec(TestUtil::alwaysDocValuesFormat(
              TestUtil::getDefaultDocValuesFormat()));
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, cfg);

  bool sdvExist = false;
  bool ssdvExist = false;

  constexpr int64_t seed = random()->nextLong();
  {
    shared_ptr<Random> *const indexRandom = make_shared<Random>(seed);
    constexpr int docs;
    docs = TestUtil::nextInt(indexRandom, 1, 4);
    // System.out.println("docs:"+docs);

    for (int i = 0; i < docs; i++) {
      shared_ptr<Document> d = make_shared<Document>();
      d->push_back(newStringField(L"id", L"" + to_wstring(i), Store::NO));
      if (rarely(indexRandom)) {
        // System.out.println("on:"+i+" rarely: true");
        d->push_back(make_shared<SortedDocValuesField>(
            L"sdv", make_shared<BytesRef>(L"" + to_wstring(i))));
        sdvExist = true;
      } else {
        // System.out.println("on:"+i+" rarely: false");
      }
      constexpr int numSortedSet = indexRandom->nextInt(5) - 3;
      for (int j = 0; j < numSortedSet; ++j) {
        // System.out.println("on:"+i+" add ssdv:"+j);
        d->push_back(make_shared<SortedSetDocValuesField>(
            L"ssdv", make_shared<BytesRef>(L"" + to_wstring(j))));
        ssdvExist = true;
      }
      iw->addDocument(d);
      iw->commit();
    }
  }
  iw->forceMerge(1);
  shared_ptr<DirectoryReader> *const reader = iw->getReader();

  // System.out.println("sdv:"+ sdvExist+ " ssdv:"+ssdvExist+", segs:
  // "+reader.leaves().size() +", "+reader.leaves());

  delete iw;
  shared_ptr<LeafReader> *const wrapper = getOnlyLeafReader(reader);

  {
    // final Random indexRandom = new Random(seed);
    shared_ptr<SortedDocValues> *const sdv =
        wrapper->getSortedDocValues(L"sdv");
    shared_ptr<SortedSetDocValues> *const ssdv =
        wrapper->getSortedSetDocValues(L"ssdv");

    assertNull(L"confusing DV type", wrapper->getSortedDocValues(L"ssdv"));
    assertNull(L"confusing DV type", wrapper->getSortedSetDocValues(L"sdv"));

    assertNull(L"absent field", wrapper->getSortedDocValues(L"NOssdv"));
    assertNull(L"absent field", wrapper->getSortedSetDocValues(L"NOsdv"));

    assertTrue(L"optional sdv field", sdvExist == (sdv != nullptr));
    assertTrue(L"optional ssdv field", ssdvExist == (ssdv != nullptr));
  }
  reader->close();

  delete dir;
}
} // namespace org::apache::lucene::index