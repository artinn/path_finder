#include <cmath> 

#include <QPainter>
#include <QColor>
#include <QtWidgets>
#include <QMenu>


#include "geneticpathfinder.h"

#include "QMessageBox"

// ����������� ���������� ������� �����  ������������� � ���������.
#define HIGHT 670             // ������.
#define WIDE 720              // ������.
#define GRID_SIZE 40          // ������ ����� �����������.
#define M_FACTOR_DEFAULT 60	  // �������� �� ��������� ��� ����. �����������.
#define LENGHT_RANDOM_RANGE 3000  // ����������� �������� �����

// ����������.
geneticPathFinder::geneticPathFinder(QWidget *parent) : 
    // ������������� ����������.
	QMainWindow(parent),
	m_timer(new QTimer(this)),
	m_generationNumber(-1),
	m_masterColor("#000"),
	m_bestPathLength(10000),
	m_bestPath(10),
	m_maxGenerations(1000),
	m_mutationPropobility(5),
	m_factorMating(M_FACTOR_DEFAULT),
	m_isFindBest(false)
{
	m_universeSize = GRID_SIZE;	  // ������ ������ ����� �����������
	m_universe = new bool[(m_universeSize + 2) * (m_universeSize + 2)];	  // ������� ���� �����������
	memset(m_universe, false, sizeof(bool)*(m_universeSize + 2) * (m_universeSize + 2));  // �������� ��� ���� ������.

    // ��������� ��� ����� � ����������� ��������
	ui.setupUi(this);

	// ������������� �������� �������� ����������.
	m_timer->setInterval(200);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(nextStep()));

	ui.updateSpinBox->setRange(50, 5000);
	ui.updateSpinBox->setSingleStep(50);
	ui.updateSpinBox->setValue(200);
	ui.updateSpinBox->setSuffix(" ms");
	connect(ui.updateSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setInterval(int)));

	// ������������� �������� ��� ���������� ����� (�����).
	ui.gensLength->setRange(3, 1000);
	ui.gensLength->setSuffix(" g");
	ui.gensLength->setValue(10);
	m_gensLength = ui.gensLength->value();
	connect(ui.gensLength, SIGNAL(valueChanged(int)), this, SLOT(setGensLength(int)));

	// ������������� �������� ��� ������� ���������.
	ui.populationSize->setRange(3, 10000);
	ui.populationSize->setValue(1000);
	m_populationSize = ui.populationSize->value();
	connect(ui.populationSize, SIGNAL(valueChanged(int)), this, SLOT(setPopulationSize(int)));
	
	// ������������� �������� ��� ���������� ���������.
	ui.maxGenerations->setRange(100, 10000);
	ui.maxGenerations->setValue(1000);
	ui.maxGenerations->setSingleStep(100);
	connect(ui.maxGenerations, SIGNAL(valueChanged(int)), this, SLOT(setMaxGenerations(int)));

	// ������������� �������� ����������� �������.
	ui.mutationPropobility->setRange(0, 100);
	ui.mutationPropobility->setValue(5);
	ui.mutationPropobility->setSuffix(" %");
	connect(ui.mutationPropobility, SIGNAL(valueChanged(int)), this, SLOT(setMutation(int)));

	// ������������� �������� ����. �����������.
	ui.factorMating->setRange(1, 100);
	ui.factorMating->setValue(M_FACTOR_DEFAULT);
	ui.factorMating->setSuffix(" %");
	connect(ui.factorMating, SIGNAL(valueChanged(int)), this, SLOT(setFactorMating(int)));

	// ��������� ������ ����.
	ui.stop->setEnabled(false);

	// ������������� ��������� � �������� ����������.
	m_startPosition.setX(WIDE - 100);
	m_startPosition.setY(HIGHT - 100);
	m_endPosition.setX(100);
	m_endPosition.setY(100);

	// ������������� ������ ���������.
	setFirstGeneration();

    // �������������� �������� ������� ����.
	m_bestPath = createRandomPath();
}

// ���������� �� ������ �����.
void geneticPathFinder::on_start_clicked()
{
	// �������� � ��������� ������ ������.
	ui.start->setEnabled(false);
	ui.stop->setEnabled(true);
	ui.gensLength->blockSignals(true);
	ui.gensLength->setEnabled(false);
	ui.populationSize->blockSignals(true);
	ui.populationSize->setEnabled(false);
	ui.maxGenerations->setEnabled(false);
	ui.mutationPropobility->setEnabled(false);
	ui.factorMating->setEnabled(false);

   // ��������� ������.
	m_timer->start();
}

// ���������� �� ������ ����.
void geneticPathFinder::on_stop_clicked()
{
	// �������� � ��������� ������ ������.
	ui.start->setEnabled(true);
	ui.stop->setEnabled(false);
	ui.gensLength->blockSignals(false);
	ui.gensLength->setEnabled(true);
	ui.populationSize->blockSignals(false);
	ui.populationSize->setEnabled(true);
	ui.maxGenerations->setEnabled(true);
	ui.mutationPropobility->setEnabled(true);
	ui.factorMating->setEnabled(true);

    // ������������� ������.
	m_timer->stop();
}

// ���������� �� ������� ������ reset.
void geneticPathFinder::on_resetButton_clicked()
{
	// ���������� ����� ���������.
	m_generationNumber = -1;

	// ����������� ������� �������� ��� ������� ���� (���������� ���).
	m_bestPathLength = 10000;

	// ������� ��� ������ ���� ��� �� ��� ������.
	m_isFindBest = false;
	ui.best->setText("Best - not find");

	// �������� ������ ����.
	on_stop_clicked();

	// �� ����� �������������� ������ ���������.
	setFirstGeneration();

	// ������ ���� ������ ����� ���������
	m_bestPath = createRandomPath();

	// ��������� �����.
	update();
}

// ���������, ���������� � ����� ��������� ��� �� ������� update().
void geneticPathFinder::paintEvent(QPaintEvent * /* event */)
{
	// ������������� ��������� ������ "�����������"
	QPainter painter(this);
	// ������� ������ "�������" � �������.
	painter.setRenderHint(QPainter::Antialiasing, true);

	// ������ �����.
	paintGrid(painter);

	// ������ ����������.
	paintUniverse(painter);

	// ������������� ����� �������.
	int a = ui.gensLength->value();
	QPen pen(QPen(Qt::red, 1, Qt::SolidLine));
	painter.setPen(pen);

	QPoint currentPos;
	QPoint nextPos;
	if (m_generationNumber >= 0)
	{
		// ������ ������ ���� ���� �� ����������
		if (m_isFindBest)
		{
			currentPos = m_startPosition;
			QPen pen2(QPen(QColor(150, 220, 15, 255), 1, Qt::SolidLine));
			painter.setPen(pen2);
			for (size_t j = 0; j < m_gensLength; j++)
			{
				nextPos.setX(currentPos.x() + sin(m_bestPath.gens[j].corner) * m_bestPath.gens[j].length);
				nextPos.setY(currentPos.y() + cos(m_bestPath.gens[j].corner) * m_bestPath.gens[j].length);

				QLineF line(currentPos.x(), currentPos.y(), nextPos.x(), nextPos.y());

				currentPos = nextPos;
				painter.drawLine(line);
			}

			QLineF line(currentPos.x(), currentPos.y(), m_endPosition.x(), m_endPosition.y());
			painter.drawLine(line);
		}
	}

	// ������ ��������� � �������� �����.
	painter.setBrush(QBrush(Qt::blue, Qt::SolidPattern));
	painter.drawEllipse(m_endPosition.x(), m_endPosition.y(), 5, 5);
	painter.setBrush(QBrush(Qt::green, Qt::SolidPattern));
	painter.drawEllipse(m_startPosition.x(), m_startPosition.y(), 5, 5);
}

// ��������� ��������� ��������� ���������� �� �������.
void geneticPathFinder::nextStep()
{
	// ������������� ����� ���������.
	m_generationNumber++;

	// ������� �� ����� ����� ���������
	QString generationNumberS = "Generation number - ";
	generationNumberS += std::to_string(m_generationNumber + 1).c_str();
	ui.generationNumber->setText(generationNumberS);

	// ���� �� ���������� ������ ���������� ��������� �� ������ �� �������.
	if (m_maxGenerations <= m_generationNumber)
	{
		return;
	}

	QPoint currentPos;
	QPoint nextPos;
	if (m_generationNumber >= 0)
	{
		for (size_t i = 0; i < m_populationSize; i++)
		{
			currentPos = m_startPosition;
			float corner = 0;
			for (size_t j = 0; j < m_gensLength; j++)
			{
				 // ��������� ���������� �����.
				nextPos.setX(currentPos.x() + sin(m_generation.paths[i].gens[j].corner) * m_generation.paths[i].gens[j].length);
				nextPos.setY(currentPos.y() + cos(m_generation.paths[i].gens[j].corner) * m_generation.paths[i].gens[j].length);

				// ��������� ����� �� �� ��������� ���� �� ����� �� �����
				QLineF line(currentPos.x(), currentPos.y(), nextPos.x(), nextPos.y());
				if (!isPosiblePath(line))
				{
					m_generation.paths[i].m_posiblePath = false;
					break;
				}
				currentPos = nextPos;
			}

			// ��������� ����� �� �� ��������� �� ����� �� ��������� �����.
			QLineF line(currentPos.x(), currentPos.y(), m_endPosition.x(), m_endPosition.y());
			if (!isPosiblePath(line))
			{
				m_generation.paths[i].m_posiblePath = false;
			}
		}
	}

	// ������� ������.
	float survivalRateSumm = 0;
	for (size_t i = 0; i < m_populationSize; i++)
	{
		currentPos = m_startPosition;
		float pathLength = 0;
		float corner = 0;
		for (size_t j = 0; j < m_gensLength; j++)
		{

			nextPos.setX(currentPos.x() + sin(m_generation.paths[i].gens[j].corner) * m_generation.paths[i].gens[j].length);
			nextPos.setY(currentPos.y() + cos(m_generation.paths[i].gens[j].corner) * m_generation.paths[i].gens[j].length);
   			pathLength += distance(currentPos, nextPos);

			currentPos = nextPos;
		}

		pathLength += distance(currentPos, m_endPosition);
		m_generation.paths[i].m_pathLength = pathLength;

		//���� ������� ���� �� ����� �������� ������������
		if (!m_isFindBest)
		{
			ui.best->setText("Best - not find");
		}

		// ���� ����� ����� ������ ���� �� ���������� ��� �����
		if ((pathLength < m_bestPathLength) && (m_generation.paths[i].m_posiblePath))
		{
			m_isFindBest = true;
			m_bestPathLength = m_generation.paths[i].m_pathLength;
			m_bestPath = m_generation.paths[i];
			QString string = "Best - ";
			string += std::to_string(m_bestPathLength).c_str();
			ui.best->setText(string);
		}

		// ����������� ������� ������� ����������� ���� �������� ������� ������ � �� ��������
		if (m_generation.paths[i].m_posiblePath)
		{
			m_generation.paths[i].m_survivalRate = 1 / (m_generation.paths[i].m_pathLength);
		}
		else
		{
			m_generation.paths[i].m_survivalRate = 1 / (m_generation.paths[i].m_pathLength + 1000000);
		}
		
		survivalRateSumm += m_generation.paths[i].m_survivalRate;

	}

	// ������� ����������� ������ �����
	for (size_t i = 0; i < m_populationSize; i++)
	{
		m_generation.paths[i].m_selectionProbability = m_generation.paths[i].m_survivalRate / survivalRateSumm;
	}

	struct sort_class
	{
		bool operator() (Path i, Path j)
		{
			return (i.m_pathLength > j.m_pathLength);
		}
	} sort_object;

	// ��������� ����� �� ������� �������.
	std::sort(m_generation.paths.begin(), m_generation.paths.end(), sort_object);

	// ������� ������ � ������� ����� �������� ���� �������� ������������ ������.
	std::vector<std::pair<int, int>> parentsPair;
	
	// �������� ���� ���������.
	for (size_t i = 0; i < m_populationSize; i++)
	{
		float sum = 0;
		float firstRand		= ((float)(rand() % 1000)) / 1000;
		float secondRand	= ((float)(rand() % 1000)) / 1000;
		int firstNumber;
		int secondNumber;

		for (size_t j = 0; j < m_populationSize; j++)
		{
			sum += m_generation.paths[j].m_selectionProbability;
			if (firstRand < sum)
			{
				firstNumber = j;
				break;
			}
		}

		sum = 0;
		for (size_t j = 0; j < m_populationSize; j++)
		{
			if (j == firstNumber)
			{
				if (j != 0)
				{
					secondNumber = j - 1;
					continue;
				}
				else
				{
					secondNumber = j + 1;
					continue;
				}
			}
			sum += m_generation.paths[j].m_selectionProbability;
			if (secondRand < sum)
			{
				secondNumber = j;
				break;
			}
		}
		
		parentsPair.push_back(std::make_pair(firstNumber, secondNumber));
	}

	// ������� ����� ���������
	int range = (m_factorMating * m_populationSize) / 100;
	for (size_t i = 0; i < range; i++)
	{
		if (m_generationShadow.paths[i].m_posiblePath)
		{
			m_generationShadow.paths[i] = crossingOver(m_generation.paths[parentsPair[i].first], m_generation.paths[parentsPair[i].second]);
		}
	}

	m_generation = m_generationShadow;
	m_generation.m_generationNumber = m_generationNumber;

	update();
}

// ���������� ��� ����� ��������� ����������.
void geneticPathFinder::setInterval(int interval)
{
	m_timer->setInterval(interval);
}

// ���������� ��� ��������� ���������� �����(�����).
void geneticPathFinder::setGensLength(int length)
{
	m_gensLength = length;
	on_resetButton_clicked();
}

// ������� ��������� ����.
Path geneticPathFinder::createRandomPath()
{
	Path path(m_gensLength);
	for (size_t i = 0; i < m_gensLength; i++)
	{
		// ������ ��������� ��� ������� ���������� ��������� � ���� �� ��������� �����.
		Gen gen;
		gen.length = (float)(rand() % LENGHT_RANDOM_RANGE) / 10;
		gen.corner = (float)(rand() % 6291) / 1000;

		// ���� ������ ����� �� ��� ����������  ����� ������.
		if (i == 0)
		{
			//	��������� ��� ��������� ��������� �����.
			QPoint place;
			place.setX(m_startPosition.x() + sin(gen.corner) * gen.length);
			place.setY(m_startPosition.y() + cos(gen.corner) * gen.length);

			// ���� ����� ��� ������� �� ����� ������ ��������� ����� �� ����.
			if (!isInPlace(place))
			{
				place.setX((float)(rand() % WIDE));
				place.setY((float)(rand() % HIGHT));

				// ��������� ���� � ����� ������� �� ���.
				gen.length = distance(place, m_startPosition);
				auto deltaY = m_startPosition.y() - place.y();
				if (deltaY == 0)
				{
					gen.corner = 0;
				}
				else
				{
					gen.corner = acos(deltaY / gen.length);
				}
			}

			gen.x = place.x();
			gen.y = place.y();
		}
		else
		{
			QPoint place;
			place.setX(path.gens[i - 1].x + sin(gen.corner) * gen.length);
			place.setY(path.gens[i - 1].y + cos(gen.corner) * gen.length);

			if (!isInPlace(place))
			{
				place.setX((float)(rand() % WIDE));
				place.setY((float)(rand() % HIGHT));

				QPoint previous;
				previous.setX(path.gens[i - 1].x);
				previous.setY(path.gens[i - 1].y);
				gen.length = distance(place, previous);
				auto deltaY = m_startPosition.y() - place.y();
				if (deltaY == 0)
				{
					gen.corner = 0;
				}
				else
				{
					gen.corner = asin(deltaY / gen.length);
				}
			}

			gen.x = place.x();
			gen.y = place.y();
		}
		path.gens.push_back(gen);
	}
	return path;
}

// ������������.
Path geneticPathFinder::crossingOver(Path pathLhs, Path pathRhs)
{
	Path newChild(m_gensLength);

	// ����� ���� �������� ��� �� ����� ������������ ����� �� �� ������.
	for (size_t i = 0; i < m_gensLength; i++)
	{
		int randomTemp1 = rand() % 2;
		if (randomTemp1 == 0)
		{
			newChild.gens.push_back(pathLhs.gens[i]);
		}
		else
		{
			newChild.gens.push_back(pathRhs.gens[i]);
		}

		// ������� ��������� � �������� ������������.
		int randomTemp2 = rand() % 100;

		if (randomTemp2 <= m_mutationPropobility)
		{
			newChild.gens[i].length = (float)(rand() % LENGHT_RANDOM_RANGE) / 10;
			newChild.gens[i].corner = (float)(rand() % 6291) / 1000;
		}
	}
	return newChild;
}

// ������������� ������ ��������� �� ��������� ���������.
void geneticPathFinder::setPopulationSize(int populationSize)
{
	m_populationSize = populationSize;
	on_resetButton_clicked();
}

// ������������� ������������ ���������� ��������� �� ��������� ���������.
void geneticPathFinder::setMaxGenerations(int maxGenerations)
{
	m_maxGenerations = maxGenerations;
	on_resetButton_clicked();
}

// ������������� ����������� ������� �� ��������� ���������	.
void geneticPathFinder::setMutation(int mutationPropobility)
{
	m_mutationPropobility = mutationPropobility;
	on_resetButton_clicked();
}

// ������������� ����������� �����������.
void geneticPathFinder::setFactorMating(int factorMating)
{
	m_factorMating = factorMating;
	on_resetButton_clicked();
}

// ���������� ����� ����� �������.
float geneticPathFinder::distance(QPoint l, QPoint r)
{
	return ::sqrt((float)((l.x() - r.x()) * (l.x() - r.x()) + (l.y() - r.y()) * (l.y() - r.y())));
}

// ������������� ������ ���������.
void geneticPathFinder::setFirstGeneration()
{
	//Generation generation;
	m_generation.m_generationNumber = 0;
	m_generation.paths.erase(m_generation.paths.begin(), m_generation.paths.end());
	m_generationShadow.paths.erase(m_generationShadow.paths.begin(), m_generationShadow.paths.end());
	for (size_t i = 0; i < m_populationSize; i++)
	{
		m_generation.paths.push_back(createRandomPath());
	}
	m_generationShadow = m_generation;
}

// ����������� �����
void geneticPathFinder::paintGrid(QPainter &p)
{
	QRect borders(0, 0, WIDE, HIGHT); // borders of the universe
	QColor gridColor = m_masterColor; // color of the grid
	gridColor.setAlpha(10); // must be lighter than main color
	p.setPen(gridColor);
	double cellWidth = (double)WIDE / m_universeSize; // width of the widget / number of cells at one row
	for (double k = cellWidth; k <= WIDE; k += cellWidth)
		p.drawLine(k, 0, k, HIGHT);
	double cellHeight = (double)HIGHT / m_universeSize; // height of the widget / number of cells at one row
	for (double k = cellHeight; k <= HIGHT; k += cellHeight)
		p.drawLine(0, k, WIDE, k);
	p.drawRect(borders);
}

// ���������� ��� ����������� ������� ����
void geneticPathFinder::mouseMoveEvent(QMouseEvent *e)
{
	double cellWidth = (double)WIDE / m_universeSize;
	double cellHeight = (double)HIGHT / m_universeSize;
	if ((e->y() <= HIGHT) && (e->x() <= WIDE))
	{
		int k = floor(e->y() / cellHeight) + 1;
		int j = floor(e->x() / cellWidth) + 1;
		int currentLocation = k*m_universeSize + j;
		if (!m_universe[currentLocation]){                //if current cell is empty,fill in it
			m_universe[currentLocation] = !m_universe[currentLocation];
			update();
		}
	}
}

// ������ �����������.
void geneticPathFinder::paintUniverse(QPainter &p)
{
	double cellWidth = (double)WIDE / m_universeSize;
	double cellHeight = (double)HIGHT / m_universeSize;
	for (int k = 1; k <= m_universeSize; k++) {
		for (int j = 1; j <= m_universeSize; j++) {
			if (m_universe[k*m_universeSize + j] == true) { // if there is any sense to paint it
				qreal left = (qreal)(cellWidth*j - cellWidth); // margin from left
				qreal top = (qreal)(cellHeight*k - cellHeight); // margin from top
				QRectF r(left, top, (qreal)cellWidth, (qreal)cellHeight);
				p.fillRect(r, QBrush(m_masterColor)); // fill cell with brush of main color
			}
		}
	}
}

// ������������� ��������� �������.
void geneticPathFinder::setStartPosition()
{
	m_startPosition = m_setPosition;
}

// ������������� �������� �������.
void geneticPathFinder::setEndingPosition()
{
	m_endPosition = m_setPosition;
}

// ���������� ��� ������� ������ ����
void geneticPathFinder::mousePressEvent(QMouseEvent *event)
{
	// ���� ������ � �������� ����
	if ((event->y() <= HIGHT) && (event->x() <= WIDE))
	{
		// ������ ������ ������ ���������� ����������� ����
		if (event->button() == Qt::RightButton)
		{
			// ������� ����.
			QMenu *contextMenu = new QMenu(tr("Context menu"), this);
		
			// ������� ���������� ����.
			m_setPosition.setX(event->pos().x());
			m_setPosition.setY(event->pos().y());

			// ������� �������� ������� � ����� ������ �������� ��� ������� �� ���.
			QAction* action1 = contextMenu->addAction("Start position", this, SLOT(setStartPosition()));
			QAction* action2 = contextMenu->addAction("Ending position", this, SLOT(setEndingPosition()));

			// ���������� ����.
			contextMenu->exec(mapToGlobal(event->pos()));
		}
		// ���� ������ ����� ������� �� ������� �����������.
		else if (event->button() == Qt::LeftButton)
		{
			emit environmentChanged(true);
			double cellWidth = (double)WIDE / m_universeSize;
			double cellHeight = (double)HIGHT / m_universeSize;
			int k = floor(event->y() / cellHeight) + 1;
			int j = floor(event->x() / cellWidth) + 1;
			m_universe[k*m_universeSize + j] = !m_universe[k*m_universeSize + j];
		}
		update();
	}
	
}

// ��������� ��������� �� ��� ���� (���� �� ���������� ����������� � �� ������ �� ������� �������� ����)
bool geneticPathFinder::isPosiblePath(QLineF line)
{
	bool value = true;
	double cellWidth = (double)WIDE / m_universeSize;
	double cellHeight = (double)HIGHT / m_universeSize;
	for (int k = 1; k <= m_universeSize; k++) 
	{
		for (int j = 1; j <= m_universeSize; j++) 
		{
			if (m_universe[k*m_universeSize + j] == true) 
			{
				qreal left = (qreal)(cellWidth*j - cellWidth); 
				qreal top = (qreal)(cellHeight*k - cellHeight);
				QRectF r(left, top, (qreal)cellWidth, (qreal)cellHeight);
				
				if (isCrossing(r, line))
				{
					value = false;
				}
			}
		}
	}

	QRectF border(0, 0, (qreal)WIDE, (qreal)HIGHT);

	if (isCrossing(border, line))
	{
		value = false;
	}

	return value;
}

// �������� ���������� �� ����� �������.
bool geneticPathFinder::isCrossing(QRectF rect, QLineF line)
{
	QPointF topLeft = rect.topLeft();
	QPointF topRight = rect.topRight();
	QPointF bottomLeft = rect.bottomLeft();
	QPointF bottomRight = rect.bottomRight();

	bool value = false;
	if (isLineCrossed(topLeft, topRight, line) ||
		(isLineCrossed(topLeft, bottomLeft, line) ||
		(isLineCrossed(bottomLeft, bottomRight, line) ||
		(isLineCrossed(bottomRight, topRight, line)))))
	{
		value = true;
	}
	return value;
}

// ��������� ������������ �� �����.
bool geneticPathFinder::isLineCrossed(QPointF a, QPointF b, QLineF line)
{
	QPointF c = line.p1();
	QPointF d = line.p2();
	return intersect_1(a.x(), b.x(), c.x(), d.x())
			&& intersect_1(a.y(), b.y(), c.y(), d.y())
			&& area(a, b, c) * area(a, b, d) <= 0
			&& area(c, d, a) * area(c, d, b) <= 0;
}

// ��������� ������� � �������� �� ������������ �� �����������.
float geneticPathFinder::area(QPointF a, QPointF b, QPointF c)
{
	return (b.x() - a.x()) * (c.y() - a.y()) - (b.y() - a.y()) * (c.x() - a.x());
}

// ��������������� ������� ��� ����������� ����������� �� ������ �������������.
bool geneticPathFinder::intersect_1(qreal a, qreal b, qreal c, qreal d)
{
	if (a > b)  std::swap(a, b);
	if (c > d)  std::swap(c, d);
	return std::max(a, c) <= std::min(b, d);
}

// ������� ���� �����������.
void geneticPathFinder::on_hideField_clicked()
{
	for (size_t i = 0; i < m_universeSize + 1; i++)
	{
		for (size_t j = 0; j < m_universeSize; j++)
		{
			int currentLocation = i*m_universeSize + j;
			m_universe[currentLocation] = false;
		}
	}
	update();
}

// �������� ��������� �� �� � ������� ����.
bool geneticPathFinder::isInPlace(QPoint point)
{
	return (((point.x() >= 0) && (point.x() <= WIDE)) &&
		((point.y() >= 0) && (point.y() <= HIGHT)));
}

// ����������.
geneticPathFinder::~geneticPathFinder()
{

}