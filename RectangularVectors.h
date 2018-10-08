#pragma once
#include "stringhelper.h"
//----------------------------------------------------------------------------------------
//	Copyright © 2007 - 2018 Tangible Software Solutions, Inc.
//	This class can be used by anyone provided that the copyright notice
//remains intact.
//
//	This class includes methods to convert multidimensional arrays to C++
//vectors.
//----------------------------------------------------------------------------------------
class RectangularVectors
{
  GET_CLASS_NAME(RectangularVectors)
public:
  static std::deque<std::deque<short>> ReturnRectangularShortVector(int size1,
                                                                      int size2)
  {
    std::deque<std::deque<short>> newVector(size1);
    for (int vector1 = 0; vector1 < size1; vector1++) {
      newVector[vector1] = std::deque<short>(size2);
    }

    return newVector;
  }

  static std::deque<std::deque<int>> ReturnRectangularIntVector(int size1,
                                                                  int size2)
  {
    std::deque<std::deque<int>> newVector(size1);
    for (int vector1 = 0; vector1 < size1; vector1++) {
      newVector[vector1] = std::deque<int>(size2);
    }

    return newVector;
  }

  static std::deque<std::deque<TangibleTemplonglong>>
  ReturnRectangularTangibleTemplonglongVector(int size1, int size2)
  {
    std::deque<std::deque<TangibleTemplonglong>> newVector(size1);
    for (int vector1 = 0; vector1 < size1; vector1++) {
      newVector[vector1] = std::deque<TangibleTemplonglong>(size2);
    }

    return newVector;
  }

  static std::deque<std::deque<std::deque>>
  ReturnRectangularStdvectorVector(int size1, int size2)
  {
    std::deque<std::deque<std::deque>> newVector(size1);
    for (int vector1 = 0; vector1 < size1; vector1++) {
      newVector[vector1] = std::deque<std::deque>(size2);
    }

    return newVector;
  }

  static std::deque<std::deque<StateList *>>
  ReturnRectangularStateListVector(int size1, int size2)
  {
    std::deque<std::deque<StateList *>> newVector(size1);
    for (int vector1 = 0; vector1 < size1; vector1++) {
      newVector[vector1] = std::deque<StateList *>(size2);
    }

    return newVector;
  }

  static std::deque<std::deque<StateListNode *>>
  ReturnRectangularStateListNodeVector(int size1, int size2)
  {
    std::deque<std::deque<StateListNode *>> newVector(size1);
    for (int vector1 = 0; vector1 < size1; vector1++) {
      newVector[vector1] = std::deque<StateListNode *>(size2);
    }

    return newVector;
  }

  static std::deque<std::deque<std::any>>
  ReturnRectangularStdanyVector(int size1, int size2)
  {
    std::deque<std::deque<std::any>> newVector(size1);
    for (int vector1 = 0; vector1 < size1; vector1++) {
      newVector[vector1] = std::deque<std::any>(size2);
    }

    return newVector;
  }

  static std::deque<std::deque<std::wstring>>
  ReturnRectangularStdwstringVector(int size1, int size2)
  {
    std::deque<std::deque<std::wstring>> newVector(size1);
    for (int vector1 = 0; vector1 < size1; vector1++) {
      newVector[vector1] = std::deque<std::wstring>(size2);
    }

    return newVector;
  }

  static std::deque<std::deque<float>> ReturnRectangularFloatVector(int size1,
                                                                      int size2)
  {
    std::deque<std::deque<float>> newVector(size1);
    for (int vector1 = 0; vector1 < size1; vector1++) {
      newVector[vector1] = std::deque<float>(size2);
    }

    return newVector;
  }
};
