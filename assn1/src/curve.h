#ifndef CURVE_H
#define CURVE_H

#include "tuple.h"
#include <vecmath.h>
#include <vector>

 //The CurvePoint object stores information about a point on a curve
 //after it has been tesselated: the vertex (V), the tangent (T), the
 //normal (N), and the binormal (B).  It is the responsiblility of
 //functions that create these objects to fill in all the data.
struct CurvePoint
{
    Vector3f V; // Vertex 位置
    Vector3f T; // Tangent  (unit) 切线
    Vector3f N; // Normal   (unit) 法线
    Vector3f B; // Binormal (unit) 副法线
};

// This is just a handy shortcut.
typedef std::vector< CurvePoint > Curve;


////////////////////////////////////////////////////////////////////////////
 //The following two functions take an array of control points (stored
 //in P) and generate an STL Vector of CurvePoints.  They should
 //return an empty array if the number of control points in C is
 //inconsistent with the type of curve.  In both these functions,
 //"step" indicates the number of samples PER PIECE.  E.g., a
 //7-control-point Bezier curve will have two pieces (and the 4th
 //control point is shared).
//以下两个函数获取控制点数组（存储在P中）并生成曲线点的STL向量。
//如果C中的控制点数量与曲线类型不一致，则返回空数组。
//在这两个函数中，“step”表示每件样品的数量。
//例如。。7控制点贝塞尔曲线有两段（第四段
//控制点是共享的）。
////////////////////////////////////////////////////////////////////////////

// Assume number of control points properly specifies a piecewise
// Bezier curve.  I.e., C.size() == 4 + 3*n, n=0,1,...
// 读入一个vector<Vector3f>数据 以及steps 返回一个曲线类 是一个vector<CurvePoint>；CurvePoint包含4个向量 
// 总之就是根据输入的那么多个点的位置 计算每个控制点的各项数据
Curve evalBezier( const std::vector< Vector3f >& P, unsigned steps ); 

// Bsplines only require that there are at least 4 control points.
Curve evalBspline( const std::vector< Vector3f >& P, unsigned steps );

// Create a circle on the xy-plane of radius and steps
Curve evalCircle( float radius, unsigned steps);

class VertexRecorder; // 用于绘制
// Record the curve vertices
void recordCurve( const Curve& curve, VertexRecorder* recorder);
// Record the curve's associated coordinate frames
void recordCurveFrames( const Curve& curve, VertexRecorder* recorder, float framesize);
#endif
