//Complete walkthrough: https://leetarxiv.substack.com/p/semaev-naive-index-calculus
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
//clear && gcc Semaev.c -lm -o m.o && ./m.o

struct Point_point_struct
{
	int x;
	int y;
	int infinity;
};
typedef struct Point_point_struct Point[1];

struct curve_struct
{
	int primeNumber;
	int pointOrder;
	int generatorX;
	int generatorY;
	int primitiveRoot;
	int b;
};

typedef struct curve_struct Curve[1];

void InitializeSmallCurve(Curve curve, int primeNumber, int pointOrder, int generatorX, int generatorY, int primitiveRoot)
{
	assert(primeNumber % 3 == 1);
	curve[0].primeNumber = primeNumber;
	curve[0].pointOrder = pointOrder;
	curve[0].generatorX = generatorX;
	curve[0].generatorY = generatorY;
	curve[0].primitiveRoot = primitiveRoot;
	curve[0].b = 7;
}

void PrintSmallCurve(Curve curve)
{
	printf("Prime number: %4d\n", curve->primeNumber);
	printf("Generator : (%4d, %4d)\n", curve->generatorX, curve->generatorY);
	printf("Point order: %4d\n", curve->pointOrder);
}

void InitializePoint(Point point)
{
	point[0].x = 0;
	point[0].y = 0;
	point[0].infinity = 0;
}

void CopyPoint(Point source, Point destination)
{
	destination->x = source->x;
	destination->y = source->y;
	destination->infinity = source->infinity;
}

void PrintPoint(Point point)
{
	printf("(%d %d)\n", point->x,point->y);
}

//Modular inverse of a mod m
int ModularInverse(int a, int m) 
{
	int t = 0, newT = 1;
	int r = m, newR = a;

	while(newR != 0)
	{
		int quotient = r / newR;
		int tempT = t;
		t = newT;
		newT = tempT - quotient * newT;
		int tempR = r;
		r = newR;
		newR = tempR - quotient * newR;
	}

	if(r > 1) return -1;  
	if(t < 0) t += m;

	return t;
}

bool PointIsOnCurve(Point point, int primeNumber)
{
	if(point->infinity) return true;
	int y2 = (point->y * point->y) % primeNumber;
	int x2 = (point->x * point->x) % primeNumber;
	int x3 = (x2 * point->x) % primeNumber;
	return y2 == (x3 + 7) % primeNumber;
}



bool AddCurvePoints(Point R, Point P, Point Q, int primeNumber)
{
	//Case 0: Handle Points at infinity
	if(P->infinity == 1){CopyPoint(Q,R);return true;}
	if(Q->infinity == 1){CopyPoint(P,R);return true;}
	
	//Case 1: Handle P->x == Q->x
	if(P->x == Q->x) 
	{
		//Case 1.1: X values similar but Y differ or 0
		if(P->y != Q->y || P->y == 0){R->infinity = 1;return true;}
		//Case 1.2: Point Doubling
		int num = P->x * P->x * 3;num = (num + primeNumber) % primeNumber;
		int den = P->y * 2;den = (den + primeNumber) % primeNumber;
		int denominatorInverse = ModularInverse(den, primeNumber);
		if(denominatorInverse == -1)
		{
			R->infinity = 1;
			return true;
		}
		//s = (3x^2)/(2y)
		int s = (num * denominatorInverse) % primeNumber;s = (s + primeNumber) % primeNumber;
		R->x = ((s*s) - 2 * P->x) % primeNumber;
		R->y = (s * (P->x - R->x) - P->y) % primeNumber;
		R->infinity = 0;
	}
	//Case 2: Handle P != Q (Point Addition)
	else
	{
		int num = Q->y - P->y;num = (num + primeNumber) % primeNumber;
		int den = Q->x - P->x;den = (den + primeNumber) % primeNumber;
		int denominatorInverse = ModularInverse(den, primeNumber);
		denominatorInverse = (denominatorInverse + primeNumber) % primeNumber;
		if(denominatorInverse == -1)
		{
			R->infinity = 1;
			return true;
		}
		int s = (num * denominatorInverse) % primeNumber;s = (s + primeNumber) % primeNumber;
		R->x = (s *s - P->x - Q->x) % primeNumber;
		//y3 = s*(x1 - x3) - y1
		R->y = (s *(P->x - R->x) - P->y) % primeNumber;
		R->infinity = 0;	
	}
	R->x = (R->x + primeNumber) % primeNumber;
	R->y = (R->y + primeNumber) % primeNumber;
	return true;
}

void LSB_DoubleAndAdd(Curve curve, Point result, int privateKey)
{
	//Create pointAtInfinity, temp0, temp1
	Point pointAtInfinity;
	InitializePoint(pointAtInfinity);
	pointAtInfinity->infinity = 1;  

	//Save generator to temp0
	Point temp0;
	InitializePoint(temp0);
	temp0->x = curve->generatorX;
	temp0->y = curve->generatorY;

	Point temp1;
	InitializePoint(temp1);
	assert(privateKey > 0);
	size_t binaryLength = floor(log2(privateKey)) + 1;
	for(size_t i = 0; i < binaryLength; ++i)
	{
		int bit = (privateKey >> i) & 1;
		if(bit)
		{
			//temp1 = pointAtInfinity + currentGenerator
			AddCurvePoints(temp1, pointAtInfinity, temp0, curve->primeNumber);
			CopyPoint(temp1, pointAtInfinity);
		}
		//Update temp0(currentGenerator) = 2*temp0(previousGenerator)
		AddCurvePoints(temp1, temp0, temp0, curve->primeNumber);
		CopyPoint(temp1, temp0);	
	}
	CopyPoint(pointAtInfinity, result);
}

void TestDoubleandAdd()
{
	int primeNumber = 20959;
	int pointOrder  = 20947;
	int generatorX  = 17263;
	int generatorY  = 13626;
	int primitiveRoot = 3;
	int b = 7;

	Curve smallCurve;
	InitializeSmallCurve(smallCurve,primeNumber,pointOrder,generatorX,generatorY,primitiveRoot);
	PrintSmallCurve(smallCurve);
	
	Point result;
	for(int privateKey = 1; privateKey < pointOrder; privateKey++)
	{
		LSB_DoubleAndAdd(smallCurve, result, privateKey);
		assert(PointIsOnCurve(result, smallCurve->primeNumber));
		printf("%4d: ",privateKey);PrintPoint(result);
	}
}

int main()
{
	TestDoubleandAdd();
	return 0;
}
