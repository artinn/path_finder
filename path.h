#pragma once

#include <vector>

#include "gen.h"

// Класс представляет из себя путь
class Path
{
public:
	Path(int size);
	~Path();
public:
	int m_size;
	std::vector<Gen> gens;	        // Вектор генов.
	float m_pathLength;		        // Длина пути.
	float m_survivalRate;	        // Обратный коэффициент выживаемости.
	float m_selectionProbability;	// вероятность выбра пути.
	bool m_posiblePath;				// Возможен ли путь.
};



