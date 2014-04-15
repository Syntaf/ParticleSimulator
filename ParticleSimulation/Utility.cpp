#include "Utility.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <iostream>

using namespace sf ;
using namespace std ;

Vector2f Diff(Vector2f const& v1, Vector2f const& v2, float const& deltaTime) 
{
	Vector2f diff = (v1-v2)/deltaTime ;
	return diff ;
}

float Diff(float const& a1, float const& a2, float const& deltaTime) 
{
	float diff = (a1-a2)/deltaTime ;
	return diff ;
}

float Distance(Vector2f const& v1, Vector2f const& v2)
{
	float distance = sqrt(pow((v2.x-v1.x),2) + pow((v2.y-v1.y),2)) ;
	return distance ;
}

float Norm (Vector2f const& v) 
{
	float result = sqrt(pow(v.x,2)+pow(v.y,2)) ;
	return result ;
}

float determinant(Vector2f const& u, Vector2f const& v) 
{
	float result = u.x*v.y - u.y*v.x ;
	return result ;
}

float dotProduct(Vector2f const& u, Vector2f const& v)
{
	return u.x*v.x + u.y*v.y ;
}

Vector2f rotatePoint(Vector2f const& point, Vector2f const& center, float const& angle) 
{
	float x = center.x + (point.x - center.x)*cos(angle*(PI/180)) - (point.y - center.y)*sin(angle*(PI/180)) ;
	float y = center.y + (point.x - center.x)*sin(angle*(PI/180)) + (point.y - center.y)*cos(angle*(PI/180)) ;
	return Vector2f(x,y) ;
}

bool checkSegmentIntersection(Vector2f const& A, Vector2f const& B, Vector2f const& C, Vector2f const& D)
{
	if(determinant(B-A,C-A)*determinant(B-A,D-A)<0 && determinant(D-C,A-C)*determinant(D-C,B-C)<0)
		return true ;
	return false ;
}

Vector2f getSegmentIntersection(Vector2f const& A, Vector2f const& B, Vector2f const& C, Vector2f const& D) 
{
	
	if(determinant(B-A,C-A)*determinant(B-A,D-A)<0 && determinant(D-C,A-C)*determinant(D-C,B-C)<0)
	{
		if(B.x-A.x != 0 && D.x-C.x !=0)
		{
			float a1 = (B.y-A.y)/(B.x-A.x) ;
			float a2 = (D.y-C.y)/(D.x-C.x) ;
			float b1 = A.y-a1*A.x ;
			float b2 = C.y-a2*C.x ;
			float Ix = (b2-b1)/(a1-a2) ;
			float Iy = a1*Ix+b1 ;
			return Vector2f(Ix,Iy) ;
		}

		if(B.x-A.x == 0)
		{
			float a2 = (D.y-C.y)/(D.x-C.x) ;
			float b1 = A.x ;
			float b2 = C.y-a2*C.x ;
			float Ix = b1 ;
			float Iy = a2*b1+b2 ;
			return Vector2f(Ix,Iy) ;
		}

		if(D.x-C.x == 0)
		{
			float a1 = (B.y-A.y)/(B.x-A.x) ;
			float b1 = A.y-a1*A.x ;
			float b2 = C.x ;
			float Ix = b2 ;
			float Iy = a1*b2+b1 ;
			return Vector2f(Ix,Iy) ;
		}		
	}
	return Vector2f();
}

void drawLine(Vector2f A, Vector2f B, RenderWindow &window, Color color)
{
	VertexArray line(Lines, 2) ;
	line[0].position = A ;
	line[0].color = color ;
	line[1].position = B ;
	line[1].color = color ;
	window.draw(line) ;
}

void drawLine(Vector2f pos, Vector2f dir, float norm, RenderWindow &window, Color color)
{
	dir  /= Norm(dir) ;
	VertexArray line(Lines, 2) ;
	line[0].position = pos ;
	line[0].color = color ;
	line[1].position = pos+dir*norm ;
	line[1].color = color ;
	window.draw(line) ;
}

float gaussianFunction(float maxVal, float wideness, float x) 
{
	return maxVal*exp(-pow(x,2)/(2*pow(wideness,2))) ;
}

float clamp(float value, float min, float max) 
{
	float result ;
	if(value > max)
		result = max ;
	else if(value < min)
		result = min ;
	else
		result = value ;
	return result ;
}