using namespace std;

#include "QueryParserTokenManager.h"

namespace org::apache::lucene::queryparser::surround::parser
{

int QueryParserTokenManager::jjStopStringLiteralDfa_1(int pos,
                                                      int64_t active0)
{
  switch (pos) {
  default:
    return -1;
  }
}

int QueryParserTokenManager::jjStartNfa_1(int pos, int64_t active0)
{
  return jjMoveNfa_1(jjStopStringLiteralDfa_1(pos, active0), pos + 1);
}

int QueryParserTokenManager::jjStopAtPos(int pos, int kind)
{
  jjmatchedKind = kind;
  jjmatchedPos = pos;
  return pos + 1;
}

int QueryParserTokenManager::jjMoveStringLiteralDfa0_1()
{
  switch (curChar) {
  case 40:
    return jjStopAtPos(0, 13);
  case 41:
    return jjStopAtPos(0, 14);
  case 44:
    return jjStopAtPos(0, 15);
  case 58:
    return jjStopAtPos(0, 16);
  case 94:
    return jjStopAtPos(0, 17);
  default:
    return jjMoveNfa_1(0, 0);
  }
}

std::deque<int64_t> const QueryParserTokenManager::jjbitVec0 = {
    0xfffffffffffffffeLL, 0xffffffffffffffffLL, 0xffffffffffffffffLL,
    0xffffffffffffffffLL};
std::deque<int64_t> const QueryParserTokenManager::jjbitVec2 = {
    0x0LL, 0x0LL, 0xffffffffffffffffLL, 0xffffffffffffffffLL};

int QueryParserTokenManager::jjMoveNfa_1(int startState, int curPos)
{
  int startsAt = 0;
  jjnewStateCnt = 38;
  int i = 1;
  jjstateSet[0] = startState;
  int kind = 0x7fffffff;
  for (;;) {
    if (++jjround == 0x7fffffff) {
      ReInitRounds();
    }
    if (curChar < 64) {
      int64_t l = 1LL << curChar;
      do {
        switch (jjstateSet[--i]) {
        case 0:
          if ((0x7bffe8faffffd9ffLL & l) != 0LL) {
            if (kind > 22) {
              kind = 22;
            }
            jjCheckNAddStates(0, 4);
          } else if ((0x100002600LL & l) != 0LL) {
            if (kind > 7) {
              kind = 7;
            }
          } else if (curChar == 34) {
            jjCheckNAddStates(5, 7);
          }
          if ((0x3fc000000000000LL & l) != 0LL) {
            jjCheckNAddStates(8, 11);
          } else if (curChar == 49) {
            jjCheckNAddTwoStates(20, 21);
          }
          break;
        case 19:
          if ((0x3fc000000000000LL & l) != 0LL) {
            jjCheckNAddStates(8, 11);
          }
          break;
        case 20:
          if ((0x3ff000000000000LL & l) != 0LL) {
            jjCheckNAdd(17);
          }
          break;
        case 21:
          if ((0x3ff000000000000LL & l) != 0LL) {
            jjCheckNAdd(18);
          }
          break;
        case 22:
          if (curChar == 49) {
            jjCheckNAddTwoStates(20, 21);
          }
          break;
        case 23:
          if (curChar == 34) {
            jjCheckNAddStates(5, 7);
          }
          break;
        case 24:
          if ((0xfffffffbffffffffLL & l) != 0LL) {
            jjCheckNAddTwoStates(24, 25);
          }
          break;
        case 25:
          if (curChar == 34) {
            jjstateSet[jjnewStateCnt++] = 26;
          }
          break;
        case 26:
          if (curChar == 42 && kind > 18) {
            kind = 18;
          }
          break;
        case 27:
          if ((0xfffffffbffffffffLL & l) != 0LL) {
            jjCheckNAddStates(12, 14);
          }
          break;
        case 29:
          if (curChar == 34) {
            jjCheckNAddStates(12, 14);
          }
          break;
        case 30:
          if (curChar == 34 && kind > 19) {
            kind = 19;
          }
          break;
        case 31:
          if ((0x7bffe8faffffd9ffLL & l) == 0LL) {
            break;
          }
          if (kind > 22) {
            kind = 22;
          }
          jjCheckNAddStates(0, 4);
          break;
        case 32:
          if ((0x7bffe8faffffd9ffLL & l) != 0LL) {
            jjCheckNAddTwoStates(32, 33);
          }
          break;
        case 33:
          if (curChar == 42 && kind > 20) {
            kind = 20;
          }
          break;
        case 34:
          if ((0x7bffe8faffffd9ffLL & l) != 0LL) {
            jjCheckNAddTwoStates(34, 35);
          }
          break;
        case 35:
          if ((0x8000040000000000LL & l) == 0LL) {
            break;
          }
          if (kind > 21) {
            kind = 21;
          }
          jjCheckNAddTwoStates(35, 36);
          break;
        case 36:
          if ((0xfbffecfaffffd9ffLL & l) == 0LL) {
            break;
          }
          if (kind > 21) {
            kind = 21;
          }
          jjCheckNAdd(36);
          break;
        case 37:
          if ((0x7bffe8faffffd9ffLL & l) == 0LL) {
            break;
          }
          if (kind > 22) {
            kind = 22;
          }
          jjCheckNAdd(37);
          break;
        default:
          break;
        }
      } while (i != startsAt);
    } else if (curChar < 128) {
      int64_t l = 1LL << (curChar & 077);
      do {
        switch (jjstateSet[--i]) {
        case 0:
          if ((0xffffffffbfffffffLL & l) != 0LL) {
            if (kind > 22) {
              kind = 22;
            }
            jjCheckNAddStates(0, 4);
          }
          if ((0x400000004000LL & l) != 0LL) {
            if (kind > 12) {
              kind = 12;
            }
          } else if ((0x80000000800000LL & l) != 0LL) {
            if (kind > 11) {
              kind = 11;
            }
          } else if (curChar == 97) {
            jjstateSet[jjnewStateCnt++] = 9;
          } else if (curChar == 65) {
            jjstateSet[jjnewStateCnt++] = 6;
          } else if (curChar == 111) {
            jjstateSet[jjnewStateCnt++] = 3;
          } else if (curChar == 79) {
            jjstateSet[jjnewStateCnt++] = 1;
          }
          if (curChar == 110) {
            jjstateSet[jjnewStateCnt++] = 15;
          } else if (curChar == 78) {
            jjstateSet[jjnewStateCnt++] = 12;
          }
          break;
        case 1:
          if (curChar == 82 && kind > 8) {
            kind = 8;
          }
          break;
        case 2:
          if (curChar == 79) {
            jjstateSet[jjnewStateCnt++] = 1;
          }
          break;
        case 3:
          if (curChar == 114 && kind > 8) {
            kind = 8;
          }
          break;
        case 4:
          if (curChar == 111) {
            jjstateSet[jjnewStateCnt++] = 3;
          }
          break;
        case 5:
          if (curChar == 68 && kind > 9) {
            kind = 9;
          }
          break;
        case 6:
          if (curChar == 78) {
            jjstateSet[jjnewStateCnt++] = 5;
          }
          break;
        case 7:
          if (curChar == 65) {
            jjstateSet[jjnewStateCnt++] = 6;
          }
          break;
        case 8:
          if (curChar == 100 && kind > 9) {
            kind = 9;
          }
          break;
        case 9:
          if (curChar == 110) {
            jjstateSet[jjnewStateCnt++] = 8;
          }
          break;
        case 10:
          if (curChar == 97) {
            jjstateSet[jjnewStateCnt++] = 9;
          }
          break;
        case 11:
          if (curChar == 84 && kind > 10) {
            kind = 10;
          }
          break;
        case 12:
          if (curChar == 79) {
            jjstateSet[jjnewStateCnt++] = 11;
          }
          break;
        case 13:
          if (curChar == 78) {
            jjstateSet[jjnewStateCnt++] = 12;
          }
          break;
        case 14:
          if (curChar == 116 && kind > 10) {
            kind = 10;
          }
          break;
        case 15:
          if (curChar == 111) {
            jjstateSet[jjnewStateCnt++] = 14;
          }
          break;
        case 16:
          if (curChar == 110) {
            jjstateSet[jjnewStateCnt++] = 15;
          }
          break;
        case 17:
          if ((0x80000000800000LL & l) != 0LL && kind > 11) {
            kind = 11;
          }
          break;
        case 18:
          if ((0x400000004000LL & l) != 0LL && kind > 12) {
            kind = 12;
          }
          break;
        case 24:
          jjAddStates(15, 16);
          break;
        case 27:
          if ((0xffffffffefffffffLL & l) != 0LL) {
            jjCheckNAddStates(12, 14);
          }
          break;
        case 28:
          if (curChar == 92) {
            jjstateSet[jjnewStateCnt++] = 29;
          }
          break;
        case 29:
          if (curChar == 92) {
            jjCheckNAddStates(12, 14);
          }
          break;
        case 31:
          if ((0xffffffffbfffffffLL & l) == 0LL) {
            break;
          }
          if (kind > 22) {
            kind = 22;
          }
          jjCheckNAddStates(0, 4);
          break;
        case 32:
          if ((0xffffffffbfffffffLL & l) != 0LL) {
            jjCheckNAddTwoStates(32, 33);
          }
          break;
        case 34:
          if ((0xffffffffbfffffffLL & l) != 0LL) {
            jjCheckNAddTwoStates(34, 35);
          }
          break;
        case 36:
          if ((0xffffffffbfffffffLL & l) == 0LL) {
            break;
          }
          if (kind > 21) {
            kind = 21;
          }
          jjstateSet[jjnewStateCnt++] = 36;
          break;
        case 37:
          if ((0xffffffffbfffffffLL & l) == 0LL) {
            break;
          }
          if (kind > 22) {
            kind = 22;
          }
          jjCheckNAdd(37);
          break;
        default:
          break;
        }
      } while (i != startsAt);
    } else {
      int hiByte = static_cast<int>(curChar >> 8);
      int i1 = hiByte >> 6;
      int64_t l1 = 1LL << (hiByte & 077);
      int i2 = (curChar & 0xff) >> 6;
      int64_t l2 = 1LL << (curChar & 077);
      do {
        switch (jjstateSet[--i]) {
        case 0:
          if (!jjCanMove_0(hiByte, i1, i2, l1, l2)) {
            break;
          }
          if (kind > 22) {
            kind = 22;
          }
          jjCheckNAddStates(0, 4);
          break;
        case 24:
          if (jjCanMove_0(hiByte, i1, i2, l1, l2)) {
            jjAddStates(15, 16);
          }
          break;
        case 27:
          if (jjCanMove_0(hiByte, i1, i2, l1, l2)) {
            jjAddStates(12, 14);
          }
          break;
        case 32:
          if (jjCanMove_0(hiByte, i1, i2, l1, l2)) {
            jjCheckNAddTwoStates(32, 33);
          }
          break;
        case 34:
          if (jjCanMove_0(hiByte, i1, i2, l1, l2)) {
            jjCheckNAddTwoStates(34, 35);
          }
          break;
        case 36:
          if (!jjCanMove_0(hiByte, i1, i2, l1, l2)) {
            break;
          }
          if (kind > 21) {
            kind = 21;
          }
          jjstateSet[jjnewStateCnt++] = 36;
          break;
        case 37:
          if (!jjCanMove_0(hiByte, i1, i2, l1, l2)) {
            break;
          }
          if (kind > 22) {
            kind = 22;
          }
          jjCheckNAdd(37);
          break;
        default:
          break;
        }
      } while (i != startsAt);
    }
    if (kind != 0x7fffffff) {
      jjmatchedKind = kind;
      jjmatchedPos = curPos;
      kind = 0x7fffffff;
    }
    ++curPos;
    if ((i = jjnewStateCnt) == (startsAt = 38 - (jjnewStateCnt = startsAt))) {
      return curPos;
    }
    try {
      curChar = input_stream->readChar();
    } catch (const IOException &e) {
      return curPos;
    }
  }
}

int QueryParserTokenManager::jjMoveStringLiteralDfa0_0()
{
  return jjMoveNfa_0(0, 0);
}

int QueryParserTokenManager::jjMoveNfa_0(int startState, int curPos)
{
  int startsAt = 0;
  jjnewStateCnt = 3;
  int i = 1;
  jjstateSet[0] = startState;
  int kind = 0x7fffffff;
  for (;;) {
    if (++jjround == 0x7fffffff) {
      ReInitRounds();
    }
    if (curChar < 64) {
      int64_t l = 1LL << curChar;
      do {
        switch (jjstateSet[--i]) {
        case 0:
          if ((0x3ff000000000000LL & l) == 0LL) {
            break;
          }
          if (kind > 23) {
            kind = 23;
          }
          jjAddStates(17, 18);
          break;
        case 1:
          if (curChar == 46) {
            jjCheckNAdd(2);
          }
          break;
        case 2:
          if ((0x3ff000000000000LL & l) == 0LL) {
            break;
          }
          if (kind > 23) {
            kind = 23;
          }
          jjCheckNAdd(2);
          break;
        default:
          break;
        }
      } while (i != startsAt);
    } else if (curChar < 128) {
      int64_t l = 1LL << (curChar & 077);
      do {
        switch (jjstateSet[--i]) {
        default:
          break;
        }
      } while (i != startsAt);
    } else {
      int hiByte = static_cast<int>(curChar >> 8);
      int i1 = hiByte >> 6;
      int64_t l1 = 1LL << (hiByte & 077);
      int i2 = (curChar & 0xff) >> 6;
      int64_t l2 = 1LL << (curChar & 077);
      do {
        switch (jjstateSet[--i]) {
        default:
          break;
        }
      } while (i != startsAt);
    }
    if (kind != 0x7fffffff) {
      jjmatchedKind = kind;
      jjmatchedPos = curPos;
      kind = 0x7fffffff;
    }
    ++curPos;
    if ((i = jjnewStateCnt) == (startsAt = 3 - (jjnewStateCnt = startsAt))) {
      return curPos;
    }
    try {
      curChar = input_stream->readChar();
    } catch (const IOException &e) {
      return curPos;
    }
  }
}

std::deque<int> const QueryParserTokenManager::jjnextStates = {
    32, 33, 34, 35, 37, 24, 27, 28, 20, 17, 21, 18, 27, 28, 30, 24, 25, 0, 1};

bool QueryParserTokenManager::jjCanMove_0(int hiByte, int i1, int i2,
                                          int64_t l1, int64_t l2)
{
  switch (hiByte) {
  case 0:
    return ((jjbitVec2[i2] & l2) != 0LL);
  default:
    if ((jjbitVec0[i1] & l1) != 0LL) {
      return true;
    }
    return false;
  }
}

std::deque<wstring> const QueryParserTokenManager::jjstrLiteralImages = {
    L"",     nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, L"\50",  L"\51",  L"\54",
    L"\72",  L"\136", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
std::deque<wstring> const QueryParserTokenManager::lexStateNames = {
    L"Boost", L"DEFAULT"};
std::deque<int> const QueryParserTokenManager::jjnewLexState = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, 0,  -1, -1, -1, -1, -1, 1};
std::deque<int64_t> const QueryParserTokenManager::jjtoToken = {0xffff01LL};
std::deque<int64_t> const QueryParserTokenManager::jjtoSkip = {0x80LL};

QueryParserTokenManager::QueryParserTokenManager(shared_ptr<CharStream> stream)
{
  input_stream = stream;
}

QueryParserTokenManager::QueryParserTokenManager(shared_ptr<CharStream> stream,
                                                 int lexState)
    : QueryParserTokenManager(stream)
{
  SwitchTo(lexState);
}

void QueryParserTokenManager::ReInit(shared_ptr<CharStream> stream)
{
  jjmatchedPos = jjnewStateCnt = 0;
  curLexState = defaultLexState;
  input_stream = stream;
  ReInitRounds();
}

void QueryParserTokenManager::ReInitRounds()
{
  int i;
  jjround = 0x80000001;
  for (i = 38; i-- > 0;) {
    jjrounds[i] = 0x80000000;
  }
}

void QueryParserTokenManager::ReInit(shared_ptr<CharStream> stream,
                                     int lexState)
{
  ReInit(stream);
  SwitchTo(lexState);
}

void QueryParserTokenManager::SwitchTo(int lexState)
{
  if (lexState >= 2 || lexState < 0) {
    throw make_shared<TokenMgrError>(
        L"Error: Ignoring invalid lexical state : " + to_wstring(lexState) +
            L". State unchanged.",
        TokenMgrError::INVALID_LEXICAL_STATE);
  } else {
    curLexState = lexState;
  }
}

shared_ptr<Token> QueryParserTokenManager::jjFillToken()
{
  shared_ptr<Token> *const t;
  const wstring curTokenImage;
  constexpr int beginLine;
  constexpr int endLine;
  constexpr int beginColumn;
  constexpr int endColumn;
  wstring im = jjstrLiteralImages[jjmatchedKind];
  curTokenImage = (im == L"") ? input_stream->GetImage() : im;
  beginLine = input_stream->getBeginLine();
  beginColumn = input_stream->getBeginColumn();
  endLine = input_stream->getEndLine();
  endColumn = input_stream->getEndColumn();
  t = Token::newToken(jjmatchedKind, curTokenImage);

  t->beginLine = beginLine;
  t->endLine = endLine;
  t->beginColumn = beginColumn;
  t->endColumn = endColumn;

  return t;
}

shared_ptr<Token> QueryParserTokenManager::getNextToken()
{
  shared_ptr<Token> matchedToken;
  int curPos = 0;

  for (;;) {
    try {
      curChar = input_stream->BeginToken();
    } catch (const IOException &e) {
      jjmatchedKind = 0;
      matchedToken = jjFillToken();
      return matchedToken;
    }

    switch (curLexState) {
    case 0:
      jjmatchedKind = 0x7fffffff;
      jjmatchedPos = 0;
      curPos = jjMoveStringLiteralDfa0_0();
      break;
    case 1:
      jjmatchedKind = 0x7fffffff;
      jjmatchedPos = 0;
      curPos = jjMoveStringLiteralDfa0_1();
      break;
    }
    if (jjmatchedKind != 0x7fffffff) {
      if (jjmatchedPos + 1 < curPos) {
        input_stream->backup(curPos - jjmatchedPos - 1);
      }
      if ((jjtoToken[jjmatchedKind >> 6] & (1LL << (jjmatchedKind & 077))) !=
          0LL) {
        matchedToken = jjFillToken();
        if (jjnewLexState[jjmatchedKind] != -1) {
          curLexState = jjnewLexState[jjmatchedKind];
        }
        return matchedToken;
      } else {
        if (jjnewLexState[jjmatchedKind] != -1) {
          curLexState = jjnewLexState[jjmatchedKind];
        }
        goto EOFLoopContinue;
      }
    }
    int error_line = input_stream->getEndLine();
    int error_column = input_stream->getEndColumn();
    wstring error_after = L"";
    bool EOFSeen = false;
    try {
      input_stream->readChar();
      input_stream->backup(1);
    } catch (const IOException &e1) {
      EOFSeen = true;
      error_after = curPos <= 1 ? L"" : input_stream->GetImage();
      if (curChar == L'\n' || curChar == L'\r') {
        error_line++;
        error_column = 0;
      } else {
        error_column++;
      }
    }
    if (!EOFSeen) {
      input_stream->backup(1);
      error_after = curPos <= 1 ? L"" : input_stream->GetImage();
    }
    throw make_shared<TokenMgrError>(EOFSeen, curLexState, error_line,
                                     error_column, error_after, curChar,
                                     TokenMgrError::LEXICAL_ERROR);
  EOFLoopContinue:;
  }
EOFLoopBreak:;
}

void QueryParserTokenManager::jjCheckNAdd(int state)
{
  if (jjrounds[state] != jjround) {
    jjstateSet[jjnewStateCnt++] = state;
    jjrounds[state] = jjround;
  }
}

void QueryParserTokenManager::jjAddStates(int start, int end)
{
  do {
    jjstateSet[jjnewStateCnt++] = jjnextStates[start];
  } while (start++ != end);
}

void QueryParserTokenManager::jjCheckNAddTwoStates(int state1, int state2)
{
  jjCheckNAdd(state1);
  jjCheckNAdd(state2);
}

void QueryParserTokenManager::jjCheckNAddStates(int start, int end)
{
  do {
    jjCheckNAdd(jjnextStates[start]);
  } while (start++ != end);
}
} // namespace org::apache::lucene::queryparser::surround::parser