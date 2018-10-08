using namespace std;

#include "BaseTokenStreamTestCase.h"

namespace org::apache::lucene::analysis
{
using namespace org::apache::lucene::analysis::tokenattributes;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexableFieldType = org::apache::lucene::index::IndexableFieldType;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using Attribute = org::apache::lucene::util::Attribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRef = org::apache::lucene::util::IntsRef;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Rethrow = org::apache::lucene::util::Rethrow;
using TestUtil = org::apache::lucene::util::TestUtil;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using AutomatonTestUtil =
    org::apache::lucene::util::automaton::AutomatonTestUtil;
using Util = org::apache::lucene::util::fst::Util;

bool BaseTokenStreamTestCase::CheckClearAttributesAttributeImpl::
    getAndResetClearCalled()
{
  try {
    return clearCalled;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    clearCalled = false;
  }
}

void BaseTokenStreamTestCase::CheckClearAttributesAttributeImpl::clear()
{
  clearCalled = true;
}

bool BaseTokenStreamTestCase::CheckClearAttributesAttributeImpl::equals(
    any other)
{
  return (std::dynamic_pointer_cast<CheckClearAttributesAttributeImpl>(other) !=
              nullptr &&
          (any_cast<std::shared_ptr<CheckClearAttributesAttributeImpl>>(other))
                  ->clearCalled == this->clearCalled);
}

int BaseTokenStreamTestCase::CheckClearAttributesAttributeImpl::hashCode()
{
  return 76137213 ^ static_cast<Boolean>(clearCalled).hashCode();
}

void BaseTokenStreamTestCase::CheckClearAttributesAttributeImpl::copyTo(
    shared_ptr<AttributeImpl> target)
{
  (std::static_pointer_cast<CheckClearAttributesAttributeImpl>(target))
      ->clear();
}

void BaseTokenStreamTestCase::CheckClearAttributesAttributeImpl::reflectWith(
    AttributeReflector reflector)
{
  reflector(CheckClearAttributesAttribute::typeid, L"clearCalled", clearCalled);
}

void BaseTokenStreamTestCase::assertTokenStreamContents(
    shared_ptr<TokenStream> ts, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    std::deque<wstring> &types, std::deque<int> &posIncrements,
    std::deque<int> &posLengths, optional<int> &finalOffset,
    optional<int> &finalPosInc, std::deque<bool> &keywordAtts,
    bool graphOffsetsAreCorrect,
    std::deque<std::deque<char>> &payloads) 
{
  assertNotNull(output);
  shared_ptr<CheckClearAttributesAttribute> checkClearAtt =
      ts->addAttribute(CheckClearAttributesAttribute::typeid);

  shared_ptr<CharTermAttribute> termAtt = nullptr;
  if (output.size() > 0) {
    assertTrue(L"has no CharTermAttribute",
               ts->hasAttribute(CharTermAttribute::typeid));
    termAtt = ts->getAttribute(CharTermAttribute::typeid);
  }

  shared_ptr<OffsetAttribute> offsetAtt = nullptr;
  if (startOffsets.size() > 0 || endOffsets.size() > 0 || finalOffset) {
    assertTrue(L"has no OffsetAttribute",
               ts->hasAttribute(OffsetAttribute::typeid));
    offsetAtt = ts->getAttribute(OffsetAttribute::typeid);
  }

  shared_ptr<TypeAttribute> typeAtt = nullptr;
  if (types.size() > 0) {
    assertTrue(L"has no TypeAttribute",
               ts->hasAttribute(TypeAttribute::typeid));
    typeAtt = ts->getAttribute(TypeAttribute::typeid);
  }

  shared_ptr<PositionIncrementAttribute> posIncrAtt = nullptr;
  if (posIncrements.size() > 0 || finalPosInc) {
    assertTrue(L"has no PositionIncrementAttribute",
               ts->hasAttribute(PositionIncrementAttribute::typeid));
    posIncrAtt = ts->getAttribute(PositionIncrementAttribute::typeid);
  }

  shared_ptr<PositionLengthAttribute> posLengthAtt = nullptr;
  if (posLengths.size() > 0) {
    assertTrue(L"has no PositionLengthAttribute",
               ts->hasAttribute(PositionLengthAttribute::typeid));
    posLengthAtt = ts->getAttribute(PositionLengthAttribute::typeid);
  }

  shared_ptr<KeywordAttribute> keywordAtt = nullptr;
  if (keywordAtts.size() > 0) {
    assertTrue(L"has no KeywordAttribute",
               ts->hasAttribute(KeywordAttribute::typeid));
    keywordAtt = ts->getAttribute(KeywordAttribute::typeid);
  }

  shared_ptr<PayloadAttribute> payloadAtt = nullptr;
  if (payloads.size() > 0) {
    assertTrue(L"has no PayloadAttribute",
               ts->hasAttribute(PayloadAttribute::typeid));
    payloadAtt = ts->getAttribute(PayloadAttribute::typeid);
  }

  // Maps position to the start/end offset:
  const unordered_map<int, int> posToStartOffset = unordered_map<int, int>();
  const unordered_map<int, int> posToEndOffset = unordered_map<int, int>();

  // TODO: would be nice to be able to assert silly duplicated tokens are not
  // created, but a number of cases do this "legitimately": LUCENE-7622

  ts->reset();
  int pos = -1;
  int lastStartOffset = 0;
  for (int i = 0; i < output.size(); i++) {
    // extra safety to enforce, that the state is not preserved and also assign
    // bogus values
    ts->clearAttributes();
    termAtt->setEmpty()->append(L"bogusTerm");
    if (offsetAtt != nullptr) {
      offsetAtt->setOffset(14584724, 24683243);
    }
    if (typeAtt != nullptr) {
      typeAtt->setType(L"bogusType");
    }
    if (posIncrAtt != nullptr) {
      posIncrAtt->setPositionIncrement(45987657);
    }
    if (posLengthAtt != nullptr) {
      posLengthAtt->setPositionLength(45987653);
    }
    if (keywordAtt != nullptr) {
      keywordAtt->setKeyword((i & 1) == 0);
    }
    if (payloadAtt != nullptr) {
      payloadAtt->setPayload(make_shared<BytesRef>(
          std::deque<char>{0x00, -0x21, 0x12, -0x43, 0x24}));
    }

    checkClearAtt->getAndResetClearCalled(); // reset it, because we called
                                             // clearAttribute() before
    assertTrue(L"token " + to_wstring(i) + L" does not exist",
               ts->incrementToken());
    assertTrue(L"clearAttributes() was not called correctly in TokenStream "
               L"chain at token " +
                   to_wstring(i),
               checkClearAtt->getAndResetClearCalled());

    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"term " + to_wstring(i), output[i], termAtt->toString());
    if (startOffsets.size() > 0) {
      assertEquals(L"startOffset " + to_wstring(i) + L" term=" + termAtt,
                   startOffsets[i], offsetAtt->startOffset());
    }
    if (endOffsets.size() > 0) {
      assertEquals(L"endOffset " + to_wstring(i) + L" term=" + termAtt,
                   endOffsets[i], offsetAtt->endOffset());
    }
    if (types.size() > 0) {
      assertEquals(L"type " + to_wstring(i) + L" term=" + termAtt, types[i],
                   typeAtt->type());
    }
    if (posIncrements.size() > 0) {
      assertEquals(L"posIncrement " + to_wstring(i) + L" term=" + termAtt,
                   posIncrements[i], posIncrAtt->getPositionIncrement());
    }
    if (posLengths.size() > 0) {
      assertEquals(L"posLength " + to_wstring(i) + L" term=" + termAtt,
                   posLengths[i], posLengthAtt->getPositionLength());
    }
    if (keywordAtts.size() > 0) {
      assertEquals(L"keywordAtt " + to_wstring(i) + L" term=" + termAtt,
                   keywordAtts[i], keywordAtt->isKeyword());
    }
    if (payloads.size() > 0) {
      if (payloads[i].size() > 0) {
        assertEquals(L"payloads " + to_wstring(i),
                     make_shared<BytesRef>(payloads[i]),
                     payloadAtt->getPayload());
      } else {
        assertNull(L"payloads " + to_wstring(i), payloads[i]);
      }
    }
    if (posIncrAtt != nullptr) {
      if (i == 0) {
        assertTrue(L"first posIncrement must be >= 1",
                   posIncrAtt->getPositionIncrement() >= 1);
      } else {
        assertTrue(L"posIncrement must be >= 0",
                   posIncrAtt->getPositionIncrement() >= 0);
      }
    }
    if (posLengthAtt != nullptr) {
      assertTrue(L"posLength must be >= 1; got: " +
                     to_wstring(posLengthAtt->getPositionLength()),
                 posLengthAtt->getPositionLength() >= 1);
    }
    // we can enforce some basic things about a few attributes even if the
    // caller doesn't check:
    if (offsetAtt != nullptr) {
      constexpr int startOffset = offsetAtt->startOffset();
      constexpr int endOffset = offsetAtt->endOffset();
      if (finalOffset) {
        assertTrue(L"startOffset (= " + to_wstring(startOffset) +
                       L") must be <= finalOffset (= " + finalOffset +
                       L") term=" + termAtt,
                   startOffset <= finalOffset.value());
        assertTrue(L"endOffset must be <= finalOffset: got endOffset=" +
                       to_wstring(endOffset) + L" vs finalOffset=" +
                       finalOffset.value() + L" term=" + termAtt,
                   endOffset <= finalOffset.value());
      }

      assertTrue(L"offsets must not go backwards startOffset=" +
                     to_wstring(startOffset) + L" is < lastStartOffset=" +
                     to_wstring(lastStartOffset) + L" term=" + termAtt,
                 offsetAtt->startOffset() >= lastStartOffset);
      lastStartOffset = offsetAtt->startOffset();

      if (graphOffsetsAreCorrect && posLengthAtt != nullptr &&
          posIncrAtt != nullptr) {
        // Validate offset consistency in the graph, ie
        // all tokens leaving from a certain pos have the
        // same startOffset, and all tokens arriving to a
        // certain pos have the same endOffset:
        constexpr int posInc = posIncrAtt->getPositionIncrement();
        pos += posInc;

        constexpr int posLength = posLengthAtt->getPositionLength();

        if (posToStartOffset.find(pos) == posToStartOffset.end()) {
          // First time we've seen a token leaving from this position:
          posToStartOffset.emplace(pos, startOffset);
          // System.out.println("  + s " + pos + " -> " + startOffset);
        } else {
          // We've seen a token leaving from this position
          // before; verify the startOffset is the same:
          // System.out.println("  + vs " + pos + " -> " + startOffset);
          assertEquals(to_wstring(i) + L" inconsistent startOffset: pos=" +
                           to_wstring(pos) + L" posLen=" +
                           to_wstring(posLength) + L" token=" + termAtt,
                       posToStartOffset[pos], startOffset);
        }

        constexpr int endPos = pos + posLength;

        if (posToEndOffset.find(endPos) == posToEndOffset.end()) {
          // First time we've seen a token arriving to this position:
          posToEndOffset.emplace(endPos, endOffset);
          // System.out.println("  + e " + endPos + " -> " + endOffset);
        } else {
          // We've seen a token arriving to this position
          // before; verify the endOffset is the same:
          // System.out.println("  + ve " + endPos + " -> " + endOffset);
          assertEquals(L"inconsistent endOffset " + to_wstring(i) + L" pos=" +
                           to_wstring(pos) + L" posLen=" +
                           to_wstring(posLength) + L" token=" + termAtt,
                       posToEndOffset[endPos], endOffset);
        }
      }
    }
  }

  if (ts->incrementToken()) {
    fail(L"TokenStream has more tokens than expected (expected count=" +
         output.size() + L"); extra token=" +
         ts->getAttribute(CharTermAttribute::typeid));
  }

  // repeat our extra safety checks for end()
  ts->clearAttributes();
  if (termAtt != nullptr) {
    termAtt->setEmpty()->append(L"bogusTerm");
  }
  if (offsetAtt != nullptr) {
    offsetAtt->setOffset(14584724, 24683243);
  }
  if (typeAtt != nullptr) {
    typeAtt->setType(L"bogusType");
  }
  if (posIncrAtt != nullptr) {
    posIncrAtt->setPositionIncrement(45987657);
  }
  if (posLengthAtt != nullptr) {
    posLengthAtt->setPositionLength(45987653);
  }
  if (keywordAtt != nullptr) {
    keywordAtt->setKeyword(true);
  }
  if (payloadAtt != nullptr) {
    payloadAtt->setPayload(make_shared<BytesRef>(
        std::deque<char>{0x00, -0x21, 0x12, -0x43, 0x24}));
  }

  checkClearAtt->getAndResetClearCalled(); // reset it, because we called
                                           // clearAttribute() before

  ts->end();
  assertTrue(L"super.end()/clearAttributes() was not called correctly in end()",
             checkClearAtt->getAndResetClearCalled());

  if (finalOffset) {
    assertEquals(L"finalOffset", finalOffset.value(), offsetAtt->endOffset());
  }
  if (offsetAtt != nullptr) {
    assertTrue(L"finalOffset must be >= 0", offsetAtt->endOffset() >= 0);
  }
  if (finalPosInc) {
    assertEquals(L"finalPosInc", finalPosInc.value(),
                 posIncrAtt->getPositionIncrement());
  }

  delete ts;
}

void BaseTokenStreamTestCase::assertTokenStreamContents(
    shared_ptr<TokenStream> ts, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    std::deque<wstring> &types, std::deque<int> &posIncrements,
    std::deque<int> &posLengths, optional<int> &finalOffset,
    std::deque<bool> &keywordAtts,
    bool graphOffsetsAreCorrect) 
{
  assertTokenStreamContents(ts, output, startOffsets, endOffsets, types,
                            posIncrements, posLengths, finalOffset, nullopt,
                            keywordAtts, graphOffsetsAreCorrect, nullptr);
}

void BaseTokenStreamTestCase::assertTokenStreamContents(
    shared_ptr<TokenStream> ts, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    std::deque<wstring> &types, std::deque<int> &posIncrements,
    std::deque<int> &posLengths, optional<int> &finalOffset,
    bool graphOffsetsAreCorrect) 
{
  assertTokenStreamContents(ts, output, startOffsets, endOffsets, types,
                            posIncrements, posLengths, finalOffset, nullptr,
                            graphOffsetsAreCorrect);
}

void BaseTokenStreamTestCase::assertTokenStreamContents(
    shared_ptr<TokenStream> ts, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    std::deque<wstring> &types, std::deque<int> &posIncrements,
    std::deque<int> &posLengths, optional<int> &finalOffset) 
{
  assertTokenStreamContents(ts, output, startOffsets, endOffsets, types,
                            posIncrements, posLengths, finalOffset, true);
}

void BaseTokenStreamTestCase::assertTokenStreamContents(
    shared_ptr<TokenStream> ts, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    std::deque<wstring> &types, std::deque<int> &posIncrements,
    optional<int> &finalOffset) 
{
  assertTokenStreamContents(ts, output, startOffsets, endOffsets, types,
                            posIncrements, nullptr, finalOffset);
}

void BaseTokenStreamTestCase::assertTokenStreamContents(
    shared_ptr<TokenStream> ts, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    std::deque<wstring> &types,
    std::deque<int> &posIncrements) 
{
  assertTokenStreamContents(ts, output, startOffsets, endOffsets, types,
                            posIncrements, nullptr, nullopt);
}

void BaseTokenStreamTestCase::assertTokenStreamContents(
    shared_ptr<TokenStream> ts, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    std::deque<wstring> &types, std::deque<int> &posIncrements,
    std::deque<int> &posLengths) 
{
  assertTokenStreamContents(ts, output, startOffsets, endOffsets, types,
                            posIncrements, posLengths, nullopt);
}

void BaseTokenStreamTestCase::assertTokenStreamContents(
    shared_ptr<TokenStream> ts, std::deque<wstring> &output) 
{
  assertTokenStreamContents(ts, output, nullptr, nullptr, nullptr, nullptr,
                            nullptr, nullopt);
}

void BaseTokenStreamTestCase::assertTokenStreamContents(
    shared_ptr<TokenStream> ts, std::deque<wstring> &output,
    std::deque<wstring> &types) 
{
  assertTokenStreamContents(ts, output, nullptr, nullptr, types, nullptr,
                            nullptr, nullopt);
}

void BaseTokenStreamTestCase::assertTokenStreamContents(
    shared_ptr<TokenStream> ts, std::deque<wstring> &output,
    std::deque<int> &posIncrements) 
{
  assertTokenStreamContents(ts, output, nullptr, nullptr, nullptr,
                            posIncrements, nullptr, nullopt);
}

void BaseTokenStreamTestCase::assertTokenStreamContents(
    shared_ptr<TokenStream> ts, std::deque<wstring> &output,
    std::deque<int> &startOffsets,
    std::deque<int> &endOffsets) 
{
  assertTokenStreamContents(ts, output, startOffsets, endOffsets, nullptr,
                            nullptr, nullptr, nullopt);
}

void BaseTokenStreamTestCase::assertTokenStreamContents(
    shared_ptr<TokenStream> ts, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    optional<int> &finalOffset) 
{
  assertTokenStreamContents(ts, output, startOffsets, endOffsets, nullptr,
                            nullptr, nullptr, finalOffset);
}

void BaseTokenStreamTestCase::assertTokenStreamContents(
    shared_ptr<TokenStream> ts, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    std::deque<int> &posIncrements) 
{
  assertTokenStreamContents(ts, output, startOffsets, endOffsets, nullptr,
                            posIncrements, nullptr, nullopt);
}

void BaseTokenStreamTestCase::assertTokenStreamContents(
    shared_ptr<TokenStream> ts, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    std::deque<int> &posIncrements,
    optional<int> &finalOffset) 
{
  assertTokenStreamContents(ts, output, startOffsets, endOffsets, nullptr,
                            posIncrements, nullptr, finalOffset);
}

void BaseTokenStreamTestCase::assertTokenStreamContents(
    shared_ptr<TokenStream> ts, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    std::deque<int> &posIncrements, std::deque<int> &posLengths,
    optional<int> &finalOffset) 
{
  assertTokenStreamContents(ts, output, startOffsets, endOffsets, nullptr,
                            posIncrements, posLengths, finalOffset);
}

void BaseTokenStreamTestCase::assertAnalyzesTo(
    shared_ptr<Analyzer> a, const wstring &input, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    std::deque<wstring> &types,
    std::deque<int> &posIncrements) 
{
  checkResetException(a, input);
  checkAnalysisConsistency(random(), a, true, input);
  assertTokenStreamContents(a->tokenStream(L"dummy", input), output,
                            startOffsets, endOffsets, types, posIncrements,
                            nullptr, input.length());
}

void BaseTokenStreamTestCase::assertAnalyzesTo(
    shared_ptr<Analyzer> a, const wstring &input, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    std::deque<wstring> &types, std::deque<int> &posIncrements,
    std::deque<int> &posLengths) 
{
  checkResetException(a, input);
  checkAnalysisConsistency(random(), a, true, input);
  assertTokenStreamContents(a->tokenStream(L"dummy", input), output,
                            startOffsets, endOffsets, types, posIncrements,
                            posLengths, input.length());
}

void BaseTokenStreamTestCase::assertAnalyzesTo(
    shared_ptr<Analyzer> a, const wstring &input, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    std::deque<wstring> &types, std::deque<int> &posIncrements,
    std::deque<int> &posLengths,
    bool graphOffsetsAreCorrect) 
{
  checkResetException(a, input);
  checkAnalysisConsistency(random(), a, true, input, graphOffsetsAreCorrect);
  assertTokenStreamContents(a->tokenStream(L"dummy", input), output,
                            startOffsets, endOffsets, types, posIncrements,
                            posLengths, input.length(), graphOffsetsAreCorrect);
}

void BaseTokenStreamTestCase::assertAnalyzesTo(
    shared_ptr<Analyzer> a, const wstring &input, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    std::deque<wstring> &types, std::deque<int> &posIncrements,
    std::deque<int> &posLengths, bool graphOffsetsAreCorrect,
    std::deque<std::deque<char>> &payloads) 
{
  checkResetException(a, input);
  assertTokenStreamContents(a->tokenStream(L"dummy", input), output,
                            startOffsets, endOffsets, types, posIncrements,
                            posLengths, input.length(), nullopt, nullptr,
                            graphOffsetsAreCorrect, payloads);
}

void BaseTokenStreamTestCase::assertAnalyzesTo(
    shared_ptr<Analyzer> a, const wstring &input,
    std::deque<wstring> &output) 
{
  assertAnalyzesTo(a, input, output, nullptr, nullptr, nullptr, nullptr,
                   nullptr);
}

void BaseTokenStreamTestCase::assertAnalyzesTo(
    shared_ptr<Analyzer> a, const wstring &input, std::deque<wstring> &output,
    std::deque<wstring> &types) 
{
  assertAnalyzesTo(a, input, output, nullptr, nullptr, types, nullptr, nullptr);
}

void BaseTokenStreamTestCase::assertAnalyzesTo(
    shared_ptr<Analyzer> a, const wstring &input, std::deque<wstring> &output,
    std::deque<int> &posIncrements) 
{
  assertAnalyzesTo(a, input, output, nullptr, nullptr, nullptr, posIncrements,
                   nullptr);
}

void BaseTokenStreamTestCase::assertAnalyzesToPositions(
    shared_ptr<Analyzer> a, const wstring &input, std::deque<wstring> &output,
    std::deque<int> &posIncrements,
    std::deque<int> &posLengths) 
{
  assertAnalyzesTo(a, input, output, nullptr, nullptr, nullptr, posIncrements,
                   posLengths);
}

void BaseTokenStreamTestCase::assertAnalyzesToPositions(
    shared_ptr<Analyzer> a, const wstring &input, std::deque<wstring> &output,
    std::deque<wstring> &types, std::deque<int> &posIncrements,
    std::deque<int> &posLengths) 
{
  assertAnalyzesTo(a, input, output, nullptr, nullptr, types, posIncrements,
                   posLengths);
}

void BaseTokenStreamTestCase::assertAnalyzesTo(
    shared_ptr<Analyzer> a, const wstring &input, std::deque<wstring> &output,
    std::deque<int> &startOffsets,
    std::deque<int> &endOffsets) 
{
  assertAnalyzesTo(a, input, output, startOffsets, endOffsets, nullptr, nullptr,
                   nullptr);
}

void BaseTokenStreamTestCase::assertAnalyzesTo(
    shared_ptr<Analyzer> a, const wstring &input, std::deque<wstring> &output,
    std::deque<int> &startOffsets, std::deque<int> &endOffsets,
    std::deque<int> &posIncrements) 
{
  assertAnalyzesTo(a, input, output, startOffsets, endOffsets, nullptr,
                   posIncrements, nullptr);
}

void BaseTokenStreamTestCase::checkResetException(
    shared_ptr<Analyzer> a, const wstring &input) 
{
  shared_ptr<TokenStream> ts = a->tokenStream(L"bogus", input);
  try {
    if (ts->incrementToken()) {
      // System.out.println(ts.reflectAsString(false));
      fail(L"didn't get expected exception when reset() not called");
    }
  } catch (const IllegalStateException &expected) {
    // ok
  } catch (const runtime_error &unexpected) {
    unexpected.printStackTrace(System::err);
    fail(L"got wrong exception when reset() not called: " + unexpected);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // consume correctly
    ts->reset();
    while (ts->incrementToken()) {
    }
    ts->end();
    delete ts;
  }

  // check for a missing close()
  ts = a->tokenStream(L"bogus", input);
  ts->reset();
  while (ts->incrementToken()) {
  }
  ts->end();
  try {
    ts = a->tokenStream(L"bogus", input);
    fail(L"didn't get expected exception when close() not called");
  } catch (const IllegalStateException &expected) {
    // ok
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete ts;
  }
}

void BaseTokenStreamTestCase::checkOneTerm(
    shared_ptr<Analyzer> a, const wstring &input,
    const wstring &expected) 
{
  assertAnalyzesTo(a, input, std::deque<wstring>{expected});
}

void BaseTokenStreamTestCase::checkRandomData(shared_ptr<Random> random,
                                              shared_ptr<Analyzer> a,
                                              int iterations) 
{
  checkRandomData(random, a, iterations, 20, false, true);
}

void BaseTokenStreamTestCase::checkRandomData(
    shared_ptr<Random> random, shared_ptr<Analyzer> a, int iterations,
    int maxWordLength) 
{
  checkRandomData(random, a, iterations, maxWordLength, false, true);
}

void BaseTokenStreamTestCase::checkRandomData(shared_ptr<Random> random,
                                              shared_ptr<Analyzer> a,
                                              int iterations,
                                              bool simple) 
{
  checkRandomData(random, a, iterations, 20, simple, true);
}

void BaseTokenStreamTestCase::assertStreamHasNumberOfTokens(
    shared_ptr<TokenStream> ts, int expectedCount) 
{
  ts->reset();
  int count = 0;
  while (ts->incrementToken()) {
    count++;
  }
  ts->end();
  assertEquals(L"wrong number of tokens", expectedCount, count);
}

BaseTokenStreamTestCase::AnalysisThread::AnalysisThread(
    int64_t seed, shared_ptr<CountDownLatch> latch, shared_ptr<Analyzer> a,
    int iterations, int maxWordLength, bool useCharFilter, bool simple,
    bool graphOffsetsAreCorrect, shared_ptr<RandomIndexWriter> iw)
    : iterations(iterations), maxWordLength(maxWordLength), seed(seed), a(a),
      useCharFilter(useCharFilter), simple(simple),
      graphOffsetsAreCorrect(graphOffsetsAreCorrect), iw(iw), latch(latch)
{
}

void BaseTokenStreamTestCase::AnalysisThread::run()
{
  bool success = false;
  try {
    latch->await();
    // see the part in checkRandomData where it replays the same text again
    // to verify reproducability/reuse: hopefully this would catch thread
    // hazards.
    checkRandomData(make_shared<Random>(seed), a, iterations, maxWordLength,
                    useCharFilter, simple, graphOffsetsAreCorrect, iw);
    success = true;
  } catch (const runtime_error &e) {
    Rethrow::rethrow(e);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    failed = !success;
  }
}

void BaseTokenStreamTestCase::checkRandomData(shared_ptr<Random> random,
                                              shared_ptr<Analyzer> a,
                                              int iterations, int maxWordLength,
                                              bool simple) 
{
  checkRandomData(random, a, iterations, maxWordLength, simple, true);
}

void BaseTokenStreamTestCase::checkRandomData(
    shared_ptr<Random> random, shared_ptr<Analyzer> a, int iterations,
    int maxWordLength, bool simple,
    bool graphOffsetsAreCorrect) 
{
  checkResetException(a, L"best effort");
  int64_t seed = random->nextLong();
  bool useCharFilter = random->nextBoolean();
  shared_ptr<Directory> dir = nullptr;
  shared_ptr<RandomIndexWriter> iw = nullptr;
  const wstring postingsFormat = TestUtil::getPostingsFormat(L"dummy");
  bool codecOk =
      iterations * maxWordLength < 100000 ||
      !(postingsFormat == L"Memory" || postingsFormat == L"SimpleText");
  if (rarely(random) && codecOk) {
    dir = newFSDirectory(createTempDir(L"bttc"));
    iw = make_shared<RandomIndexWriter>(make_shared<Random>(seed), dir, a);
  }
  bool success = false;
  try {
    checkRandomData(make_shared<Random>(seed), a, iterations, maxWordLength,
                    useCharFilter, simple, graphOffsetsAreCorrect, iw);
    // now test with multiple threads: note we do the EXACT same thing we did
    // before in each thread, so this should only really fail from another
    // thread if it's an actual thread problem
    int numThreads = TestUtil::nextInt(random, 2, 4);
    shared_ptr<CountDownLatch> *const startingGun =
        make_shared<CountDownLatch>(1);
    std::deque<std::shared_ptr<AnalysisThread>> threads(numThreads);
    for (int i = 0; i < threads.size(); i++) {
      threads[i] = make_shared<AnalysisThread>(
          seed, startingGun, a, iterations, maxWordLength, useCharFilter,
          simple, graphOffsetsAreCorrect, iw);
    }
    for (int i = 0; i < threads.size(); i++) {
      threads[i]->start();
    }
    startingGun->countDown();
    for (int i = 0; i < threads.size(); i++) {
      try {
        threads[i]->join();
      } catch (const InterruptedException &e) {
        throw runtime_error(e);
      }
    }
    for (int i = 0; i < threads.size(); i++) {
      if (threads[i]->failed) {
        throw runtime_error(L"some thread(s) failed");
      }
    }
    if (iw != nullptr) {
      delete iw;
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({dir});
    } else {
      IOUtils::closeWhileHandlingException({dir}); // checkindex
    }
  }
}

void BaseTokenStreamTestCase::checkRandomData(
    shared_ptr<Random> random, shared_ptr<Analyzer> a, int iterations,
    int maxWordLength, bool useCharFilter, bool simple,
    bool graphOffsetsAreCorrect,
    shared_ptr<RandomIndexWriter> iw) 
{

  shared_ptr<LineFileDocs> *const docs = make_shared<LineFileDocs>(random);
  shared_ptr<Document> doc;
  shared_ptr<Field> field = nullptr, currentField = nullptr;
  shared_ptr<StringReader> bogus = make_shared<StringReader>(L"");
  if (iw != nullptr) {
    doc = make_shared<Document>();
    shared_ptr<FieldType> ft =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    if (random->nextBoolean()) {
      ft->setStoreTermVectors(true);
      ft->setStoreTermVectorOffsets(random->nextBoolean());
      ft->setStoreTermVectorPositions(random->nextBoolean());
      if (ft->storeTermVectorPositions()) {
        ft->setStoreTermVectorPayloads(random->nextBoolean());
      }
    }
    if (random->nextBoolean()) {
      ft->setOmitNorms(true);
    }
    switch (random->nextInt(4)) {
    case 0:
      ft->setIndexOptions(IndexOptions::DOCS);
      break;
    case 1:
      ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
      break;
    case 2:
      ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
      break;
    default:
      ft->setIndexOptions(
          IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
    }
    currentField = field = make_shared<Field>(L"dummy", bogus, ft);
    doc->push_back(currentField);
  }

  try {
    for (int i = 0; i < iterations; i++) {
      wstring text;

      if (random->nextInt(10) == 7) {
        // real data from linedocs
        text = docs->nextDoc()[L"body"];
        if (text.length() > maxWordLength) {

          // Take a random slice from the text...:
          int startPos = random->nextInt(text.length() - maxWordLength);
          if (startPos > 0 && Character::isLowSurrogate(text[startPos])) {
            // Take care not to split up a surrogate pair:
            startPos--;
            assert(Character::isHighSurrogate(text[startPos]));
          }
          int endPos = startPos + maxWordLength - 1;
          if (Character::isHighSurrogate(text[endPos])) {
            // Take care not to split up a surrogate pair:
            endPos--;
          }
          text = text.substr(startPos, (1 + endPos) - startPos);
        }
      } else {
        // synthetic
        text = TestUtil::randomAnalysisString(random, maxWordLength, simple);
      }

      try {
        checkAnalysisConsistency(random, a, useCharFilter, text,
                                 graphOffsetsAreCorrect, currentField);
        if (iw != nullptr) {
          if (random->nextInt(7) == 0) {
            // pile up a multivalued field
            shared_ptr<IndexableFieldType> ft = field->fieldType();
            currentField = make_shared<Field>(L"dummy", bogus, ft);
            doc->push_back(currentField);
          } else {
            iw->addDocument(doc);
            if (doc->getFields().size() > 1) {
              // back to 1 field
              currentField = field;
              doc->removeFields(L"dummy");
              doc->push_back(currentField);
            }
          }
        }
      } catch (const runtime_error &t) {
        // TODO: really we should pass a random seed to
        // checkAnalysisConsistency then print it here too:
        System::err::println(L"TEST FAIL: useCharFilter=" +
                             StringHelper::toString(useCharFilter) +
                             L" text='" + escape(text) + L"'");
        Rethrow::rethrow(t);
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::closeWhileHandlingException({docs});
  }
}

wstring BaseTokenStreamTestCase::escape(const wstring &s)
{
  int charUpto = 0;
  shared_ptr<StringBuilder> *const sb = make_shared<StringBuilder>();
  while (charUpto < s.length()) {
    constexpr int c = s[charUpto];
    if (c == 0xa) {
      // Strangely, you cannot put \ u000A into Java
      // sources (not in a comment nor a string
      // constant)...:
      sb->append(L"\\n");
    } else if (c == 0xd) {
      // ... nor \ u000D:
      sb->append(L"\\r");
    } else if (c == L'"') {
      sb->append(L"\\\"");
    } else if (c == L'\\') {
      sb->append(L"\\\\");
    } else if (c >= 0x20 && c < 0x80) {
      sb->append(static_cast<wchar_t>(c));
    } else {
      // TODO: we can make ascii easier to read if we
      // don't escape...
      sb->append(wstring::format(Locale::ROOT, L"\\u%04x", c));
    }
    charUpto++;
  }
  return sb->toString();
}

void BaseTokenStreamTestCase::checkAnalysisConsistency(
    shared_ptr<Random> random, shared_ptr<Analyzer> a, bool useCharFilter,
    const wstring &text) 
{
  checkAnalysisConsistency(random, a, useCharFilter, text, true);
}

void BaseTokenStreamTestCase::checkAnalysisConsistency(
    shared_ptr<Random> random, shared_ptr<Analyzer> a, bool useCharFilter,
    const wstring &text, bool graphOffsetsAreCorrect) 
{
  checkAnalysisConsistency(random, a, useCharFilter, text,
                           graphOffsetsAreCorrect, nullptr);
}

void BaseTokenStreamTestCase::checkAnalysisConsistency(
    shared_ptr<Random> random, shared_ptr<Analyzer> a, bool useCharFilter,
    const wstring &text, bool graphOffsetsAreCorrect,
    shared_ptr<Field> field) 
{

  if (VERBOSE) {
    wcout
        << Thread::currentThread().getName()
        << L": NOTE: BaseTokenStreamTestCase: get first token stream now text="
        << text << endl;
  }

  int remainder = random->nextInt(10);
  shared_ptr<Reader> reader = make_shared<StringReader>(text);
  shared_ptr<TokenStream> ts = a->tokenStream(
      L"dummy",
      useCharFilter ? make_shared<MockCharFilter>(reader, remainder) : reader);
  shared_ptr<CharTermAttribute> termAtt =
      ts->getAttribute(CharTermAttribute::typeid);
  shared_ptr<OffsetAttribute> offsetAtt =
      ts->getAttribute(OffsetAttribute::typeid);
  shared_ptr<PositionIncrementAttribute> posIncAtt =
      ts->getAttribute(PositionIncrementAttribute::typeid);
  shared_ptr<PositionLengthAttribute> posLengthAtt =
      ts->getAttribute(PositionLengthAttribute::typeid);
  shared_ptr<TypeAttribute> typeAtt = ts->getAttribute(TypeAttribute::typeid);
  deque<wstring> tokens = deque<wstring>();
  deque<wstring> types = deque<wstring>();
  deque<int> positions = deque<int>();
  deque<int> positionLengths = deque<int>();
  deque<int> startOffsets = deque<int>();
  deque<int> endOffsets = deque<int>();
  ts->reset();

  // First pass: save away "correct" tokens
  while (ts->incrementToken()) {
    assertNotNull(L"has no CharTermAttribute", termAtt);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    tokens.push_back(termAtt->toString());
    if (typeAtt != nullptr) {
      types.push_back(typeAtt->type());
    }
    if (posIncAtt != nullptr) {
      positions.push_back(posIncAtt->getPositionIncrement());
    }
    if (posLengthAtt != nullptr) {
      positionLengths.push_back(posLengthAtt->getPositionLength());
    }
    if (offsetAtt != nullptr) {
      startOffsets.push_back(offsetAtt->startOffset());
      endOffsets.push_back(offsetAtt->endOffset());
    }
  }
  ts->end();
  delete ts;

  // verify reusing is "reproducable" and also get the normal tokenstream sanity
  // checks
  if (!tokens.empty()) {

    // KWTokenizer (for example) can produce a token
    // even when input is length 0:
    if (text.length() != 0) {

      // (Optional) second pass: do something evil:
      constexpr int evilness = random->nextInt(50);
      if (evilness == 17) {
        if (VERBOSE) {
          wcout << Thread::currentThread().getName()
                << L": NOTE: BaseTokenStreamTestCase: re-run analysis w/ "
                   L"exception"
                << endl;
        }
        // Throw an errant exception from the Reader:

        shared_ptr<MockReaderWrapper> evilReader =
            make_shared<MockReaderWrapper>(random,
                                           make_shared<StringReader>(text));
        evilReader->throwExcAfterChar(random->nextInt(text.length() + 1));
        reader = evilReader;

        try {
          // NOTE: some Tokenizers go and read characters
          // when you call .setReader(Reader), eg
          // PatternTokenizer.  This is a bit
          // iffy... (really, they should only
          // pull from the Reader when you call
          // .incremenToken(), I think?), but we
          // currently allow it, so, we must call
          // a.tokenStream inside the try since we may
          // hit the exc on init:
          ts = a->tokenStream(
              L"dummy", useCharFilter
                            ? make_shared<MockCharFilter>(reader, remainder)
                            : reader);
          ts->reset();
          while (ts->incrementToken()) {
            ;
          }
          fail(L"did not hit exception");
        } catch (const runtime_error &re) {
          assertTrue(MockReaderWrapper::isMyEvilException(re));
        }
        try {
          ts->end();
        } catch (const IllegalStateException &ise) {
          // Catch & ignore MockTokenizer's
          // anger...
          if (ise->getMessage()->contains(L"end() called in wrong state=")) {
            // OK
          } else {
            throw ise;
          }
        }
        delete ts;
      } else if (evilness == 7) {
        // Only consume a subset of the tokens:
        constexpr int numTokensToRead = random->nextInt(tokens.size());
        if (VERBOSE) {
          wcout << Thread::currentThread().getName()
                << L": NOTE: BaseTokenStreamTestCase: re-run analysis, only "
                   L"consuming "
                << numTokensToRead << L" of " << tokens.size() << L" tokens"
                << endl;
        }

        reader = make_shared<StringReader>(text);
        ts = a->tokenStream(L"dummy",
                            useCharFilter
                                ? make_shared<MockCharFilter>(reader, remainder)
                                : reader);
        ts->reset();
        for (int tokenCount = 0; tokenCount < numTokensToRead; tokenCount++) {
          assertTrue(ts->incrementToken());
        }
        try {
          ts->end();
        } catch (const IllegalStateException &ise) {
          // Catch & ignore MockTokenizer's
          // anger...
          if (ise->getMessage()->contains(L"end() called in wrong state=")) {
            // OK
          } else {
            throw ise;
          }
        }
        delete ts;
      }
    }
  }

  // Final pass: verify clean tokenization matches
  // results from first pass:

  if (VERBOSE) {
    wcout << Thread::currentThread().getName()
          << L": NOTE: BaseTokenStreamTestCase: re-run analysis; "
          << tokens.size() << L" tokens" << endl;
  }
  reader = make_shared<StringReader>(text);

  int64_t seed = random->nextLong();
  random = make_shared<Random>(seed);
  if (random->nextInt(30) == 7) {
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": NOTE: BaseTokenStreamTestCase: using spoon-feed reader"
            << endl;
    }

    reader = make_shared<MockReaderWrapper>(random, reader);
  }

  ts = a->tokenStream(
      L"dummy",
      useCharFilter ? make_shared<MockCharFilter>(reader, remainder) : reader);
  if (typeAtt != nullptr && posIncAtt != nullptr && posLengthAtt != nullptr &&
      offsetAtt != nullptr) {
    // offset + pos + posLength + type
    assertTokenStreamContents(
        ts, tokens.toArray(std::deque<wstring>(tokens.size())),
        toIntArray(startOffsets), toIntArray(endOffsets),
        types.toArray(std::deque<wstring>(types.size())),
        toIntArray(positions), toIntArray(positionLengths), text.length(),
        graphOffsetsAreCorrect);
  } else if (typeAtt != nullptr && posIncAtt != nullptr &&
             offsetAtt != nullptr) {
    // offset + pos + type
    assertTokenStreamContents(
        ts, tokens.toArray(std::deque<wstring>(tokens.size())),
        toIntArray(startOffsets), toIntArray(endOffsets),
        types.toArray(std::deque<wstring>(types.size())),
        toIntArray(positions), nullptr, text.length(), graphOffsetsAreCorrect);
  } else if (posIncAtt != nullptr && posLengthAtt != nullptr &&
             offsetAtt != nullptr) {
    // offset + pos + posLength
    assertTokenStreamContents(
        ts, tokens.toArray(std::deque<wstring>(tokens.size())),
        toIntArray(startOffsets), toIntArray(endOffsets), nullptr,
        toIntArray(positions), toIntArray(positionLengths), text.length(),
        graphOffsetsAreCorrect);
  } else if (posIncAtt != nullptr && offsetAtt != nullptr) {
    // offset + pos
    assertTokenStreamContents(
        ts, tokens.toArray(std::deque<wstring>(tokens.size())),
        toIntArray(startOffsets), toIntArray(endOffsets), nullptr,
        toIntArray(positions), nullptr, text.length(), graphOffsetsAreCorrect);
  } else if (offsetAtt != nullptr) {
    // offset
    assertTokenStreamContents(
        ts, tokens.toArray(std::deque<wstring>(tokens.size())),
        toIntArray(startOffsets), toIntArray(endOffsets), nullptr, nullptr,
        nullptr, text.length(), graphOffsetsAreCorrect);
  } else {
    // terms only
    assertTokenStreamContents(
        ts, tokens.toArray(std::deque<wstring>(tokens.size())));
  }

  a->normalize(L"dummy", text);
  // TODO: what can we do besides testing that the above method does not throw?

  if (field != nullptr) {
    reader = make_shared<StringReader>(text);
    random = make_shared<Random>(seed);
    if (random->nextInt(30) == 7) {
      if (VERBOSE) {
        wcout << Thread::currentThread().getName()
              << L": NOTE: BaseTokenStreamTestCase: indexing using spoon-feed "
                 L"reader"
              << endl;
      }

      reader = make_shared<MockReaderWrapper>(random, reader);
    }

    field->setReaderValue(useCharFilter
                              ? make_shared<MockCharFilter>(reader, remainder)
                              : reader);
  }
}

wstring
BaseTokenStreamTestCase::toDot(shared_ptr<Analyzer> a,
                               const wstring &inputText) 
{
  shared_ptr<StringWriter> *const sw = make_shared<StringWriter>();
  shared_ptr<TokenStream> *const ts = a->tokenStream(L"field", inputText);
  ts->reset();
  (make_shared<TokenStreamToDot>(inputText, ts, make_shared<PrintWriter>(sw)))
      ->toDot();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return sw->toString();
}

void BaseTokenStreamTestCase::toDotFile(
    shared_ptr<Analyzer> a, const wstring &inputText,
    const wstring &localFileName) 
{
  shared_ptr<Writer> w = Files::newBufferedWriter(Paths->get(localFileName),
                                                  StandardCharsets::UTF_8);
  shared_ptr<TokenStream> *const ts = a->tokenStream(L"field", inputText);
  ts->reset();
  (make_shared<TokenStreamToDot>(inputText, ts, make_shared<PrintWriter>(w)))
      ->toDot();
  w->close();
}

std::deque<int> BaseTokenStreamTestCase::toIntArray(deque<int> &deque)
{
  std::deque<int> ret(deque.size());
  int offset = 0;
  for (shared_ptr<> : : optional<int> i : deque) {
    ret[offset++] = i;
  }
  return ret;
}

shared_ptr<MockTokenizer> BaseTokenStreamTestCase::whitespaceMockTokenizer(
    shared_ptr<Reader> input) 
{
  shared_ptr<MockTokenizer> mockTokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  mockTokenizer->setReader(input);
  return mockTokenizer;
}

shared_ptr<MockTokenizer> BaseTokenStreamTestCase::whitespaceMockTokenizer(
    const wstring &input) 
{
  shared_ptr<MockTokenizer> mockTokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  mockTokenizer->setReader(make_shared<StringReader>(input));
  return mockTokenizer;
}

shared_ptr<MockTokenizer> BaseTokenStreamTestCase::keywordMockTokenizer(
    shared_ptr<Reader> input) 
{
  shared_ptr<MockTokenizer> mockTokenizer =
      make_shared<MockTokenizer>(MockTokenizer::KEYWORD, false);
  mockTokenizer->setReader(input);
  return mockTokenizer;
}

shared_ptr<MockTokenizer> BaseTokenStreamTestCase::keywordMockTokenizer(
    const wstring &input) 
{
  shared_ptr<MockTokenizer> mockTokenizer =
      make_shared<MockTokenizer>(MockTokenizer::KEYWORD, false);
  mockTokenizer->setReader(make_shared<StringReader>(input));
  return mockTokenizer;
}

shared_ptr<AttributeFactory>
BaseTokenStreamTestCase::newAttributeFactory(shared_ptr<Random> random)
{
  switch (random->nextInt(3)) {
  case 0:
    return TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY;
  case 1:
    return Token::TOKEN_ATTRIBUTE_FACTORY;
  case 2:
    return AttributeFactory::DEFAULT_ATTRIBUTE_FACTORY;
  default:
    throw make_shared<AssertionError>(
        L"Please fix the Random.nextInt() call above");
  }
}

shared_ptr<AttributeFactory> BaseTokenStreamTestCase::newAttributeFactory()
{
  return newAttributeFactory(random());
}

wstring BaseTokenStreamTestCase::toString(shared_ptr<Set<wstring>> strings)
{
  deque<wstring> stringsList = deque<wstring>(strings);
  sort(stringsList.begin(), stringsList.end());
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  for (auto s : stringsList) {
    b->append(L"  ");
    b->append(s);
    b->append(L'\n');
  }
  return b->toString();
}

void BaseTokenStreamTestCase::assertGraphStrings(
    shared_ptr<Analyzer> analyzer, const wstring &text,
    deque<wstring> &expectedStrings) 
{
  checkAnalysisConsistency(random(), analyzer, true, text, true);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (TokenStream tokenStream =
  // analyzer.tokenStream("dummy", text))
  {
    TokenStream tokenStream = analyzer->tokenStream(L"dummy", text);
    assertGraphStrings(tokenStream, {expectedStrings});
  }
}

void BaseTokenStreamTestCase::assertGraphStrings(
    shared_ptr<TokenStream> tokenStream,
    deque<wstring> &expectedStrings) 
{
  shared_ptr<Automaton> automaton =
      (make_shared<TokenStreamToAutomaton>())->toAutomaton(tokenStream);
  shared_ptr<Set<std::shared_ptr<IntsRef>>> actualStringPaths =
      AutomatonTestUtil::getFiniteStringsRecursive(automaton, -1);

  shared_ptr<Set<wstring>> expectedStringsSet =
      unordered_set<wstring>(Arrays::asList(expectedStrings));

  shared_ptr<BytesRefBuilder> scratchBytesRefBuilder =
      make_shared<BytesRefBuilder>();
  shared_ptr<Set<wstring>> actualStrings = unordered_set<wstring>();
  for (auto ir : actualStringPaths) {
    actualStrings->add(StringHelper::replace(
        Util::toBytesRef(ir, scratchBytesRefBuilder)->utf8ToString(),
        static_cast<wchar_t>(TokenStreamToAutomaton::POS_SEP), L' '));
  }
  for (auto s : actualStrings) {
    assertTrue(L"Analyzer created unexpected string path: " + s +
                   L"\nexpected:\n" + toString(expectedStringsSet) +
                   L"\nactual:\n" + toString(actualStrings),
               expectedStringsSet->contains(s));
  }
  for (wstring s : expectedStrings) {
    assertTrue(L"Analyzer created unexpected string path: " + s +
                   L"\nexpected:\n" + toString(expectedStringsSet) +
                   L"\nactual:\n" + toString(actualStrings),
               actualStrings->contains(s));
  }
}

shared_ptr<Set<wstring>>
BaseTokenStreamTestCase::getGraphStrings(shared_ptr<Analyzer> analyzer,
                                         const wstring &text) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try(TokenStream tokenStream =
  // analyzer.tokenStream("dummy", text))
  {
    TokenStream tokenStream = analyzer->tokenStream(L"dummy", text);
    return getGraphStrings(tokenStream);
  }
}

shared_ptr<Set<wstring>> BaseTokenStreamTestCase::getGraphStrings(
    shared_ptr<TokenStream> tokenStream) 
{
  shared_ptr<Automaton> automaton =
      (make_shared<TokenStreamToAutomaton>())->toAutomaton(tokenStream);
  shared_ptr<Set<std::shared_ptr<IntsRef>>> actualStringPaths =
      AutomatonTestUtil::getFiniteStringsRecursive(automaton, -1);
  shared_ptr<BytesRefBuilder> scratchBytesRefBuilder =
      make_shared<BytesRefBuilder>();
  shared_ptr<Set<wstring>> paths = unordered_set<wstring>();
  for (auto ir : actualStringPaths) {
    paths->add(StringHelper::replace(
        Util::toBytesRef(ir, scratchBytesRefBuilder)->utf8ToString(),
        static_cast<wchar_t>(TokenStreamToAutomaton::POS_SEP), L' '));
  }
  return paths;
}

wstring
BaseTokenStreamTestCase::toString(shared_ptr<Analyzer> analyzer,
                                  const wstring &text) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try(TokenStream ts =
  // analyzer.tokenStream("field", text))
  {
    TokenStream ts = analyzer->tokenStream(L"field", text);
    shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
    shared_ptr<CharTermAttribute> termAtt =
        ts->getAttribute(CharTermAttribute::typeid);
    shared_ptr<PositionIncrementAttribute> posIncAtt =
        ts->getAttribute(PositionIncrementAttribute::typeid);
    shared_ptr<PositionLengthAttribute> posLengthAtt =
        ts->getAttribute(PositionLengthAttribute::typeid);
    shared_ptr<OffsetAttribute> offsetAtt =
        ts->getAttribute(OffsetAttribute::typeid);
    assertNotNull(offsetAtt);
    ts->reset();
    int pos = -1;
    while (ts->incrementToken()) {
      pos += posIncAtt->getPositionIncrement();
      b->append(termAtt);
      b->append(L" at pos=");
      b->append(pos);
      if (posLengthAtt != nullptr) {
        b->append(L" to pos=");
        b->append(pos + posLengthAtt->getPositionLength());
      }
      b->append(L" offsets=");
      b->append(offsetAtt->startOffset());
      b->append(L'-');
      b->append(offsetAtt->endOffset());
      b->append(L'\n');
    }
    ts->end();
    return b->toString();
  }
}
} // namespace org::apache::lucene::analysis