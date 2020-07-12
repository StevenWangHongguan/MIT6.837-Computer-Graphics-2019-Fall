#include "curve.h"
#include "vertexrecorder.h"
#include "calculate.h"
#include <cassert>



namespace
{
	// Approximately equal to.  We don't want to use == because of
	// precision issues with floating point.
	// �ж����������Ƿ���� ���ھ���ԭ�� ����ֱ��ʹ��==
	inline bool approx(const Vector3f& lhs, const Vector3f& rhs)
	{
		const float eps = 1e-8f;
		return (lhs - rhs).absSquared() < eps;
	}

	const float c_pi = 3.14159265358979323846f;
int* constCoefficients = nullptr; //������ϵ��

// ��Ϊ������һ��һ�й���� �������ڵ�д����Ȼ�;�������һ�� ����ʵ���л����� Ҳ���Ǿ����ת�� Ҫ��ת�û���
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

Curve evalBezier(const vector< Vector3f >& P, unsigned steps) //stepsӦ�����Ϊsteps��С�߶ε���˼ ���������ϵĵ�Ӧ����steps+1��
{
	// Check
	if (P.size() < 4 || P.size() % 3 != 1)
	{
		cerr << "evalBezier must be called with 3n+1 control points." << endl;
		exit(0);
	}

	// TODO:
	// You should implement this function so that it returns a Curve       /*�������
	// (e.g., a vector< CurvePoint >).  The variable "steps" tells you	   ��Ӧ��ʵ�ִ˺������Ա����������ߣ����磬����һ��vector<CurvePoint>��
	// the number of points to generate on each piece of the spline.	   ������steps��������Ҫ��ÿ���������������ɵĵ�����
	// At least, that's how the sample solution is implemented and how	   ��ʵ���ϻ���step+1���㣬��Ϊ���һ���������غϣ�
	// the SWP files are written.  But you are free to interpret this	   ���٣������ʾ�����������ʵ�ַ�ʽ��SWP�ļ��ı�д��ʽ��
	// variable however you want, so long as you can control the		   ������������������������ֻҪ����������������ɢ�������ߵġ��ֱ��ʡ���
	// "resolution" of the discretized spline curve with it.			   ȷ���˺���Ϊÿ�����ߵ���������ʵ���ʸ��3f : V��T��N��B��
	// Make sure that this function computes all the appropriate		   [NBT]ӦΪ��λ������������ע�⣬�����ܼ�����յ������б��������߶�����
	// Vector3fs for each CurvePoint: V,T,N,B.							   G1�����ԡ�����TNB�������ڲ����õĵ㶨�塣*/
	// [NBT] should be unit and orthogonal.

	// Also note that you may assume that all Bezier curves that you
	// receive have G1 continuity.  Otherwise, the TNB will not be
	// be defined at points where this does not hold.

	int poinsCount = steps + 1; //�����ϵĵ�ĸ���
	Curve curvePoins(poinsCount);

	int controlPointsCount = int(P.size());
	int* constCoefficients = calculateNthTermExpansion(controlPointsCount -1); // ���㳣����ϵ�������� n�����Ƶ� n-1�� n��ϵ��

	for (unsigned i = 0;i <= steps;i++)
	{
		float t = float(i) / steps; // 0-steps��ʱ�� �൱��ӳ�䵽0-1��ʱ�� �ָ�Ϊsteps+1��ʱ���(��βͬ���� 0ʱ�̺�stepsʱ����ͬһ��λ�ã�
		Vector3f B(0.0, 0.0, 1.0); // �Զ������ʼB
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
		Matrix4f G( // ΪʲôҪ����һ��ά�� ��������ΪMatrix4f�ĳ˷� ֻ��ά����ȫ��ͬ�ľ���(4x4)����� ��ͬά�ȵľ���(����)�˲���
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
		Curve portion = evalBezier(P_converted, steps); // ÿһС�������ϵĵ㼯

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
		piece += 3; //����Ϊɶ��+1 ��Ӧ����+3��
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
	Curve R(steps + 1); // Ԥ����step+1���յ����ߵ�

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
	for (size_t i = 0; i < (int)curve.size() - 1; ++i) // ΪʲôҪ������¼������ ���ڻ��Ƶ�ʱ�����ߣ�
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
		// ���þ���ķ��� ��������������� ���Լ���һ��ά��
		T.setCol(0, Vector4f(curve[i].N, 0));
		T.setCol(1, Vector4f(curve[i].B, 0));
		T.setCol(2, Vector4f(curve[i].T, 0));
		T.setCol(3, Vector4f(curve[i].V, 1));
 
		// Transform orthogonal frames into model space ����������������з���任��
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