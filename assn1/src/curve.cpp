#include "curve.h"
#include "vertexrecorder.h"
#include "calculate.h"
#include <cassert>



namespace
{
	// Approximately equal to.  We don't want to use == because of
	// precision issues with floating point.
	// 判断两个向量是否相等 由于精度原因 不能直接使用==
	inline bool approx(const Vector3f& lhs, const Vector3f& rhs)
	{
		const float eps = 1e-8f;
		return (lhs - rhs).absSquared() < eps;
	}

	const float c_pi = 3.14159265358979323846f;
int* constCoefficients = nullptr; //常数项系数

// 因为矩阵是一列一列构造的 所以现在的写法虽然和矩阵看起来一致 但其实行列互换了 也就是矩阵的转置 要再转置回来
const Matrix4f b_spline_basis = Matrix4f(
	1.0f, 4.0f, 1.0f, 0.0f,
	-3.0f, 0.0f, 3.0f, 0.0f,
	3.0f, -6.0f, 3.0f, 0.0f,
	-1.0f, 3.0f, -3.0f, 1.0f
).transposed() * (1.0f / 6.0f);

const Matrix4f b_spline_inverse = b_spline_basis.inverse();

const Matrix4f bezier_basis = Matrix4f(
	1.0f, -3.0f, 3.0f, -1.0f,
	0.0f, 3.0f, -6.0f, 3.0f,
	0.0f, 0.0f, 3.0f, -3.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);

const Matrix4f bezier_inverse = bezier_basis.inverse();
}

Curve evalBezier(const vector< Vector3f >& P, unsigned steps) //steps应该理解为steps条小线段的意思 所以曲线上的点应该有steps+1个
{
	// Check
	if (P.size() < 4 || P.size() % 3 != 1)
	{
		cerr << "evalBezier must be called with 3n+1 control points." << endl;
		exit(0);
	}

	// TODO:
	// You should implement this function so that it returns a Curve       /*待办事项：
	// (e.g., a vector< CurvePoint >).  The variable "steps" tells you	   您应该实现此函数，以便它返回曲线（例如，返回一个vector<CurvePoint>。
	// the number of points to generate on each piece of the spline.	   变量“steps”告诉您要在每条样条曲线上生成的点数。
	// At least, that's how the sample solution is implemented and how	   （实际上会有step+1个点，因为最后一个点跟起点重合）
	// the SWP files are written.  But you are free to interpret this	   至少，这就是示例解决方案的实现方式和SWP文件的编写方式。
	// variable however you want, so long as you can control the		   但您可以随意解释这个变量，只要您可以用它控制离散样条曲线的“分辨率”。
	// "resolution" of the discretized spline curve with it.			   确保此函数为每个曲线点计算所有适当的矢量3f : V、T、N、B。
	// Make sure that this function computes all the appropriate		   [NBT]应为单位且正交。还请注意，您可能假设接收到的所有贝塞尔曲线都具有
	// Vector3fs for each CurvePoint: V,T,N,B.							   G1连续性。否则，TNB将不会在不适用的点定义。*/
	// [NBT] should be unit and orthogonal.

	// Also note that you may assume that all Bezier curves that you
	// receive have G1 continuity.  Otherwise, the TNB will not be
	// be defined at points where this does not hold.

	int poinsCount = steps + 1; //曲线上的点的个数
	Curve curvePoins(poinsCount);

	int controlPointsCount = int(P.size());
	int* constCoefficients = calculateNthTermExpansion(controlPointsCount -1); // 计算常数项系数的数组 n个控制点 n-1阶 n项系数

	for (unsigned i = 0;i <= steps;i++)
	{
		float t = float(i) / steps; // 0-steps个时刻 相当于映射到0-1的时间 分割为steps+1个时间点(首尾同个点 0时刻和steps时刻是同一个位置）
		Vector3f B(0.0, 0.0, 1.0); // 自定义的起始B
		curvePoins[i].V = calculateV(P, t, constCoefficients);
		curvePoins[i].T = calculateT(P, t, constCoefficients);
		if(i == 0)
			curvePoins[i].N = calculateN(B, curvePoins[i].T);
		else
			curvePoins[i].N = calculateN(curvePoins[i-1].B, curvePoins[i].T);
		curvePoins[i].B = calculateB(curvePoins[i].T, curvePoins[i].N);
	}
	
	delete[] constCoefficients;

	cerr << "\t>>> evalBezier has been called with the following input:" << endl;

	cerr << "\t>>> Steps (type steps): " << steps << endl;

	cerr << "\t>>> Control points (type vector< Vector3f >): " << endl;
	for (int i = 0; i < (int)P.size(); ++i)
	{
		cout << "\t>>> " << P[i].x() << " " << P[i].y() << " " << P[i].z() << endl;
	}

	cerr << "\t>>> Control points' position(Vector3f): " << endl;
	//for (int i = 0; i < (int)curvePoins.size(); ++i)
	//{
	//	cerr << "\t>>> " << curvePoins[i].V.x() << " " << curvePoins[i].V.y() << " " << curvePoins[i].V.z() << endl;
	//}

	if (curvePoins.size()!=0)
		return curvePoins;

	// Right now this will just return this empty curve.
	cerr << "\t>>> Returning empty curve." << endl;
	return Curve();
}

Curve evalBspline(const vector< Vector3f >& P, unsigned steps)
{
	// Check
	if (P.size() < 4)
	{
		cerr << "evalBspline must be called with 4 or more control points." << endl;
		exit(0);
	}

	// TODO:
	// It is suggested that you implement this function by changing
	// basis from B-spline to Bezier.  That way, you can just call
	// your evalBezier function.

	int poinsCount = steps + 1;
	Curve Bspline(poinsCount);

	int piece = 0;
	// Deal with groups of 4 at a time.
	while (piece < P.size() - 3)
	{
		vector<Vector3f> G_vector = vector<Vector3f>(
			P.begin() + piece,
			P.begin() + piece + 4
			);
		Matrix4f G( // 为什么要增加一个维度 可能是因为Matrix4f的乘法 只有维度完全相同的矩阵(4x4)能相乘 不同维度的矩阵(向量)乘不了
			Vector4f(G_vector[0], 0),
			Vector4f(G_vector[1], 0),
			Vector4f(G_vector[2], 0),
			Vector4f(G_vector[3], 0)
		);
		// Convert to the Bernstein basis for bezier curves.
		Matrix4f basis_change_P = G * b_spline_basis * bezier_inverse;
		vector<Vector3f> P_converted
		{
			basis_change_P.getCol(0).xyz(),
			basis_change_P.getCol(1).xyz(),
			basis_change_P.getCol(2).xyz(),
			basis_change_P.getCol(3).xyz()
		};
		Curve portion = evalBezier(P_converted, steps); // 每一小段曲线上的点集

		if (piece == P.size() - 4)
		{
			for (CurvePoint p : portion)
				Bspline.push_back(p);
		}
		else
		{
			for (int i = 0; i < portion.size() - 1; i++)
				Bspline.push_back(portion[i]);
		}
		piece += 3; //这里为啥是+1 不应该是+3吗
	}


	cerr << "\t>>> evalBSpline has been called with the following input:" << endl;

	cerr << "\t>>> Steps (type steps): " << steps << endl;

	cerr << "\t>>> Control points (type vector< Vector3f >): " << endl;
	for (int i = 0; i < (int)P.size(); ++i)
	{
		cout << "\t>>> " << P[i].x() << " " << P[i].y() << " " << P[i].z() << endl;
	}


	if (Bspline.size() != 0)
		return Bspline;

	// Right now this will just return this empty curve.
	cerr << "\t>>> Returning empty curve." << endl;
	cerr << "\t>>> Returning empty curve." << endl;
	return Curve();
}

Curve evalCircle(float radius, unsigned steps)
{
	// This is a sample function on how to properly initialize a Curve
	// (which is a vector< CurvePoint >).

	// Preallocate a curve with steps+1 CurvePoints
	Curve R(steps + 1); // 预分配step+1个空的曲线点

	// Fill it in counterclockwise
	for (unsigned i = 0; i <= steps; ++i)
	{
		// step from 0 to 2pi
		float t = 2.0f * c_pi * float(i) / steps;

		// Initialize position
		// We're pivoting counterclockwise around the y-axis
		R[i].V = radius * Vector3f(cos(t), sin(t), 0);

		// Tangent vector is first derivative
		R[i].T = Vector3f(-sin(t), cos(t), 0);

		// Normal vector is second derivative
		R[i].N = Vector3f(-cos(t), -sin(t), 0);

		// Finally, binormal is facing up.
		R[i].B = Vector3f(0, 0, 1);
	}

	return R;
}

void recordCurve(const Curve& curve, VertexRecorder* recorder)
{
	const Vector3f WHITE(1, 1, 1);
	for (size_t i = 0; i < (int)curve.size() - 1; ++i) // 为什么要连续记录两个点 用于绘制的时候连线？
	{
		recorder->record_poscolor(curve[i].V, WHITE);
		recorder->record_poscolor(curve[i + 1].V, WHITE);
	}
}
void recordCurveFrames(const Curve& curve, VertexRecorder* recorder, float framesize)
{
	Matrix4f T;
	const Vector3f RED(1, 0, 0);
	const Vector3f GREEN(0, 1, 0);
	const Vector3f BLUE(0, 0, 1);
	
	const Vector4f ORGN(0, 0, 0, 1);
	const Vector4f AXISX(framesize, 0, 0, 1);
	const Vector4f AXISY(0, framesize, 0, 1);
	const Vector4f AXISZ(0, 0, framesize, 1);

	for (int i = 0; i < (int)curve.size(); ++i)
	{
		// 设置矩阵的方法 这里用了齐次坐标 所以加了一个维度
		T.setCol(0, Vector4f(curve[i].N, 0));
		T.setCol(1, Vector4f(curve[i].B, 0));
		T.setCol(2, Vector4f(curve[i].T, 0));
		T.setCol(3, Vector4f(curve[i].V, 1));
 
		// Transform orthogonal frames into model space 将正交的坐标轴进行仿射变换？
		Vector4f MORGN  = T * ORGN;
		Vector4f MAXISX = T * AXISX;
		Vector4f MAXISY = T * AXISY;
		Vector4f MAXISZ = T * AXISZ;

		// Record in model space
		recorder->record_poscolor(MORGN.xyz(), RED);
		recorder->record_poscolor(MAXISX.xyz(), RED);

		recorder->record_poscolor(MORGN.xyz(), GREEN);
		recorder->record_poscolor(MAXISY.xyz(), GREEN);

		recorder->record_poscolor(MORGN.xyz(), BLUE);
		recorder->record_poscolor(MAXISZ.xyz(), BLUE);
	}
}