#pragma once

#include <vector>

#include "path.h"

class Generation
{
public:
	Generation();
	~Generation();
public:
	std::vector<Path> paths;  // ���� � ���������.
	int m_generationNumber;	  // ����� ��������� 
};

