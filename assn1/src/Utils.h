#include<iostream>
#include<vector>
#include<stack>
#include<queue>
#include<vecmath.h>
using namespace std;


struct ListNode // 需要的时候 再写一个构造链表的方法
{
	int data;
	ListNode* next;
};

ListNode* ConstructListByArray(int* data, int length)
{
	if (data == nullptr || length <= 0)
		return nullptr;

	ListNode* phead = new ListNode();
	phead->data = data[0];
	phead->next = nullptr;
	ListNode* tail = phead;

	for (int i = 1; i < length; i++)
	{
		ListNode* newNode = new ListNode();
		newNode->next = nullptr;
		newNode->data = data[i];
		tail->next = newNode;
		tail = tail->next;
	}

	return phead;
}

struct BinaryTreeNode
{
	double data;
	BinaryTreeNode* left;
	BinaryTreeNode* right;
};

void PrintList(ListNode* phead)
{
	while (phead != nullptr)
	{
		cout << phead->data << " ";
		phead = phead->next;
	}
	cout << endl;
}

template<typename T>
void Print(const T& value)
{
	cout << "value: " << value << endl;
}

void PrintArray(int array[], int length)
{
	cout << "PrinArray:" << endl;
	for (int i = 0; i < length; i++)
	{
		cout << array[i] << " ";
	}
	cout << endl;
}

//测试单个输入数据和单个结果输出
template<typename T>
void PrintResultByInput(T function(T parameter))
{
	cout << "input: " << endl;
	T input;
	while (cin >> input) //ctrl+z或者ctrl+d结束
	{	
		cout << "result: " << endl;
		cout << function(input) << endl;
		cout << endl;
		cout << "input: " << endl;
	}
}

void printVec3(const Vector3f& vec3)
{
	cout << vec3.x() << "," << vec3.y() << "," << vec3.z() << endl;
}

void PrintCurvePointsV(const Curve& curve)
{
	cout << "print one Curve's V:" << endl;
	for (int i = 0; i < curve.size(); i++)
	{
		cout << "curve[" << i << "].V: ";
		printVec3(curve[i].V);
	}
}

void PrintCurvePointsN(const Curve& curve)
{
	cout << "print one Curve's N:" << endl;
	for (int i = 0; i < curve.size(); i++)
	{
		cout << "curve[" << i << "].N: ";
		printVec3(curve[i].N);
	}
}