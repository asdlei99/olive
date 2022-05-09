/***

  Olive - Non-Linear Video Editor
  Copyright (C) 2021 Olive Team

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef NODEVALUE_H
#define NODEVALUE_H

#include <QMatrix4x4>
#include <QString>
#include <QVariant>
#include <QVector>

#include "codec/samplebuffer.h"
#include "common/bezier.h"
#include "node/splitvalue.h"
#include "render/color.h"
#include "render/texture.h"
#include "undo/undocommand.h"

namespace olive {

class Node;

class NodeValue
{
public:
  /**
   * @brief The types of data that can be passed between Nodes
   */
  enum Type {
    kNone,

    /**
     ****************************** SPECIFIC IDENTIFIERS ******************************
     */

    /**
     * Integer type
     *
     * Resolves to int64_t.
     */
    kInt,

    /**
     * Decimal (floating-point) type
     *
     * Resolves to `double`.
     */
    kFloat,

    /**
     * Decimal (rational) type
     *
     * Resolves to `double`.
     */
    kRational,

    /**
     * Boolean type
     *
     * Resolves to `bool`.
     */
    kBoolean,

    /**
     * Floating-point type
     *
     * Resolves to `Color`.
     *
     * Colors passed around the nodes should always be in reference space and preferably use
     */
    kColor,

    /**
     * Matrix type
     *
     * Resolves to `QMatrix4x4`.
     */
    kMatrix,

    /**
     * Text type
     *
     * Resolves to `QString`.
     */
    kText,

    /**
     * Font type
     *
     * Resolves to `QFont`.
     */
    kFont,

    /**
     * File type
     *
     * Resolves to a `QString` containing an absolute file path.
     */
    kFile,

    /**
     * Image buffer type
     *
     * True value type depends on the render engine used.
     */
    kTexture,

    /**
     * Audio samples type
     *
     * Resolves to `SampleBufferPtr`.
     */
    kSamples,

    /**
     * Two-dimensional vector (XY) type
     *
     * Resolves to `QVector2D`.
     */
    kVec2,

    /**
     * Three-dimensional vector (XYZ) type
     *
     * Resolves to `QVector3D`.
     */
    kVec3,

    /**
     * Four-dimensional vector (XYZW) type
     *
     * Resolves to `QVector4D`.
     */
    kVec4,

    /**
     * Cubic bezier type that contains three X/Y coordinates, the main point, and two control points
     *
     * Resolves to `Bezier`
     */
    kBezier,

    /**
     * ComboBox type
     *
     * Resolves to `int` - the index currently selected
     */
    kCombo,

    /**
     * Video Parameters type
     *
     * Resolves to `VideoParams`
     */
    kVideoParams,

    /**
     * Audio Parameters type
     *
     * Resolves to `AudioParams`
     */
    kAudioParams,

    /**
     * End of list
     */
    kDataTypeCount
  };

  static const QVector<Type> kNumber;
  static const QVector<Type> kBuffer;
  static const QVector<Type> kVector;

  NodeValue() :
    type_(kNone),
    from_(nullptr),
    array_(false)
  {
  }

  NodeValue(Type type, const QVariant& data, const Node* from = nullptr, bool array = false, const QString& tag = QString()) :
    type_(type),
    data_(data),
    from_(from),
    tag_(tag),
    array_(array)
  {
  }

  Type type() const
  {
    return type_;
  }

  template <typename T>
  T value() const
  {
    return data_.value<T>();
  }

  template <typename T>
  void set_value(const T &v)
  {
    data_ = QVariant::fromValue(v);
  }

  template <typename T>
  bool canConvert() const
  {
    return data_.canConvert<T>();
  }

  const QString& tag() const
  {
    return tag_;
  }

  const Node* source() const
  {
    return from_;
  }

  bool array() const
  {
    return array_;
  }

  bool operator==(const NodeValue& rhs) const
  {
    return type_ == rhs.type_ && tag_ == rhs.tag_ && data_ == rhs.data_;
  }

  static QString GetPrettyDataTypeName(Type type);

  static QString GetDataTypeName(Type type);
  static NodeValue::Type GetDataTypeFromName(const QString &n);

  static QString ValueToString(Type data_type, const QVariant& value, bool value_is_a_key_track);
  static QString ValueToString(const NodeValue &v, bool value_is_a_key_track)
  {
    return ValueToString(v.type_, v.data_, value_is_a_key_track);
  }

  static QVariant StringToValue(Type data_type, const QString &string, bool value_is_a_key_track);

  /**
   * @brief Convert a value from a NodeParam into bytes
   */
  static QByteArray ValueToBytes(Type type, const QVariant& value);
  static QByteArray ValueToBytes(const NodeValue &value)
  {
    return ValueToBytes(value.type(), value.data_);
  }

  static QVector<QVariant> split_normal_value_into_track_values(Type type, const QVariant &value);

  static QVariant combine_track_values_into_normal_value(Type type, const QVector<QVariant>& split);

  SplitValue to_split_value() const
  {
    return split_normal_value_into_track_values(type_, data_);
  }

  /**
   * @brief Returns whether a data type can be interpolated or not
   */
  static bool type_can_be_interpolated(NodeValue::Type type)
  {
    return type == kFloat
        || type == kVec2
        || type == kVec3
        || type == kVec4
        || type == kBezier
        || type == kColor
        || type == kRational;
  }

  static bool type_is_numeric(NodeValue::Type type)
  {
    return kNumber.contains(type);
  }

  static bool type_is_vector(NodeValue::Type type)
  {
    return kVector.contains(type);
  }

  static int get_number_of_keyframe_tracks(Type type);

  static void ValidateVectorString(QStringList* list, int count);

  TexturePtr toTexture() const { return value<TexturePtr>(); }
  SampleBuffer toSamples() const { return value<SampleBuffer>(); }
  bool toBool() const { return value<bool>(); }
  double toDouble() const { return value<double>(); }
  int64_t toInt() const { return value<int64_t>(); }
  rational toRational() const { return value<rational>(); }
  QString toString() const { return value<QString>(); }
  Color toColor() const { return value<Color>(); }
  QMatrix4x4 toMatrix() const { return value<QMatrix4x4>(); }
  VideoParams toVideoParams() const { return value<VideoParams>(); }
  AudioParams toAudioParams() const { return value<AudioParams>(); }
  QVector2D toVec2() const { return value<QVector2D>(); }
  QVector3D toVec3() const { return value<QVector3D>(); }
  QVector4D toVec4() const { return value<QVector4D>(); }
  Bezier toBezier() const { return value<Bezier>(); }

private:
  Type type_;
  QVariant data_;
  const Node* from_;
  QString tag_;
  bool array_;

};

class NodeValueTable
{
public:
  NodeValueTable() = default;

  NodeValue Get(NodeValue::Type type, const QString& tag = QString()) const
  {
    QVector<NodeValue::Type> types = {type};
    return Get(types, tag);
  }

  NodeValue Get(const QVector<NodeValue::Type>& type, const QString& tag = QString()) const;

  NodeValue Take(NodeValue::Type type, const QString& tag = QString())
  {
    QVector<NodeValue::Type> types = {type};
    return Take(types, tag);
  }

  NodeValue Take(const QVector<NodeValue::Type>& type, const QString& tag = QString());

  void Push(const NodeValue& value)
  {
    values_.append(value);
  }

  void Push(const NodeValueTable& value)
  {
    values_.append(value.values_);
  }

  void Push(NodeValue::Type type, const QVariant& data, const Node *from, bool array = false, const QString& tag = QString())
  {
    Push(NodeValue(type, data, from, array, tag));
  }

  void Prepend(const NodeValue& value)
  {
    values_.prepend(value);
  }

  void Prepend(NodeValue::Type type, const QVariant& data, const Node *from, bool array = false, const QString& tag = QString())
  {
    Prepend(NodeValue(type, data, from, array, tag));
  }

  const NodeValue& at(int index) const
  {
    return values_.at(index);
  }
  NodeValue TakeAt(int index)
  {
    return values_.takeAt(index);
  }

  int Count() const
  {
    return values_.size();
  }

  bool Has(NodeValue::Type type) const;
  void Remove(const NodeValue& v);

  void Clear()
  {
    values_.clear();
  }

  bool isEmpty() const
  {
    return values_.isEmpty();
  }

  int GetValueIndex(const QVector<NodeValue::Type> &type, const QString& tag) const;

  static NodeValueTable Merge(QList<NodeValueTable> tables);

private:
  QVector<NodeValue> values_;

};

using NodeValueRow = QHash<QString, NodeValue>;

}

Q_DECLARE_METATYPE(olive::NodeValue)
Q_DECLARE_METATYPE(olive::NodeValueTable)

#endif // NODEVALUE_H
