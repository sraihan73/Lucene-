#pragma once
#include "stringhelper.h"

#include <exception>

class IOException : public std::exception
{
  GET_CLASS_NAME(IOException)
private:
  std::string msg;

public:
  IOException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class Error : public std::exception
{
  GET_CLASS_NAME(Error)
private:
  std::string msg;

public:
  Error(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class SAXException : public std::exception
{
  GET_CLASS_NAME(SAXException)
private:
  std::string msg;

public:
  SAXException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class AssertionError : public std::exception
{
  GET_CLASS_NAME(AssertionError)
private:
  std::string msg;

public:
  AssertionError(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class IllegalStateException : public std::exception
{
  GET_CLASS_NAME(IllegalStateException)
private:
  std::string msg;

public:
  IllegalStateException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class UnsupportedOperationException : public std::exception
{
  GET_CLASS_NAME(UnsupportedOperationException)
private:
  std::string msg;

public:
  UnsupportedOperationException(const std::string &message = "") : msg(message)
  {
  }

  virtual const char* what() const noexcept { return msg.data(); }
};

class NumberFormatException : public std::exception
{
  GET_CLASS_NAME(NumberFormatException)
private:
  std::string msg;

public:
  NumberFormatException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class ParseException : public std::exception
{
  GET_CLASS_NAME(ParseException)
private:
  std::string msg;

public:
  ParseException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class PatternSyntaxException : public std::exception
{
  GET_CLASS_NAME(PatternSyntaxException)
private:
  std::string msg;

public:
  PatternSyntaxException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class ServiceConfigurationError : public std::exception
{
  GET_CLASS_NAME(ServiceConfigurationError)
private:
  std::string msg;

public:
  ServiceConfigurationError(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class InvocationTargetException : public std::exception
{
  GET_CLASS_NAME(InvocationTargetException)
private:
  std::string msg;

public:
  InvocationTargetException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class ReflectiveOperationException : public std::exception
{
  GET_CLASS_NAME(ReflectiveOperationException)
private:
  std::string msg;

public:
  ReflectiveOperationException(const std::string &message = "") : msg(message)
  {
  }

  virtual const char* what() const noexcept { return msg.data(); }
};

class CloneNotSupportedException : public std::exception
{
  GET_CLASS_NAME(CloneNotSupportedException)
private:
  std::string msg;

public:
  CloneNotSupportedException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class NullPointerException : public std::exception
{
  GET_CLASS_NAME(NullPointerException)
private:
  std::string msg;

public:
  NullPointerException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class FileNotFoundException : public std::exception
{
  GET_CLASS_NAME(FileNotFoundException)
private:
  std::string msg;

public:
  FileNotFoundException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class EOFException : public std::exception
{
  GET_CLASS_NAME(EOFException)
private:
  std::string msg;

public:
  EOFException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class UnsupportedEncodingException : public std::exception
{
  GET_CLASS_NAME(UnsupportedEncodingException)
private:
  std::string msg;

public:
  UnsupportedEncodingException(const std::string &message = "") : msg(message)
  {
  }

  virtual const char* what() const noexcept { return msg.data(); }
};

class ClassNotFoundException : public std::exception
{
  GET_CLASS_NAME(ClassNotFoundException)
private:
  std::string msg;

public:
  ClassNotFoundException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class NoSuchElementException : public std::exception
{
  GET_CLASS_NAME(NoSuchElementException)
private:
  std::string msg;

public:
  NoSuchElementException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class StringIndexOutOfBoundsException : public std::exception
{
  GET_CLASS_NAME(StringIndexOutOfBoundsException)
private:
  std::string msg;

public:
  StringIndexOutOfBoundsException(const std::string &message = "")
      : msg(message)
  {
  }

  virtual const char* what() const noexcept { return msg.data(); }
};

class CASException : public std::exception
{
  GET_CLASS_NAME(CASException)
private:
  std::string msg;

public:
  CASException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class InterruptedException : public std::exception
{
  GET_CLASS_NAME(InterruptedException)
private:
  std::string msg;

public:
  InterruptedException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class NoSuchMethodException : public std::exception
{
  GET_CLASS_NAME(NoSuchMethodException)
private:
  std::string msg;

public:
  NoSuchMethodException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class CompressorException : public std::exception
{
  GET_CLASS_NAME(CompressorException)
private:
  std::string msg;

public:
  CompressorException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class TimeoutException : public std::exception
{
  GET_CLASS_NAME(TimeoutException)
private:
  std::string msg;

public:
  TimeoutException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class SecurityException : public std::exception
{
  GET_CLASS_NAME(SecurityException)
private:
  std::string msg;

public:
  SecurityException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class DataFormatException : public std::exception
{
  GET_CLASS_NAME(DataFormatException)
private:
  std::string msg;

public:
  DataFormatException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class ArithmeticException : public std::exception
{
  GET_CLASS_NAME(ArithmeticException)
private:
  std::string msg;

public:
  ArithmeticException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class VirtualMachineError : public std::exception
{
  GET_CLASS_NAME(VirtualMachineError)
private:
  std::string msg;

public:
  VirtualMachineError(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class NoSuchFileException : public std::exception
{
  GET_CLASS_NAME(NoSuchFileException)
private:
  std::string msg;

public:
  NoSuchFileException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class CharacterCodingException : public std::exception
{
  GET_CLASS_NAME(CharacterCodingException)
private:
  std::string msg;

public:
  CharacterCodingException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class ExecutionException : public std::exception
{
  GET_CLASS_NAME(ExecutionException)
private:
  std::string msg;

public:
  ExecutionException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class ConcurrentModificationException : public std::exception
{
  GET_CLASS_NAME(ConcurrentModificationException)
private:
  std::string msg;

public:
  ConcurrentModificationException(const std::string &message = "")
      : msg(message)
  {
  }

  virtual const char* what() const noexcept { return msg.data(); }
};

class BufferUnderflowException : public std::exception
{
  GET_CLASS_NAME(BufferUnderflowException)
private:
  std::string msg;

public:
  BufferUnderflowException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class FileAlreadyExistsException : public std::exception
{
  GET_CLASS_NAME(FileAlreadyExistsException)
private:
  std::string msg;

public:
  FileAlreadyExistsException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class ClassCastException : public std::exception
{
  GET_CLASS_NAME(ClassCastException)
private:
  std::string msg;

public:
  ClassCastException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class RejectedExecutionException : public std::exception
{
  GET_CLASS_NAME(RejectedExecutionException)
private:
  std::string msg;

public:
  RejectedExecutionException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class OutOfMemoryError : public std::exception
{
  GET_CLASS_NAME(OutOfMemoryError)
private:
  std::string msg;

public:
  OutOfMemoryError(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class UnknownError : public std::exception
{
  GET_CLASS_NAME(UnknownError)
private:
  std::string msg;

public:
  UnknownError(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class StackOverflowError : public std::exception
{
  GET_CLASS_NAME(StackOverflowError)
private:
  std::string msg;

public:
  StackOverflowError(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class IllegalAccessException : public std::exception
{
  GET_CLASS_NAME(IllegalAccessException)
private:
  std::string msg;

public:
  IllegalAccessException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class RecognitionException : public std::exception
{
  GET_CLASS_NAME(RecognitionException)
private:
  std::string msg;

public:
  RecognitionException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class UnknownHostException : public std::exception
{
  GET_CLASS_NAME(UnknownHostException)
private:
  std::string msg;

public:
  UnknownHostException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class MissingResourceException : public std::exception
{
  GET_CLASS_NAME(MissingResourceException)
private:
  std::string msg;

public:
  MissingResourceException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class ParserConfigurationException : public std::exception
{
  GET_CLASS_NAME(ParserConfigurationException)
private:
  std::string msg;

public:
  ParserConfigurationException(const std::string &message = "") : msg(message)
  {
  }

  virtual const char* what() const noexcept { return msg.data(); }
};

class ServletException : public std::exception
{
  GET_CLASS_NAME(ServletException)
private:
  std::string msg;

public:
  ServletException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class SocketException : public std::exception
{
  GET_CLASS_NAME(SocketException)
private:
  std::string msg;

public:
  SocketException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class InvalidShapeException : public std::exception
{
  GET_CLASS_NAME(InvalidShapeException)
private:
  std::string msg;

public:
  InvalidShapeException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class NoClassDefFoundError : public std::exception
{
  GET_CLASS_NAME(NoClassDefFoundError)
private:
  std::string msg;

public:
  NoClassDefFoundError(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class InstantiationException : public std::exception
{
  GET_CLASS_NAME(InstantiationException)
private:
  std::string msg;

public:
  InstantiationException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class ProviderMismatchException : public std::exception
{
  GET_CLASS_NAME(ProviderMismatchException)
private:
  std::string msg;

public:
  ProviderMismatchException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class InvalidPathException : public std::exception
{
  GET_CLASS_NAME(InvalidPathException)
private:
  std::string msg;

public:
  InvalidPathException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class AccessDeniedException : public std::exception
{
  GET_CLASS_NAME(AccessDeniedException)
private:
  std::string msg;

public:
  AccessDeniedException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class AssertionFailedError : public std::exception
{
  GET_CLASS_NAME(AssertionFailedError)
private:
  std::string msg;

public:
  AssertionFailedError(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class PrivilegedActionException : public std::exception
{
  GET_CLASS_NAME(PrivilegedActionException)
private:
  std::string msg;

public:
  PrivilegedActionException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class AssumptionViolatedException : public std::exception
{
  GET_CLASS_NAME(AssumptionViolatedException)
private:
  std::string msg;

public:
  AssumptionViolatedException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class BuildException : public std::exception
{
  GET_CLASS_NAME(BuildException)
private:
  std::string msg;

public:
  BuildException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};

class NoSuchAlgorithmException : public std::exception
{
  GET_CLASS_NAME(NoSuchAlgorithmException)
private:
  std::string msg;

public:
  NoSuchAlgorithmException(const std::string &message = "") : msg(message) {}

  virtual const char* what() const noexcept { return msg.data(); }
};
