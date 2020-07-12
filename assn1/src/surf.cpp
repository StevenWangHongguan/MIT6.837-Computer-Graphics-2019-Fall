#include "surf.h"
#include "vertexrecorder.h"
#include "starter1_util.h"

#include "Utils.h"

using namespace std;

vector<Curve> gCurves; // 定义全局变量

Curve calculateRevCurve(const Curve& profile, float revAngle)
{
    // cout << "profile.size()" << profile.size() << endl;
    Curve newCurve(profile.size());

    //cout << "before newCurve.size()" << newCurve.size() << endl;

    float apha = deg2rad(revAngle);
    // 由于只有旋转没有平移 所以不使用齐次坐标了
    Matrix3f revMatrix_Y = Matrix3f(
        cos(apha), 0.0f, sin(apha),
        0.0f, 1.0f, 0.0f,
        -sin(apha), 0.0f, cos(apha)
    ).transposed();

    for (int i = 0;i < profile.size();i++) // 对剖线上的每个点 进行变换 包括V和N
    {
        newCurve[i].V = revMatrix_Y * profile[i].V; // 3x3矩阵和3维向量相乘
        newCurve[i].N = revMatrix_Y * profile[i].N;
        newCurve[i].T = revMatrix_Y * profile[i].T;
        // newCurve[i].B = revMatrix_Y * profile[i].B;
    }

    //cout << "after newCurve.size()" << newCurve.size() << endl;

    return newCurve;
}

namespace
{
    
    // We're only implenting swept surfaces where the profile curve is
    // flat on the xy-plane.  This is a check function.
    static bool checkFlat(const Curve &profile)
    {
        for (unsigned i=0; i<profile.size(); i++) // 检查z分量是否都是0
            if (profile[i].V[2] != 0.0 ||
                profile[i].T[2] != 0.0 ||
                profile[i].N[2] != 0.0)
                return false;
    
        return true;
    }
}

// DEBUG HELPER
Surface quad() { 
	Surface ret;
	ret.VV.push_back(Vector3f(-1, -1, 0));
	ret.VV.push_back(Vector3f(+1, -1, 0));
	ret.VV.push_back(Vector3f(+1, +1, 0));
	ret.VV.push_back(Vector3f(-1, +1, 0));

	ret.VN.push_back(Vector3f(0, 0, 1));
	ret.VN.push_back(Vector3f(0, 0, 1));
	ret.VN.push_back(Vector3f(0, 0, 1));
	ret.VN.push_back(Vector3f(0, 0, 1));

	ret.VF.push_back(Tup3u(0, 1, 2));
	ret.VF.push_back(Tup3u(0, 2, 3));

    // 在这里添加测试 加入重复的节点(不重复的索引编号 看是否显示正常）

	return ret;
}

void constructSuface(Surface& surface, const vector<Curve>& curves)
{
    int leftBottomIndex = 0, rightBottomIndex = 1, rightTopIndex = 2, leftTopIndex = 3; // 四个顶点的索引

   // 每相邻两条曲线 构成一条曲面 从上往下遍历曲线上的点 构成三角形面元 加入surface中
    for (int i = 0; i < curves.size() - 1; i++) // curves.size() - 1
    {
        // 相邻两条曲线
        Curve curve1 = curves[i];
        Curve curve2 = curves[i + 1];
        int curvePoinsCount = int(curve1.size());
        for (int pointIndex = 0; pointIndex < curvePoinsCount - 1; pointIndex++)
        {
            surface.VV.push_back(curve1[pointIndex + 1].V); // 左下角
            surface.VV.push_back(curve2[pointIndex + 1].V); // 右下角
            surface.VV.push_back(curve2[pointIndex].V); // 右上角
            surface.VV.push_back(curve1[pointIndex].V); // 左上角

            surface.VN.push_back(curve1[pointIndex + 1].N); // 左下角
            surface.VN.push_back(curve2[pointIndex + 1].N); // 右下角
            surface.VN.push_back(curve2[pointIndex].N); // 右上角
            surface.VN.push_back(curve1[pointIndex].N); // 左上角

            //surface.VN.push_back(Vector3f(0, 0, 1)); // 左下角
            //surface.VN.push_back(Vector3f(0, 0, 1)); // 右下角
            //surface.VN.push_back(Vector3f(0, 0, 1)); // 右上角
            //surface.VN.push_back(Vector3f(0, 0, 1)); // 左上角

            surface.VF.push_back(Tup3u(leftBottomIndex, rightBottomIndex, rightTopIndex));
            surface.VF.push_back(Tup3u(leftBottomIndex, rightTopIndex, leftTopIndex));

            leftTopIndex += 4;
            leftBottomIndex += 4;
            rightBottomIndex += 4;
            rightTopIndex += 4;
        }

    }
}

// vector<Curve>& curves 作为调试用 后续删除
Surface makeSurfRev(const Curve &profile, unsigned steps) // 曲面是由无数个顶点位置，顶点法线(小区域平面的法线)以及三角形面元组成的
{
    Surface surface;
	// surface = quad();
    
    if (!checkFlat(profile))
    {
        cerr << "surfRev profile curve must be flat on xy plane." << endl;
        exit(0);
    }

    // TODO: Here you should build the surface.  See surf.h for details.

    // steps是细分的片段数 比如steps=90 相当于细分了90个小曲面片段 那么实际生成的曲线数目应该是steps+1 
    // 因为两两构成一个小片段 要形成首尾部相接 最后就得加一个点 其实跟圆点是同一个点

    // cout << "print profile curve:" << endl;
    // PrintCurvePointsV(profile);

    // Curve fixProfile = profile;
    //for (int i = 0; i < profile.size(); i++)
    //{
    //    /*if(fixProfile[i].T.y() < 0)*/
    //    //profile[i].N = -profile[i].N;
    //}
    

    vector<Curve> curves;
    curves.push_back(profile); // 先加入第一条曲线

    float revAngle = 0.0f;
    float stepAngle = 360.0f / (int)steps; // 角度步长
    for (int i = 1;i <= (int)steps;i++) //
    {
        revAngle += stepAngle;
        Curve revCurve = calculateRevCurve(profile, revAngle);
        curves.push_back(revCurve);
    }

    //gCurves = curves;

    //for (auto& curve : curves) // 翻转所有顶点的法线方向
    //{
    //    for (int i = 0;i < curve.size();i++)
    //    {
    //        if( curve[i].V.x()< 0 && curve[i].T.y() < 0 ) 
    //            curve[i].N = -curve[i].N; //
    //    }
    //}
    
    constructSuface(surface, curves);

    cerr << "\t>>> makeSurfRev called (but not implemented).\n\t>>> Returning empty surface." << endl;
 
    return surface;
}

Surface makeGenCyl(const Curve &profile, const Curve &sweep )
{
    Surface surface;
	// surface = quad();

    if (!checkFlat(profile))
    {
        cerr << "genCyl profile curve must be flat on xy plane." << endl;
        exit(0);
    }

    // TODO: Here you should build the surface.  See surf.h for details.

    vector<Curve> curves;
    curves.push_back(profile); // 先加入第一条曲线

    Vector3f profileFirstPointV = profile[0].V;
    for (int i = 0; i < sweep.size(); i++) // 对扫掠曲线上的每一个点 将轮廓线移动到这个点的位置上
    {
        Vector3f positionOffset = sweep[i].V - profileFirstPointV;
        Curve newCurve = profile; // profile的拷贝
        for (int j = 0; j < profile.size(); j++)
        {
            newCurve[j].V += positionOffset; //拷贝轮廓线上的每一个点 都随着扫掠线上的点移动
        }
        curves.push_back(newCurve);
    }

    //gCurves = curves;

    for (auto& curve : curves) // 翻转所有顶点的法线方向
    {
        for (int i = 0;i < curve.size();i++)
        {
            curve[i].N = -curve[i].N;
        }
    }

    constructSuface(surface, curves);

    cerr << "\t>>> makeGenCyl called (but not implemented).\n\t>>> Returning empty surface." <<endl;

    return surface;
}

void recordSurface(const Surface &surface, VertexRecorder* recorder) {
	const Vector3f WIRECOLOR(0.4f, 0.4f, 0.4f);
    for (int i=0; i<(int)surface.VF.size(); i++) // 每个循环记录一个三角形面元的数据
    {
		recorder->record(surface.VV[surface.VF[i][0]], surface.VN[surface.VF[i][0]], WIRECOLOR); // 记录第一个顶点的位置和法线 通过面元的索引获取
		recorder->record(surface.VV[surface.VF[i][1]], surface.VN[surface.VF[i][1]], WIRECOLOR);
		recorder->record(surface.VV[surface.VF[i][2]], surface.VN[surface.VF[i][2]], WIRECOLOR);
    }
}

void recordNormals(const Surface &surface, VertexRecorder* recorder, float len)
{
	const Vector3f NORMALCOLOR(0, 1, 1);
    for (int i=0; i<(int)surface.VV.size(); i++)
    {
		recorder->record_poscolor(surface.VV[i], NORMALCOLOR);
		recorder->record_poscolor(surface.VV[i] + surface.VN[i] * len, NORMALCOLOR); // 法线记录长一点 便于显示？
    }
}

void outputObjFile(ostream &out, const Surface &surface)
{
    
    for (int i=0; i<(int)surface.VV.size(); i++)
        out << "v  "
            << surface.VV[i][0] << " "
            << surface.VV[i][1] << " "
            << surface.VV[i][2] << endl;

    for (int i=0; i<(int)surface.VN.size(); i++)
        out << "vn "
            << surface.VN[i][0] << " "
            << surface.VN[i][1] << " "
            << surface.VN[i][2] << endl;

    out << "vt  0 0 0" << endl;
    
    for (int i=0; i<(int)surface.VF.size(); i++)
    {
        out << "f  ";
        for (unsigned j=0; j<3; j++)
        {
            unsigned a = surface.VF[i][j]+1;
            out << a << "/" << "1" << "/" << a << " ";
        }
        out << endl;
    }
}
