#include "surf.h"
#include "vertexrecorder.h"
#include "starter1_util.h"

#include "Utils.h"

using namespace std;

vector<Curve> gCurves; // ����ȫ�ֱ���

Curve calculateRevCurve(const Curve& profile, float revAngle)
{
    // cout << "profile.size()" << profile.size() << endl;
    Curve newCurve(profile.size());

    //cout << "before newCurve.size()" << newCurve.size() << endl;

    float apha = deg2rad(revAngle);
    // ����ֻ����תû��ƽ�� ���Բ�ʹ�����������
    Matrix3f revMatrix_Y = Matrix3f(
        cos(apha), 0.0f, sin(apha),
        0.0f, 1.0f, 0.0f,
        -sin(apha), 0.0f, cos(apha)
    ).transposed();

    for (int i = 0;i < profile.size();i++) // �������ϵ�ÿ���� ���б任 ����V��N
    {
        newCurve[i].V = revMatrix_Y * profile[i].V; // 3x3�����3ά�������
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
        for (unsigned i=0; i<profile.size(); i++) // ���z�����Ƿ���0
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

    // ��������Ӳ��� �����ظ��Ľڵ�(���ظ���������� ���Ƿ���ʾ������

	return ret;
}

void constructSuface(Surface& surface, const vector<Curve>& curves)
{
    int leftBottomIndex = 0, rightBottomIndex = 1, rightTopIndex = 2, leftTopIndex = 3; // �ĸ����������

   // ÿ������������ ����һ������ �������±��������ϵĵ� ������������Ԫ ����surface��
    for (int i = 0; i < curves.size() - 1; i++) // curves.size() - 1
    {
        // ������������
        Curve curve1 = curves[i];
        Curve curve2 = curves[i + 1];
        int curvePoinsCount = int(curve1.size());
        for (int pointIndex = 0; pointIndex < curvePoinsCount - 1; pointIndex++)
        {
            surface.VV.push_back(curve1[pointIndex + 1].V); // ���½�
            surface.VV.push_back(curve2[pointIndex + 1].V); // ���½�
            surface.VV.push_back(curve2[pointIndex].V); // ���Ͻ�
            surface.VV.push_back(curve1[pointIndex].V); // ���Ͻ�

            surface.VN.push_back(curve1[pointIndex + 1].N); // ���½�
            surface.VN.push_back(curve2[pointIndex + 1].N); // ���½�
            surface.VN.push_back(curve2[pointIndex].N); // ���Ͻ�
            surface.VN.push_back(curve1[pointIndex].N); // ���Ͻ�

            //surface.VN.push_back(Vector3f(0, 0, 1)); // ���½�
            //surface.VN.push_back(Vector3f(0, 0, 1)); // ���½�
            //surface.VN.push_back(Vector3f(0, 0, 1)); // ���Ͻ�
            //surface.VN.push_back(Vector3f(0, 0, 1)); // ���Ͻ�

            surface.VF.push_back(Tup3u(leftBottomIndex, rightBottomIndex, rightTopIndex));
            surface.VF.push_back(Tup3u(leftBottomIndex, rightTopIndex, leftTopIndex));

            leftTopIndex += 4;
            leftBottomIndex += 4;
            rightBottomIndex += 4;
            rightTopIndex += 4;
        }

    }
}

// vector<Curve>& curves ��Ϊ������ ����ɾ��
Surface makeSurfRev(const Curve &profile, unsigned steps) // ������������������λ�ã����㷨��(С����ƽ��ķ���)�Լ���������Ԫ��ɵ�
{
    Surface surface;
	// surface = quad();
    
    if (!checkFlat(profile))
    {
        cerr << "surfRev profile curve must be flat on xy plane." << endl;
        exit(0);
    }

    // TODO: Here you should build the surface.  See surf.h for details.

    // steps��ϸ�ֵ�Ƭ���� ����steps=90 �൱��ϸ����90��С����Ƭ�� ��ôʵ�����ɵ�������ĿӦ����steps+1 
    // ��Ϊ��������һ��СƬ�� Ҫ�γ���β����� ���͵ü�һ���� ��ʵ��Բ����ͬһ����

    // cout << "print profile curve:" << endl;
    // PrintCurvePointsV(profile);

    // Curve fixProfile = profile;
    //for (int i = 0; i < profile.size(); i++)
    //{
    //    /*if(fixProfile[i].T.y() < 0)*/
    //    //profile[i].N = -profile[i].N;
    //}
    

    vector<Curve> curves;
    curves.push_back(profile); // �ȼ����һ������

    float revAngle = 0.0f;
    float stepAngle = 360.0f / (int)steps; // �ǶȲ���
    for (int i = 1;i <= (int)steps;i++) //
    {
        revAngle += stepAngle;
        Curve revCurve = calculateRevCurve(profile, revAngle);
        curves.push_back(revCurve);
    }

    //gCurves = curves;

    //for (auto& curve : curves) // ��ת���ж���ķ��߷���
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
    curves.push_back(profile); // �ȼ����һ������

    Vector3f profileFirstPointV = profile[0].V;
    for (int i = 0; i < sweep.size(); i++) // ��ɨ�������ϵ�ÿһ���� ���������ƶ���������λ����
    {
        Vector3f positionOffset = sweep[i].V - profileFirstPointV;
        Curve newCurve = profile; // profile�Ŀ���
        for (int j = 0; j < profile.size(); j++)
        {
            newCurve[j].V += positionOffset; //�����������ϵ�ÿһ���� ������ɨ�����ϵĵ��ƶ�
        }
        curves.push_back(newCurve);
    }

    //gCurves = curves;

    for (auto& curve : curves) // ��ת���ж���ķ��߷���
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
    for (int i=0; i<(int)surface.VF.size(); i++) // ÿ��ѭ����¼һ����������Ԫ������
    {
		recorder->record(surface.VV[surface.VF[i][0]], surface.VN[surface.VF[i][0]], WIRECOLOR); // ��¼��һ�������λ�úͷ��� ͨ����Ԫ��������ȡ
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
		recorder->record_poscolor(surface.VV[i] + surface.VN[i] * len, NORMALCOLOR); // ���߼�¼��һ�� ������ʾ��
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
