using namespace std;

#include "Test2BFST.h"

namespace org::apache::lucene::util::fst
{
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using MMapDirectory = org::apache::lucene::store::MMapDirectory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TimeUnits = org::apache::lucene::util::TimeUnits;
using com::carrotsearch::randomizedtesting::annotations::TimeoutSuite;
using org::junit::Ignore;
int64_t Test2BFST::LIMIT = 3LL * 1024 * 1024 * 1024;

void Test2BFST::test() 
{
  assumeWorkingMMapOnWindows();

  std::deque<int> ints(7);
  shared_ptr<IntsRef> input = make_shared<IntsRef>(ints, 0, ints.size());
  int64_t seed = random()->nextLong();

  shared_ptr<Directory> dir =
      make_shared<MMapDirectory>(createTempDir(L"2BFST"));

  for (int iter = 0; iter < 1; iter++) {
    {
      // Build FST w/ NoOutputs and stop when nodeCount > 2.2B
      wcout << L"\nTEST: 3B nodes; doPack=false output=NO_OUTPUTS" << endl;
      shared_ptr<Outputs<any>> outputs = NoOutputs::getSingleton();
      any NO_OUTPUT = outputs->getNoOutput();
      shared_ptr<Builder<any>> *const b = make_shared<Builder<any>>(
          FST::INPUT_TYPE::BYTE1, 0, 0, true, true, numeric_limits<int>::max(),
          outputs, true, 15);

      int count = 0;
      shared_ptr<Random> r = make_shared<Random>(seed);
      std::deque<int> ints2(200);
      shared_ptr<IntsRef> input2 = make_shared<IntsRef>(ints2, 0, ints2.size());
      while (true) {
        // System.out.println("add: " + input + " -> " + output);
        for (int i = 10; i < ints2.size(); i++) {
          ints2[i] = r->nextInt(256);
        }
        b->add(input2, NO_OUTPUT);
        count++;
        if (count % 100000 == 0) {
          wcout << count << L": " << b->fstRamBytesUsed() << L" bytes; "
                << b->getNodeCount() << L" nodes" << endl;
        }
        if (b->getNodeCount() >
            numeric_limits<int>::max() + 100LL * 1024 * 1024) {
          break;
        }
        nextInput(r, ints2);
      }

      shared_ptr<FST<any>> fst = b->finish();

      for (int verify = 0; verify < 2; verify++) {
        wcout << L"\nTEST: now verify [fst size=" << fst->ramBytesUsed()
              << L"; nodeCount=" << b->getNodeCount() << L"; arcCount="
              << b->getArcCount() << L"]" << endl;

        Arrays::fill(ints2, 0);
        r = make_shared<Random>(seed);

        for (int i = 0; i < count; i++) {
          if (i % 1000000 == 0) {
            wcout << i << L"...: " << endl;
          }
          for (int j = 10; j < ints2.size(); j++) {
            ints2[j] = r->nextInt(256);
          }
          assertEquals(NO_OUTPUT, Util::get(fst, input2));
          nextInput(r, ints2);
        }

        wcout << L"\nTEST: enum all input/outputs" << endl;
        shared_ptr<IntsRefFSTEnum<any>> fstEnum =
            make_shared<IntsRefFSTEnum<any>>(fst);

        Arrays::fill(ints2, 0);
        r = make_shared<Random>(seed);
        int upto = 0;
        while (true) {
          shared_ptr<IntsRefFSTEnum::InputOutput<any>> pair = fstEnum->next();
          if (pair == nullptr) {
            break;
          }
          for (int j = 10; j < ints2.size(); j++) {
            ints2[j] = r->nextInt(256);
          }
          assertEquals(input2, pair->input);
          assertEquals(NO_OUTPUT, pair->output);
          upto++;
          nextInput(r, ints2);
        }
        assertEquals(count, upto);

        if (verify == 0) {
          wcout << L"\nTEST: save/load FST and re-verify" << endl;
          shared_ptr<IndexOutput> out =
              dir->createOutput(L"fst", IOContext::DEFAULT);
          fst->save(out);
          delete out;
          shared_ptr<IndexInput> in_ =
              dir->openInput(L"fst", IOContext::DEFAULT);
          fst = make_shared<FST<any>>(in_, outputs);
          delete in_;
        } else {
          dir->deleteFile(L"fst");
        }
      }
    }

    // Build FST w/ ByteSequenceOutputs and stop when FST
    // size = 3GB
    {
      wcout << L"\nTEST: 3 GB size; outputs=bytes" << endl;
      shared_ptr<Outputs<std::shared_ptr<BytesRef>>> outputs =
          ByteSequenceOutputs::getSingleton();
      shared_ptr<Builder<std::shared_ptr<BytesRef>>> *const b =
          make_shared<Builder<std::shared_ptr<BytesRef>>>(
              FST::INPUT_TYPE::BYTE1, 0, 0, true, true,
              numeric_limits<int>::max(), outputs, true, 15);

      std::deque<char> outputBytes(20);
      shared_ptr<BytesRef> output = make_shared<BytesRef>(outputBytes);
      Arrays::fill(ints, 0);
      int count = 0;
      shared_ptr<Random> r = make_shared<Random>(seed);
      while (true) {
        r->nextBytes(outputBytes);
        // System.out.println("add: " + input + " -> " + output);
        b->add(input, BytesRef::deepCopyOf(output));
        count++;
        if (count % 1000000 == 0) {
          wcout << count << L"...: " << b->fstRamBytesUsed() << L" bytes"
                << endl;
        }
        if (b->fstRamBytesUsed() > LIMIT) {
          break;
        }
        nextInput(r, ints);
      }

      shared_ptr<FST<std::shared_ptr<BytesRef>>> fst = b->finish();
      for (int verify = 0; verify < 2; verify++) {

        wcout << L"\nTEST: now verify [fst size=" << fst->ramBytesUsed()
              << L"; nodeCount=" << b->getNodeCount() << L"; arcCount="
              << b->getArcCount() << L"]" << endl;

        r = make_shared<Random>(seed);
        Arrays::fill(ints, 0);

        for (int i = 0; i < count; i++) {
          if (i % 1000000 == 0) {
            wcout << i << L"...: " << endl;
          }
          r->nextBytes(outputBytes);
          assertEquals(output, Util::get(fst, input));
          nextInput(r, ints);
        }

        wcout << L"\nTEST: enum all input/outputs" << endl;
        shared_ptr<IntsRefFSTEnum<std::shared_ptr<BytesRef>>> fstEnum =
            make_shared<IntsRefFSTEnum<std::shared_ptr<BytesRef>>>(fst);

        Arrays::fill(ints, 0);
        r = make_shared<Random>(seed);
        int upto = 0;
        while (true) {
          shared_ptr<IntsRefFSTEnum::InputOutput<std::shared_ptr<BytesRef>>>
              pair = fstEnum->next();
          if (pair == nullptr) {
            break;
          }
          assertEquals(input, pair->input);
          r->nextBytes(outputBytes);
          assertEquals(output, pair->output);
          upto++;
          nextInput(r, ints);
        }
        assertEquals(count, upto);

        if (verify == 0) {
          wcout << L"\nTEST: save/load FST and re-verify" << endl;
          shared_ptr<IndexOutput> out =
              dir->createOutput(L"fst", IOContext::DEFAULT);
          fst->save(out);
          delete out;
          shared_ptr<IndexInput> in_ =
              dir->openInput(L"fst", IOContext::DEFAULT);
          fst = make_shared<FST<std::shared_ptr<BytesRef>>>(in_, outputs);
          delete in_;
        } else {
          dir->deleteFile(L"fst");
        }
      }
    }

    // Build FST w/ PositiveIntOutputs and stop when FST
    // size = 3GB
    {
      wcout << L"\nTEST: 3 GB size; outputs=long" << endl;
      shared_ptr<Outputs<int64_t>> outputs =
          PositiveIntOutputs::getSingleton();
      shared_ptr<Builder<int64_t>> *const b = make_shared<Builder<int64_t>>(
          FST::INPUT_TYPE::BYTE1, 0, 0, true, true, numeric_limits<int>::max(),
          outputs, true, 15);

      int64_t output = 1;

      Arrays::fill(ints, 0);
      int count = 0;
      shared_ptr<Random> r = make_shared<Random>(seed);
      while (true) {
        // System.out.println("add: " + input + " -> " + output);
        b->add(input, output);
        output += 1 + r->nextInt(10);
        count++;
        if (count % 1000000 == 0) {
          wcout << count << L"...: " << b->fstRamBytesUsed() << L" bytes"
                << endl;
        }
        if (b->fstRamBytesUsed() > LIMIT) {
          break;
        }
        nextInput(r, ints);
      }

      shared_ptr<FST<int64_t>> fst = b->finish();

      for (int verify = 0; verify < 2; verify++) {

        wcout << L"\nTEST: now verify [fst size=" << fst->ramBytesUsed()
              << L"; nodeCount=" << b->getNodeCount() << L"; arcCount="
              << b->getArcCount() << L"]" << endl;

        Arrays::fill(ints, 0);

        output = 1;
        r = make_shared<Random>(seed);
        for (int i = 0; i < count; i++) {
          if (i % 1000000 == 0) {
            wcout << i << L"...: " << endl;
          }

          // forward lookup:
          assertEquals(output, Util::get(fst, input).longValue());
          // reverse lookup:
          assertEquals(input, Util::getByOutput(fst, output));
          output += 1 + r->nextInt(10);
          nextInput(r, ints);
        }

        wcout << L"\nTEST: enum all input/outputs" << endl;
        shared_ptr<IntsRefFSTEnum<int64_t>> fstEnum =
            make_shared<IntsRefFSTEnum<int64_t>>(fst);

        Arrays::fill(ints, 0);
        r = make_shared<Random>(seed);
        int upto = 0;
        output = 1;
        while (true) {
          shared_ptr<IntsRefFSTEnum::InputOutput<int64_t>> pair =
              fstEnum->next();
          if (pair == nullptr) {
            break;
          }
          assertEquals(input, pair->input);
          assertEquals(output, pair->output.value());
          output += 1 + r->nextInt(10);
          upto++;
          nextInput(r, ints);
        }
        assertEquals(count, upto);

        if (verify == 0) {
          wcout << L"\nTEST: save/load FST and re-verify" << endl;
          shared_ptr<IndexOutput> out =
              dir->createOutput(L"fst", IOContext::DEFAULT);
          fst->save(out);
          delete out;
          shared_ptr<IndexInput> in_ =
              dir->openInput(L"fst", IOContext::DEFAULT);
          fst = make_shared<FST<int64_t>>(in_, outputs);
          delete in_;
        } else {
          dir->deleteFile(L"fst");
        }
      }
    }
  }
  delete dir;
}

void Test2BFST::nextInput(shared_ptr<Random> r, std::deque<int> &ints)
{
  int downTo = 6;
  while (downTo >= 0) {
    // Must add random amounts (and not just 1) because
    // otherwise FST outsmarts us and remains tiny:
    ints[downTo] += 1 + r->nextInt(10);
    if (ints[downTo] < 256) {
      break;
    } else {
      ints[downTo] = 0;
      downTo--;
    }
  }
}
} // namespace org::apache::lucene::util::fst