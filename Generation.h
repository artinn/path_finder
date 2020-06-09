#pragma once

#include <vector>

#include "path.h"

class Generation
{
public:
	Generation();
	~Generation();
public:
	std::vector<Path> paths;  // Пути в поколении.
	int m_generationNumber;	  // Номер поколения 
};

