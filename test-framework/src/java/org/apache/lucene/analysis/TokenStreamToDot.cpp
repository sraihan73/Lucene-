using namespace std;

#include "TokenStreamToDot.h"

namespace org::apache::lucene::analysis
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;

TokenStreamToDot::TokenStreamToDot(const wstring &inputText,
                                   shared_ptr<TokenStream> in_,
                                   shared_ptr<PrintWriter> out)
    : in_(in_), termAtt(in_->addAttribute(CharTermAttribute::typeid)),
      posIncAtt(in_->addAttribute(PositionIncrementAttribute::typeid)),
      posLengthAtt(in_->addAttribute(PositionLengthAttribute::typeid)),
      inputText(inputText), out(out)
{
  if (in_->hasAttribute(OffsetAttribute::typeid)) {
    offsetAtt = in_->addAttribute(OffsetAttribute::typeid);
  } else {
    offsetAtt.reset();
  }
}

void TokenStreamToDot::toDot() 
{
  in_->reset();
  writeHeader();

  // TODO: is there some way to tell dot that it should
  // make the "main path" a straight line and have the
  // non-sausage arcs not affect node placement...

  int pos = -1;
  int lastEndPos = -1;
  while (in_->incrementToken()) {
    constexpr bool isFirst = pos == -1;
    int posInc = posIncAtt->getPositionIncrement();
    if (isFirst && posInc == 0) {
      // TODO: hmm are TS's still allowed to do this...?
      System::err::println(L"WARNING: first posInc was 0; correcting to 1");
      posInc = 1;
    }

    if (posInc > 0) {
      // New node:
      pos += posInc;
      // C++ TODO: There is no native C++ equivalent to 'toString':
      writeNode(pos, Integer::toString(pos));
    }

    if (posInc > 1) {
      // Gap!
      writeArc(lastEndPos, pos, L"", L"dotted");
    }

    if (isFirst) {
      writeNode(-1, L"");
      writeArc(-1, pos, L"", L"");
    }

    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring arcLabel = termAtt->toString();
    if (offsetAtt != nullptr) {
      constexpr int startOffset = offsetAtt->startOffset();
      constexpr int endOffset = offsetAtt->endOffset();
      // System.out.println("start=" + startOffset + " end=" + endOffset + "
      // len=" + inputText.length());
      if (inputText != L"") {
        wstring fragment =
            inputText.substr(startOffset, endOffset - startOffset);
        // C++ TODO: There is no native C++ equivalent to 'toString':
        if (fragment == termAtt->toString() == false) {
          arcLabel += L" / " + fragment;
        }
      } else {
        arcLabel +=
            L" / " + to_wstring(startOffset) + L"-" + to_wstring(endOffset);
      }
    }

    writeArc(pos, pos + posLengthAtt->getPositionLength(), arcLabel, L"");
    lastEndPos = pos + posLengthAtt->getPositionLength();
  }

  in_->end();

  if (lastEndPos != -1) {
    // TODO: should we output any final text (from end
    // offsets) on this arc...?
    writeNode(-2, L"");
    writeArc(lastEndPos, -2, L"", L"");
  }

  writeTrailer();
}

void TokenStreamToDot::writeArc(int fromNode, int toNode, const wstring &label,
                                const wstring &style)
{
  out->print(L"  " + to_wstring(fromNode) + L" -> " + to_wstring(toNode) +
             L" [");
  if (label != L"") {
    out->print(L" label=\"" + label + L"\"");
  }
  if (style != L"") {
    out->print(L" style=\"" + style + L"\"");
  }
  out->println(L"]");
}

void TokenStreamToDot::writeNode(int name, const wstring &label)
{
  out->print(L"  " + to_wstring(name));
  if (label != L"") {
    out->print(L" [label=\"" + label + L"\"]");
  } else {
    out->print(L" [shape=point color=white]");
  }
  out->println();
}

const wstring TokenStreamToDot::FONT_NAME = L"Helvetica";

void TokenStreamToDot::writeHeader()
{
  out->println(L"digraph tokens {");
  out->println(L"  graph [ fontsize=30 labelloc=\"t\" label=\"\" splines=true "
               L"overlap=false rankdir = \"LR\" ];");
  out->println(L"  // A2 paper size");
  out->println(L"  size = \"34.4,16.5\";");
  // out.println("  // try to fill paper");
  // out.println("  ratio = fill;");
  out->println(L"  edge [ fontname=\"" + FONT_NAME +
               L"\" fontcolor=\"red\" color=\"#606060\" ]");
  out->println(L"  node [ style=\"filled\" fillcolor=\"#e8e8f0\" "
               L"shape=\"Mrecord\" fontname=\"" +
               FONT_NAME + L"\" ]");
  out->println();
}

void TokenStreamToDot::writeTrailer() { out->println(L"}"); }
} // namespace org::apache::lucene::analysis