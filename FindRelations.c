//Complete walkthrough: https://leetarxiv.substack.com/p/semaev-naive-index-calculus
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
//clear && gcc FindRelations.c -lm -o m.o && ./m.o

struct elliptic_point_struct
{
	int x;
	int y;
	int infinity;
};
typedef struct elliptic_point_struct Point[1];

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
	printf("(%d %d) ", point->x,point->y);
}

void PrintRelation(int a, Point P, int b, Point Q, Point factorBasePoint)
{
	printf("%d*(%d,%d) + %d*(%d,%d) +  (%d,%d) == Infinity\n", a, P->x, P->y, b, Q->x, Q->y, factorBasePoint->x, factorBasePoint->y);
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

void LSB_DoubleAndAdd(Curve curve, Point result, Point generator, int privateKey)
{
	//Create pointAtInfinity, temp0, temp1
	Point pointAtInfinity;
	InitializePoint(pointAtInfinity);
	pointAtInfinity->infinity = 1;  

	//Save generator to temp0
	Point temp0;
	InitializePoint(temp0);
	CopyPoint(generator, temp0);

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
	Point generator;
	InitializePoint(generator);
	generator->x = generatorX;
	generator->y = generatorY;
	Point result;
	for(int privateKey = 1; privateKey < pointOrder; privateKey++)
	{
		LSB_DoubleAndAdd(smallCurve, result, generator,privateKey);
		assert(PointIsOnCurve(result, smallCurve->primeNumber));
		printf("%4d: ",privateKey);PrintPoint(result);
	}
}

int Semaev1(int x1)
{
	return x1;
}

//(x1, x2) = x1 - x2  
int Semaev2(int x1, int x2)
{
	return x1 - x2;
}

// Third Semaev polynomial for y^2 = x^3 + Ax + B
int Semaev3(int x1, int x2, int x3, int A, int B)
{
	int term1 = (x1 - x2) * (x1 - x2) * x3 * x3;
	int term2 = -2 * ((x1 + x2) * (x1*x2 + A) + 2*B) * x3;
	int term3 = (x1*x2 - A) * (x1*x2 - A) - 4*B*(x1 + x2);
	return term1 + term2 + term3;
}

int Semaev3_mod(int x1, int x2, int x3, int A, int B, int p)
{
	int t1 = ((x1 - x2 + p) % p);
	t1 = ((int64_t)t1 * t1) % p;
	t1 = ((int64_t)t1 * x3) % p;
	t1 = ((int64_t)t1 * x3) % p;

	int sum_x1x2 = (x1 + x2) % p;
	int prod_x1x2 = (x1 * x2) % p;
	int inner = (prod_x1x2 + A) % p;
	int t2 = ((int64_t)sum_x1x2 * inner) % p;
	t2 = (t2 + 2 * B) % p;
	t2 = ((int64_t)t2 * x3) % p;
	t2 = (2 * t2) % p;

	int diff = ((prod_x1x2 - A + p) % p);
	int t3 = ((int64_t)diff * diff) % p;
	int t3b = ((int64_t)4 * B % p * sum_x1x2) % p;
	t3 = (t3 - t3b + p) % p;

	// Final polynomial result
	int result = (t1 - t2 + t3 + p + p) % p;  
	return result;
}


void TestBuildFactor()
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
	Point generator;
	InitializePoint(generator);
	generator->x = generatorX;
	generator->y = generatorY;
	
	//Find all points on the curve
	Point *allCurvePoints = malloc(pointOrder * sizeof(Point));
	allCurvePoints[0]->x = generatorX;
	allCurvePoints[0]->y = generatorY;
	allCurvePoints[0]->infinity = 0;
	Point result;
	for(int privateKey = 1; privateKey < pointOrder; privateKey++)
	{
		LSB_DoubleAndAdd(smallCurve, result, generator, privateKey);
		assert(PointIsOnCurve(result, smallCurve->primeNumber));
		CopyPoint(result, allCurvePoints[privateKey]);
	}
	
	//Find a factor base
	printf("Factor Base:\n");
	int factorBaseSize = 5;
	Point *factorBase = malloc(factorBaseSize * sizeof(Point));
	for(int i = 0; i < factorBaseSize; i++)
	{
		int index = rand() % pointOrder;
		CopyPoint(allCurvePoints[index], factorBase[i]);
		printf(" | %d:", index);PrintPoint(factorBase[i]);
	}
	
	printf("\n\nCollecting relations:\n");
	//Initialize target points
	Point target, P, Q;
	InitializePoint(target);
	
	int targetIndex = rand() % pointOrder;
	printf("Target : %d\n",targetIndex);
	CopyPoint(allCurvePoints[targetIndex], target);
	
	for(int i = 0; i < pointOrder*2; i++)
	{
		int randomA = rand() % pointOrder;if(randomA == 0){randomA += 1;}
		int randomB = rand() % pointOrder;if(randomB == 0){randomB += 1;}
		//Find aP
		LSB_DoubleAndAdd(smallCurve, P, generator, randomA);
		//Find bQ
		LSB_DoubleAndAdd(smallCurve, Q, target, randomB);
		
		//Pass x coordinates to Semaev3 with a factor base point
		for(int j = 0; j < factorBaseSize; j++)
		{
			int semaevSum = Semaev3_mod(P->x, Q->x, factorBase[j]->x, 0, b, primeNumber);
			//Check if sum == 0 mod field characteristic
			if(semaevSum == 0)
			{
				Point sum;
				InitializePoint(sum);
				AddCurvePoints(sum, P, Q, primeNumber);  
				AddCurvePoints(sum, sum, factorBase[j], primeNumber); 
				if(sum->infinity)
				{
					PrintRelation(randomA, generator, randomB, target, factorBase[j]);
				}	
			}

		}	
		
	}
	
	
	
	printf("\n");
	free(factorBase);
	free(allCurvePoints);
}

int main()
{
	TestBuildFactor();
	return 0;
}
