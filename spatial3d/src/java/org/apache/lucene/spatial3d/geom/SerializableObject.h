#pragma once
#include "stringhelper.h"
#include <memory>

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace org::apache::lucene::spatial3d::geom
{

/**
 * Indicates that a geo3d object can be serialized and deserialized.
 *
 * @lucene.experimental
 */
class SerializableObject
{
  GET_CLASS_NAME(SerializableObject)

  /** Serialize to output stream.
   * @param outputStream is the output stream to write to.
   */
public:
  virtual void write(std::shared_ptr<OutputStream> outputStream) = 0;

  /** Write a PlanetObject to a stream.
   * @param outputStream is the output stream.
   * @param object is the object to write.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      public static void writePlanetObject(final java.io.OutputStream
  //      outputStream, final PlanetObject object) throws java.io.IOException
  //  {
  //    object.getPlanetModel().write(outputStream);
  //    writeObject(outputStream, object);
  //  }

  /** Read a PlanetObject from a stream.
   * @param inputStream is the input stream.
   * @return the PlanetObject.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      public static PlanetObject readPlanetObject(final java.io.InputStream
  //      inputStream) throws java.io.IOException
  //  {
  //    final PlanetModel pm = new PlanetModel(inputStream);
  //    final SerializableObject so = readObject(pm, inputStream);
  //    if (!(so instanceof PlanetObject))
  //    {
  //      throw new IOException("Type of object is not expected PlanetObject:
  //      "+so.getClassName());
  //    }
  //    return (PlanetObject)so;
  //  }

  /** Write an object to a stream.
   * @param outputStream is the output stream.
   * @param object is the object to write.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      public static void writeObject(final java.io.OutputStream
  //      outputStream, final SerializableObject object) throws
  //      java.io.IOException
  //  {
  //    writeClass(outputStream, object.getClass());
  //    object.write(outputStream);
  //  }

  /** Read an object from a stream (for objects that need a PlanetModel).
   * @param planetModel is the planet model to use to deserialize the object.
   * @param inputStream is the input stream.
   * @return the deserialized object.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      public static SerializableObject readObject(final PlanetModel
  //      planetModel, final java.io.InputStream inputStream) throws
  //      java.io.IOException
  //  {
  //    try
  //    {
  //      // Read the class
  //      final Class clazz = readClass(inputStream);
  //      return readObject(planetModel, inputStream, clazz);
  //    }
  //    catch (ClassNotFoundException e)
  //    {
  //      throw new IOException("Can't find class for deserialization:
  //      "+e.getMessage(), e);
  //    }
  //  }

  /** Read an object from a stream (for objects that do not need a PlanetModel).
   * @param inputStream is the input stream.
   * @return the deserialized object.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      public static SerializableObject readObject(final java.io.InputStream
  //      inputStream) throws java.io.IOException
  //  {
  //    try
  //    {
  //      // read the class
  //      final Class clazz = readClass(inputStream);
  //      return readObject(inputStream, clazz);
  //    }
  //    catch (ClassNotFoundException e)
  //    {
  //      throw new IOException("Can't find class for deserialization:
  //      "+e.getMessage(), e);
  //    }
  //  }

  /** Instantiate a serializable object from a stream.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   * @param clazz is the class to instantiate.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static SerializableObject readObject(final PlanetModel planetModel,
  //      final java.io.InputStream inputStream, final Class clazz) throws
  //      java.io.IOException
  //  {
  //    try
  //    {
  //      // Look for the right constructor
  //      final Constructor<?> c =
  //      clazz.getDeclaredConstructor(PlanetModel.class, InputStream.class);
  //      // Invoke it
  //      final Object object = c.newInstance(planetModel, inputStream);
  //      // check whether caste will work
  //      if (!(object instanceof SerializableObject))
  //      {
  //        throw new IOException("Object "+clazz.getName()+" does not implement
  //        SerializableObject");
  //      }
  //      return (SerializableObject)object;
  //    }
  //    catch (InstantiationException e)
  //    {
  //      throw new IOException("Instantiation exception for class
  //      "+clazz.getName()+": "+e.getMessage(), e);
  //    }
  //    catch (IllegalAccessException e)
  //    {
  //      throw new IOException("Illegal access creating class
  //      "+clazz.getName()+": "+e.getMessage(), e);
  //    }
  //    catch (NoSuchMethodException e)
  //    {
  //      throw new IOException("No such method exception for class
  //      "+clazz.getName()+": "+e.getMessage(), e);
  //    }
  //    catch (InvocationTargetException e)
  //    {
  //      throw new IOException("Exception instantiating class
  //      "+clazz.getName()+": "+e.getMessage(), e);
  //    }
  //
  //  }

  /** Instantiate a serializable object from a stream without a planet model.
   * @param inputStream is the input stream.
   * @param clazz is the class to instantiate.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static SerializableObject readObject(final java.io.InputStream
  //      inputStream, final Class clazz) throws java.io.IOException
  //  {
  //    try
  //    {
  //      // Look for the right constructor
  //      final Constructor<?> c =
  //      clazz.getDeclaredConstructor(InputStream.class);
  //      // Invoke it
  //      final Object object = c.newInstance(inputStream);
  //      // check whether caste will work
  //      if (!(object instanceof SerializableObject))
  //      {
  //        throw new IOException("Object "+clazz.getName()+" does not implement
  //        SerializableObject");
  //      }
  //      return (SerializableObject)object;
  //    }
  //    catch (InstantiationException e)
  //    {
  //      throw new IOException("Instantiation exception for class
  //      "+clazz.getName()+": "+e.getMessage(), e);
  //    }
  //    catch (IllegalAccessException e)
  //    {
  //      throw new IOException("Illegal access creating class
  //      "+clazz.getName()+": "+e.getMessage(), e);
  //    }
  //    catch (NoSuchMethodException e)
  //    {
  //      throw new IOException("No such method exception for class
  //      "+clazz.getName()+": "+e.getMessage(), e);
  //    }
  //    catch (InvocationTargetException e)
  //    {
  //      throw new IOException("Exception instantiating class
  //      "+clazz.getName()+": "+e.getMessage(), e);
  //    }
  //
  //  }

  /** Write a class to a stream.
   * @param outputStream is the output stream.
   * @param clazz is the class to write.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writeClass(final java.io.OutputStream outputStream, final
  //      Class clazz) throws java.io.IOException
  //  {
  //    std::optional<int> index = StandardObjects.classRegsitry.get(clazz);
  //    if (index == nullptr)
  //    {
  //      writeBoolean(outputStream, false);
  //      writeString(outputStream, clazz.getName());
  //    }
  //    else
  //    {
  //      writeBoolean(outputStream, true);
  //      outputStream.write(index);
  //    }
  //  }

  /**
   * Read the class from the stream
   * @param inputStream is the stream to read from.
   * @return is the class read
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static Class readClass(final java.io.InputStream inputStream) throws
  //      java.io.IOException, ClassNotFoundException
  //  {
  //    bool standard = readBoolean(inputStream);
  //    if (standard)
  //    {
  //      int index = inputStream.read();
  //      return StandardObjects.codeRegsitry.get(index);
  //    }
  //    else
  //    {
  //      std::wstring className = readString(inputStream);
  //      return Class.forName(className);
  //    }
  //  }

  /** Write a string to a stream.
   * @param outputStream is the output stream.
   * @param value is the string to write.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writeString(final java.io.OutputStream outputStream, final
  //      std::wstring value) throws java.io.IOException
  //  {
  //    writeByteArray(outputStream, value.getBytes(StandardCharsets.UTF_8));
  //  }

  /** Read a string from a stream.
   * @param inputStream is the stream to read from.
   * @return the string that was read.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static std::wstring readString(final java.io.InputStream inputStream) throws
  //      java.io.IOException
  //  {
  //    return new std::wstring(readByteArray(inputStream), StandardCharsets.UTF_8);
  //  }

  /** Write a point array.
   * @param outputStream is the output stream.
   * @param values is the array of points to write.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writePointArray(final java.io.OutputStream outputStream,
  //      final GeoPoint[] values) throws java.io.IOException
  //  {
  //    writeHomogeneousArray(outputStream, values);
  //  }

  /** Write a point array.
   * @param outputStream is the output stream.
   * @param values is the deque of points to write.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writePointArray(final java.io.OutputStream outputStream,
  //      final java.util.List<GeoPoint> values) throws java.io.IOException
  //  {
  //    writeHomogeneousArray(outputStream, values);
  //  }

  /** Read a point array.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   * @return the array of points that was read.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static GeoPoint[] readPointArray(final PlanetModel planetModel, final
  //      java.io.InputStream inputStream) throws java.io.IOException
  //  {
  //    return readHomogeneousArray(planetModel, inputStream, GeoPoint.class);
  //  }

  /** Write a polgon array.
   * @param outputStream is the output stream.
   * @param values is the array of points to write.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writePolygonArray(final java.io.OutputStream outputStream,
  //      final GeoPolygon[] values) throws java.io.IOException
  //  {
  //    writeHeterogeneousArray(outputStream, values);
  //  }

  /** Write a polygon array.
   * @param outputStream is the output stream.
   * @param values is the deque of points to write.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writePolygonArray(final java.io.OutputStream outputStream,
  //      final java.util.List<GeoPolygon> values) throws java.io.IOException
  //  {
  //    writeHeterogeneousArray(outputStream, values);
  //  }

  /** Read a polygon array.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   * @return the array of polygons that was read.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static GeoPolygon[] readPolygonArray(final PlanetModel planetModel,
  //      final java.io.InputStream inputStream) throws java.io.IOException
  //  {
  //    return readHeterogeneousArray(planetModel, inputStream,
  //    GeoPolygon.class);
  //  }

  /** Write an array.
   * @param outputStream is the output stream,.
   * @param values is the array.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writeHomogeneousArray(final java.io.OutputStream
  //      outputStream, final SerializableObject[] values) throws
  //      java.io.IOException
  //  {
  //    if (values == nullptr)
  //    {
  //      writeInt(outputStream, 0);
  //    }
  //    else
  //    {
  //      writeInt(outputStream, values.length);
  //      for (final SerializableObject value : values)
  //      {
  //        value.write(outputStream);
  //      }
  //    }
  //  }

  /** Write an array.
   * @param outputStream is the output stream,.
   * @param values is the array.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writeHomogeneousArray(final java.io.OutputStream
  //      outputStream, final java.util.List<? extends SerializableObject>
  //      values) throws java.io.IOException
  //  {
  //    if (values == nullptr)
  //    {
  //      writeInt(outputStream, 0);
  //    }
  //    else
  //    {
  //      writeInt(outputStream, values.size());
  //      for (final SerializableObject value : values)
  //      {
  //        value.write(outputStream);
  //      }
  //    }
  //  }

  /** Read an array.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   * @param clazz is the class of the objects to read.
   * @return the array.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static <T extends SerializableObject> T[] readHomogeneousArray(final
  //      PlanetModel planetModel, final java.io.InputStream inputStream, final
  //      Class<T> clazz) throws java.io.IOException
  //  {
  //    final int count = readInt(inputStream);
  //    @@SuppressWarnings("unchecked") final T[] rval =
  //    (T[])Array.newInstance(clazz, count); for (int i = 0; i < count; i++)
  //    {
  //      rval[i] = clazz.cast(readObject(planetModel, inputStream, clazz));
  //    }
  //    return rval;
  //  }

  /** Write an array.
   * @param outputStream is the output stream,.
   * @param values is the array.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writeHeterogeneousArray(final java.io.OutputStream
  //      outputStream, final SerializableObject[] values) throws
  //      java.io.IOException
  //  {
  //    if (values == nullptr)
  //    {
  //      writeInt(outputStream, 0);
  //    }
  //    else
  //    {
  //      writeInt(outputStream, values.length);
  //      for (final SerializableObject value : values)
  //      {
  //        writeObject(outputStream, value);
  //      }
  //    }
  //  }

  /** Write an array.
   * @param outputStream is the output stream,.
   * @param values is the array.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writeHeterogeneousArray(final java.io.OutputStream
  //      outputStream, final java.util.List<? extends SerializableObject>
  //      values) throws java.io.IOException
  //  {
  //    if (values == nullptr)
  //    {
  //      writeInt(outputStream, 0);
  //    }
  //    else
  //    {
  //      writeInt(outputStream, values.size());
  //      for (final SerializableObject value : values)
  //      {
  //        writeObject(outputStream, value);
  //      }
  //    }
  //  }

  /** Read an array.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   * @return the array.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static <T extends SerializableObject> T[] readHeterogeneousArray(final
  //      PlanetModel planetModel, final java.io.InputStream inputStream, final
  //      Class<T> clazz) throws java.io.IOException
  //  {
  //    final int count = readInt(inputStream);
  //    @@SuppressWarnings("unchecked") final T[] rval =
  //    (T[])Array.newInstance(clazz, count); for (int i = 0; i < count; i++)
  //    {
  //      rval[i] = clazz.cast(readObject(planetModel, inputStream));
  //    }
  //    return rval;
  //  }

  /** Write a bitset to a stream.
   * @param outputStream is the output stream.
   * @param bitSet is the bit set to write.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writeBitSet(final java.io.OutputStream outputStream, final
  //      java.util.BitSet bitSet) throws java.io.IOException
  //  {
  //    writeByteArray(outputStream, bitSet.toByteArray());
  //  }

  /** Read a bitset from a stream.
   * @param inputStream is the input stream.
   * @return the bitset read from the stream.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static java.util.BitSet readBitSet(final java.io.InputStream
  //      inputStream) throws java.io.IOException
  //  {
  //    return BitSet.valueOf(readByteArray(inputStream));
  //  }

  /** Write byte array.
   * @param outputStream is the output stream.
   * @param bytes is the byte array.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writeByteArray(final java.io.OutputStream outputStream,
  //      final byte[] bytes) throws java.io.IOException
  //  {
  //    writeInt(outputStream, bytes.length);
  //    outputStream.write(bytes);
  //  }

  /** Read byte array.
   * @param inputStream is the input stream.
   * @return the byte array.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static byte[] readByteArray(final java.io.InputStream inputStream)
  //      throws java.io.IOException
  //  {
  //    int stringLength = readInt(inputStream);
  //    int stringOffset = 0;
  //    final byte[] bytes = new byte[stringLength];
  //    while (stringLength > 0)
  //    {
  //      final int amt = inputStream.read(bytes, stringOffset, stringLength);
  //      if (amt == -1)
  //      {
  //        throw new IOException("Unexpected end of input stream");
  //      }
  //      stringOffset += amt;
  //      stringLength -= amt;
  //    }
  //    return bytes;
  //  }

  /** Write a double to a stream.
   * @param outputStream is the output stream.
   * @param value is the value to write.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writeDouble(final java.io.OutputStream outputStream, final
  //      double value) throws java.io.IOException
  //  {
  //    writeLong(outputStream, Double.doubleToLongBits(value));
  //  }

  /** Read a double from a stream.
   * @param inputStream is the input stream.
   * @return the double value read from the stream.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static double readDouble(final java.io.InputStream inputStream) throws
  //      java.io.IOException
  //  {
  //    return Double.longBitsToDouble(readLong(inputStream));
  //  }

  /** Write a long to a stream.
   * @param outputStream is the output stream.
   * @param value is the value to write.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writeLong(final java.io.OutputStream outputStream, final
  //      long value) throws java.io.IOException
  //  {
  //    writeInt(outputStream, (int)value);
  //    writeInt(outputStream, (int)(value >> 32));
  //  }

  /** Read a long from a stream.
   * @param inputStream is the input stream.
   * @return the long value read from the stream.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static long readLong(final java.io.InputStream inputStream) throws
  //      java.io.IOException
  //  {
  //    final long lower = ((long)(readInt(inputStream))) & 0x00000000ffffffffL;
  //    final long upper = (((long)(readInt(inputStream))) << 32) &
  //    0xffffffff00000000L; return lower + upper;
  //  }

  /** Write an int to a stream.
   * @param outputStream is the output stream.
   * @param value is the value to write.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writeInt(final java.io.OutputStream outputStream, final
  //      int value) throws java.io.IOException
  //  {
  //    outputStream.write(value);
  //    outputStream.write(value >> 8);
  //    outputStream.write(value >> 16);
  //    outputStream.write(value >> 24);
  //  }

  /** Read an int from a stream.
   * @param inputStream is the input stream.
   * @return the value read from the stream.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static int readInt(final java.io.InputStream inputStream) throws
  //      java.io.IOException
  //  {
  //    final int l1 = (inputStream.read()) & 0x000000ff;
  //    final int l2 = (inputStream.read() << 8) & 0x0000ff00;
  //    final int l3 = (inputStream.read() << 16) & 0x00ff0000;
  //    final int l4 = (inputStream.read() << 24) & 0xff000000;
  //    return l1 + l2 + l3 + l4;
  //  }

  /** Write a bool to a stream.
   * @param outputStream is the output stream.
   * @param value is the value to write.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static void writeBoolean(final java.io.OutputStream outputStream,
  //      final bool value) throws java.io.IOException
  //  {
  //    outputStream.write(value?1:0);
  //  }

  /** Read a bool from a stream.
   * @param inputStream is the input stream.
   * @return the bool value.
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static bool readBoolean(final java.io.InputStream inputStream)
  //      throws java.io.IOException
  //  {
  //    final int valueRead = inputStream.read();
  //    if (valueRead == -1)
  //    {
  //      throw new IOException("Unexpected end of input stream");
  //    }
  //    return (valueRead == 0)?false:true;
  //  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
