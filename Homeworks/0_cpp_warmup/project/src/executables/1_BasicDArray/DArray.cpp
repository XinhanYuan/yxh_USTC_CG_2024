// implementation of class DArray
#include "DArray.h"

#include<iostream>
using namespace std;

// default constructor
DArray::DArray() {
	Init();
}

// set an array with default values
DArray::DArray(int nSize, double dValue) {
	m_nSize = nSize;
	for (int i = 0; i < nSize; i++)
	{
		m_pData[i] = dValue;
	}
}

DArray::DArray(const DArray& arr) 
	: m_pData(new double[arr.m_nSize]), m_nSize(arr.m_nSize)
{
	for (int i = 0; i < m_nSize; i++)
	{
		m_pData[i] = arr.m_pData[i];
	}
}

// deconstructor
DArray::~DArray() {
	Free();
}

// display the elements of the array
void DArray::Print() const {
	cout << "The size of the array is :" << m_nSize << endl << "element :";
	for (int i = 0; i < m_nSize; i++)
	{
		cout << m_pData[i] << " ";
	}
	cout << endl;
}

// initilize the array
void DArray::Init() {
	m_nSize = 0;
	m_pData = nullptr;
}

// free the array
void DArray::Free() {
	delete[] m_pData;
	m_pData = nullptr;
	m_nSize = 0;
}

// get the size of the array
int DArray::GetSize() const {
	return m_nSize; // you should return a correct value
}

// set the size of the array
void DArray::SetSize(int nSize) {
	if (nSize < m_nSize) 
	{
		double* data = new double[nSize];
		for (int i = 0; i < nSize; i++)
		{
			data[i] = m_pData[i];
		}
		m_pData = data;
	}
	else if (nSize > m_nSize)
	{
		double* data = new double[nSize];
		for (int i = 0; i < nSize; i++)
		{
			if (i < m_nSize)
				data[i] = m_pData[i];
			else
				data[i] = 0;
		}
		m_pData = data;
	}
	m_nSize = nSize;
}

// get an element at an index
const double& DArray::GetAt(int nIndex) const {
	return m_pData[nIndex]; // you should return a correct value
}

// set the value of an element 
void DArray::SetAt(int nIndex, double dValue) {
	m_pData[nIndex] = dValue;
}

// overload operator '[]'
const double& DArray::operator[](int nIndex) const {
	return m_pData[nIndex]; // you should return a correct value
}

// add a new element at the end of the array
void DArray::PushBack(double dValue) {
	double* data = new double[m_nSize + 1];
	for (int i = 0; i < m_nSize; i++)
	{
		data[i] = m_pData[i];
	}
	data[m_nSize] = dValue;
	delete[] m_pData;
	m_pData = data;
	m_nSize++;
}

// delete an element at some index
void DArray::DeleteAt(int nIndex) {
	double* data = new double[m_nSize - 1];

	for (int i = 0; i < nIndex; i++)
		data[i] = m_pData[i];

	for (int i = nIndex; i < m_nSize - 1; i++)
		data[i] = m_pData[i + 1];

	delete[] m_pData;
	m_pData = data;
	m_nSize = m_nSize - 1;
}

// insert a new element at some index
void DArray::InsertAt(int nIndex, double dValue) {
	double* data = new double[m_nSize + 1];

	for (int i = 0; i < nIndex; i++)
		data[i] = m_pData[i];

	data[nIndex] = dValue;

	for (int i = nIndex + 1; i < m_nSize + 1; i++)
		data[i] = m_pData[i - 1];

	delete[] m_pData;
	m_pData = data;
	m_nSize = m_nSize + 1;
}

// overload operator '='
DArray& DArray::operator = (const DArray& arr) {
	delete[] m_pData;
	m_nSize = arr.m_nSize;
	m_pData = new double[m_nSize];
	for (int i = 0; i < m_nSize; i++)
	{
		m_pData[i] = arr[i];
	}
	return *this;
}
