#pragma once
#include <list>

#define  CIRCLE_RADIANS  6.283185307179586476925286766559
//离散点计算工具
class DiscretePoint
{
public:
	DiscretePoint() {};
	DiscretePoint(double x, double y)
	{
		this->x = x;
		this->y = y;
	}
	double getX() { return x; }
	double getY() { return y; }
	void setX(double x) { this->x = x; }
	void setY(double y) { this->y = y; }
	bool operator == (DiscretePoint& pt)
	{
		return (this->x == pt.getX() && this->y == pt.getY());
	}
	bool bFound = false;
private:
	double x;
	double y;
};

typedef std::list<DiscretePoint> LDISCRETEPOINT;

class DiscretePointUtil
{
public:
	DiscretePointUtil() {};
	~DiscretePointUtil() {};
	//查找顶点，Y最大，x最小
	DiscretePoint calMaxDots(LDISCRETEPOINT lDiscretePoint)
	{
		if (lDiscretePoint.size() == 0)
			return ;
	}

	/**
	* <b>计算向量角</b>
	* 计算两点组成的向量与x轴正方向的向量角
	*/
	double angleOf(DiscretePoint s, DiscretePoint d) 
	{
		double dist = sqrt((s.getX() - d.getX()) * (s.getX() - d.getX()) + (s.getY() - d.getY()) * (s.getY() - d.getY()));

		if (dist <= 0)
			return .0;

		double x = d.getX() - s.getX(); // 直角三角形的直边a
		double y = d.getY() - s.getY(); // 直角三角形的直边b

		if (y >= 0.)	/* 1 2 象限 */
			return acos(x / dist);
		else			/* 3 4 象限 */
			return acos(-x / dist) + 0.5 * CIRCLE_RADIANS;
	}
	/**
	* <b>修正角度</b>
	* 修正角度到 [0, 2PI]
	* @return  修正后的角度
	*/
	double reviseAngle(double angle) {
		while (angle < 0.) 
			angle += CIRCLE_RADIANS;
		while (angle >= CIRCLE_RADIANS)
			angle -= CIRCLE_RADIANS;

		return angle;
	}
};

class CMinimumBoundingPolygon
{
public:
	CMinimumBoundingPolygon() {};
	~CMinimumBoundingPolygon() {};

	LDISCRETEPOINT findSmallestPolygon(LDISCRETEPOINT lDiscretePt)
	{
		if (lDiscretePt.size() < 1) return ;

		//找到最靠上最靠左的定点
		DiscretePoint ptPeak = lDiscretePt.front();
		for (auto itemPt : lDiscretePt)
			if ((itemPt.getY() > ptPeak.getY()) || (itemPt.getY() == ptPeak.getY() && itemPt.getX() < ptPeak.getX()))
				ptPeak = itemPt;

		double minAngleDif, oldAngle = CIRCLE_RADIANS;
		LDISCRETEPOINT lBound;
		do 
		{
			minAngleDif = CIRCLE_RADIANS;
			lBound.push_back(ptPeak);

			DiscretePoint nextPoint = ptPeak;
			double nextAngle = oldAngle;
			for (auto pt : lDiscretePt)
			{
				if(pt.bFound) continue;
				if(pt == ptPeak) continue;

				double currAngle = angleOf(ptPeak, pt); /* 当前向量与x轴正方向的夹角 */
				double angleDif = reviseAngle(oldAngle - currAngle); /* 两条向量之间的夹角（顺时针旋转的夹角） */

				if (angleDif < minAngleDif) 
				{
					minAngleDif = angleDif;
					nextPoint = pt;
					nextAngle = currAngle;
				}
			}
			oldAngle = nextAngle;
			ptPeak = nextPoint;
			ptPeak.bFound = true;
		} while (!(ptPeak == lBound.front()));

		return lBound;
	}

private:
	/**
	* <b>计算向量角</b>
	* 计算两点组成的向量与x轴正方向的向量角
	*/
	double angleOf(DiscretePoint s, DiscretePoint d)
	{
		double dist = sqrt((s.getX() - d.getX()) * (s.getX() - d.getX()) + (s.getY() - d.getY()) * (s.getY() - d.getY()));

		if (dist <= 0)
			return .0;

		double x = d.getX() - s.getX(); // 直角三角形的直边a
		double y = d.getY() - s.getY(); // 直角三角形的直边b

		if (y >= 0.)	/* 1 2 象限 */
			return acos(x / dist);
		else			/* 3 4 象限 */
			return acos(-x / dist) + 0.5 * CIRCLE_RADIANS;
	}
	/**
	* <b>修正角度</b>
	* 修正角度到 [0, 2PI]
	* @return  修正后的角度
	*/
	double reviseAngle(double angle) {
		while (angle < 0.)
			angle += CIRCLE_RADIANS;
		while (angle >= CIRCLE_RADIANS)
			angle -= CIRCLE_RADIANS;

		return angle;
	}
};

