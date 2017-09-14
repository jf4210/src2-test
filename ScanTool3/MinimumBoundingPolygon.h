#pragma once
#include <list>

#define  CIRCLE_RADIANS  6.283185307179586476925286766559
//��ɢ����㹤��
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
	//���Ҷ��㣬Y���x��С
	DiscretePoint calMaxDots(LDISCRETEPOINT lDiscretePoint)
	{
		if (lDiscretePoint.size() == 0)
			return ;
	}

	/**
	* <b>����������</b>
	* ����������ɵ�������x���������������
	*/
	double angleOf(DiscretePoint s, DiscretePoint d) 
	{
		double dist = sqrt((s.getX() - d.getX()) * (s.getX() - d.getX()) + (s.getY() - d.getY()) * (s.getY() - d.getY()));

		if (dist <= 0)
			return .0;

		double x = d.getX() - s.getX(); // ֱ�������ε�ֱ��a
		double y = d.getY() - s.getY(); // ֱ�������ε�ֱ��b

		if (y >= 0.)	/* 1 2 ���� */
			return acos(x / dist);
		else			/* 3 4 ���� */
			return acos(-x / dist) + 0.5 * CIRCLE_RADIANS;
	}
	/**
	* <b>�����Ƕ�</b>
	* �����Ƕȵ� [0, 2PI]
	* @return  ������ĽǶ�
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

		//�ҵ�������Ķ���
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

				double currAngle = angleOf(ptPeak, pt); /* ��ǰ������x��������ļн� */
				double angleDif = reviseAngle(oldAngle - currAngle); /* ��������֮��ļнǣ�˳ʱ����ת�ļнǣ� */

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
	* <b>����������</b>
	* ����������ɵ�������x���������������
	*/
	double angleOf(DiscretePoint s, DiscretePoint d)
	{
		double dist = sqrt((s.getX() - d.getX()) * (s.getX() - d.getX()) + (s.getY() - d.getY()) * (s.getY() - d.getY()));

		if (dist <= 0)
			return .0;

		double x = d.getX() - s.getX(); // ֱ�������ε�ֱ��a
		double y = d.getY() - s.getY(); // ֱ�������ε�ֱ��b

		if (y >= 0.)	/* 1 2 ���� */
			return acos(x / dist);
		else			/* 3 4 ���� */
			return acos(-x / dist) + 0.5 * CIRCLE_RADIANS;
	}
	/**
	* <b>�����Ƕ�</b>
	* �����Ƕȵ� [0, 2PI]
	* @return  ������ĽǶ�
	*/
	double reviseAngle(double angle) {
		while (angle < 0.)
			angle += CIRCLE_RADIANS;
		while (angle >= CIRCLE_RADIANS)
			angle -= CIRCLE_RADIANS;

		return angle;
	}
};

