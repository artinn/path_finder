#pragma once

#include <vector>

#include "gen.h"

// ����� ������������ �� ���� ����
class Path
{
public:
	Path(int size);
	~Path();
public:
	int m_size;
	std::vector<Gen> gens;	        // ������ �����.
	float m_pathLength;		        // ����� ����.
	float m_survivalRate;	        // �������� ����������� ������������.
	float m_selectionProbability;	// ����������� ����� ����.
	bool m_posiblePath;				// �������� �� ����.
};



