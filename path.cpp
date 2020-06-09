#include "path.h"


Path::Path(int size):
	m_size(size),
	m_posiblePath(true)
{
	gens.reserve(m_size);
}


Path::~Path()
{
}

