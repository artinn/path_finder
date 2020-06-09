#include <cmath> 

#include <QPainter>
#include <QColor>
#include <QtWidgets>
#include <QMenu>


#include "geneticpathfinder.h"

#include "QMessageBox"

// Константный переменные который будут  использоватся в программе.
#define HIGHT 670             // Высота.
#define WIDE 720              // Ширина.
#define GRID_SIZE 40          // Размер сетки препятствий.
#define M_FACTOR_DEFAULT 60	  // Значение по умолчанию для коэф. скрещивания.
#define LENGHT_RANDOM_RANGE 3000  // Максимально значение длины

// Кнструктор.
geneticPathFinder::geneticPathFinder(QWidget *parent) : 
    // Инициализация переменных.
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
	m_universeSize = GRID_SIZE;	  // Задаем размер сетки препядствий
	m_universe = new bool[(m_universeSize + 2) * (m_universeSize + 2)];	  // Создаем поле препядствий
	memset(m_universe, false, sizeof(bool)*(m_universeSize + 2) * (m_universeSize + 2));  // Выделяем под поле память.

    // Привязали наш класс и графический интерфей
	ui.setupUi(this);

	// Установливаем спинбокс интервал обновления.
	m_timer->setInterval(200);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(nextStep()));

	ui.updateSpinBox->setRange(50, 5000);
	ui.updateSpinBox->setSingleStep(50);
	ui.updateSpinBox->setValue(200);
	ui.updateSpinBox->setSuffix(" ms");
	connect(ui.updateSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setInterval(int)));

	// Установливаем спинбокс для количества точек (генов).
	ui.gensLength->setRange(3, 1000);
	ui.gensLength->setSuffix(" g");
	ui.gensLength->setValue(10);
	m_gensLength = ui.gensLength->value();
	connect(ui.gensLength, SIGNAL(valueChanged(int)), this, SLOT(setGensLength(int)));

	// Установливаем спинбокс для размера популяции.
	ui.populationSize->setRange(3, 10000);
	ui.populationSize->setValue(1000);
	m_populationSize = ui.populationSize->value();
	connect(ui.populationSize, SIGNAL(valueChanged(int)), this, SLOT(setPopulationSize(int)));
	
	// Установливаем спинбокс для количество поколений.
	ui.maxGenerations->setRange(100, 10000);
	ui.maxGenerations->setValue(1000);
	ui.maxGenerations->setSingleStep(100);
	connect(ui.maxGenerations, SIGNAL(valueChanged(int)), this, SLOT(setMaxGenerations(int)));

	// Установливаем спинбокс вероятность мутации.
	ui.mutationPropobility->setRange(0, 100);
	ui.mutationPropobility->setValue(5);
	ui.mutationPropobility->setSuffix(" %");
	connect(ui.mutationPropobility, SIGNAL(valueChanged(int)), this, SLOT(setMutation(int)));

	// Устанавливаем спинбокс коэф. скрещивания.
	ui.factorMating->setRange(1, 100);
	ui.factorMating->setValue(M_FACTOR_DEFAULT);
	ui.factorMating->setSuffix(" %");
	connect(ui.factorMating, SIGNAL(valueChanged(int)), this, SLOT(setFactorMating(int)));

	// Отключаем кнопку стоп.
	ui.stop->setEnabled(false);

	// Устанавливаем начальные и конечные координаты.
	m_startPosition.setX(WIDE - 100);
	m_startPosition.setY(HIGHT - 100);
	m_endPosition.setX(100);
	m_endPosition.setY(100);

	// Устанавливаем первое поколение.
	setFirstGeneration();

    // Инициализируем значение лучшего пути.
	m_bestPath = createRandomPath();
}

// Вызывается по кнопке старт.
void geneticPathFinder::on_start_clicked()
{
	// Включаем и выключаем нужные кнопки.
	ui.start->setEnabled(false);
	ui.stop->setEnabled(true);
	ui.gensLength->blockSignals(true);
	ui.gensLength->setEnabled(false);
	ui.populationSize->blockSignals(true);
	ui.populationSize->setEnabled(false);
	ui.maxGenerations->setEnabled(false);
	ui.mutationPropobility->setEnabled(false);
	ui.factorMating->setEnabled(false);

   // Запускаем таймер.
	m_timer->start();
}

// Вызывается по кнопке стоп.
void geneticPathFinder::on_stop_clicked()
{
	// Включаем и выключаем нужные кнопки.
	ui.start->setEnabled(true);
	ui.stop->setEnabled(false);
	ui.gensLength->blockSignals(false);
	ui.gensLength->setEnabled(true);
	ui.populationSize->blockSignals(false);
	ui.populationSize->setEnabled(true);
	ui.maxGenerations->setEnabled(true);
	ui.mutationPropobility->setEnabled(true);
	ui.factorMating->setEnabled(true);

    // Останавливаем таймер.
	m_timer->stop();
}

// Вызывается по нажатию кнопки reset.
void geneticPathFinder::on_resetButton_clicked()
{
	// Сбрасываем номер поколения.
	m_generationNumber = -1;

	// Прописываем большое значение для лучшего пути (сбрасываем его).
	m_bestPathLength = 10000;

	// Говорим что лучший путь еще не был найден.
	m_isFindBest = false;
	ui.best->setText("Best - not find");

	// Нажимаем кнопку стоп.
	on_stop_clicked();

	// По новой инициализируем первое поколение.
	setFirstGeneration();

	// Лучший путь ставим любым случайным
	m_bestPath = createRandomPath();

	// Обновляем экран.
	update();
}

// Отрисовка, вызывается в цикле автоматом или по команде update().
void geneticPathFinder::paintEvent(QPaintEvent * /* event */)
{
	// Устанавливаем экземпляр класса "рисовальщик"
	QPainter painter(this);
	// Убираем эффект "лесенки" в рисовке.
	painter.setRenderHint(QPainter::Antialiasing, true);

	// Рисуем сетку.
	paintGrid(painter);

	// Рисуем предяствия.
	paintUniverse(painter);

	// Устанавливаем стиль рисовки.
	int a = ui.gensLength->value();
	QPen pen(QPen(Qt::red, 1, Qt::SolidLine));
	painter.setPen(pen);

	QPoint currentPos;
	QPoint nextPos;
	if (m_generationNumber >= 0)
	{
		// Рисуем лучший путь если он существует
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

	// Рисуем начальную и конечную точки.
	painter.setBrush(QBrush(Qt::blue, Qt::SolidPattern));
	painter.drawEllipse(m_endPosition.x(), m_endPosition.y(), 5, 5);
	painter.setBrush(QBrush(Qt::green, Qt::SolidPattern));
	painter.drawEllipse(m_startPosition.x(), m_startPosition.y(), 5, 5);
}

// Вычисляем следующее поколение вызывается по таймеру.
void geneticPathFinder::nextStep()
{
	// Устанавливаем номер поколения.
	m_generationNumber++;

	// Выводим на экран номер поколения
	QString generationNumberS = "Generation number - ";
	generationNumberS += std::to_string(m_generationNumber + 1).c_str();
	ui.generationNumber->setText(generationNumberS);

	// Если мы подсчитали нужное количество поколений то дальше не считаем.
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
				 // Вычисляем координаты точки.
				nextPos.setX(currentPos.x() + sin(m_generation.paths[i].gens[j].corner) * m_generation.paths[i].gens[j].length);
				nextPos.setY(currentPos.y() + cos(m_generation.paths[i].gens[j].corner) * m_generation.paths[i].gens[j].length);

				// Проверяем можем ли мы проложить путь от точки до точки
				QLineF line(currentPos.x(), currentPos.y(), nextPos.x(), nextPos.y());
				if (!isPosiblePath(line))
				{
					m_generation.paths[i].m_posiblePath = false;
					break;
				}
				currentPos = nextPos;
			}

			// Проверяем можем ли мы проложить от точки до последней точки.
			QLineF line(currentPos.x(), currentPos.y(), m_endPosition.x(), m_endPosition.y());
			if (!isPosiblePath(line))
			{
				m_generation.paths[i].m_posiblePath = false;
			}
		}
	}

	// Считаем выгоду.
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

		//Если лучшего пути не нашли сообщаем пользователю
		if (!m_isFindBest)
		{
			ui.best->setText("Best - not find");
		}

		// если нашли новый лучший путь то отобразили его длину
		if ((pathLength < m_bestPathLength) && (m_generation.paths[i].m_posiblePath))
		{
			m_isFindBest = true;
			m_bestPathLength = m_generation.paths[i].m_pathLength;
			m_bestPath = m_generation.paths[i];
			QString string = "Best - ";
			string += std::to_string(m_bestPathLength).c_str();
			ui.best->setText(string);
		}

		// Своебразная фитнесс функция невозможные пути получают большие штрафы и не выживают
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

	// Считаем вероятность выбора особи
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

	// Сортируем особи по фитнесс функции.
	std::sort(m_generation.paths.begin(), m_generation.paths.end(), sort_object);

	// Создаем список в котором будут хранится пары индексов родительских особей.
	std::vector<std::pair<int, int>> parentsPair;
	
	// Выбираем пары родителей.
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

	// Создаем новое поколение
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

// Вызывается при смене интервала обновления.
void geneticPathFinder::setInterval(int interval)
{
	m_timer->setInterval(interval);
}

// Вызывается при изменении количества генов(точек).
void geneticPathFinder::setGensLength(int length)
{
	m_gensLength = length;
	on_resetButton_clicked();
}

// Создает случайный путь.
Path geneticPathFinder::createRandomPath()
{
	Path path(m_gensLength);
	for (size_t i = 0; i < m_gensLength; i++)
	{
		// Задаем случайный ген который определяет растояние и угол до следующей точки.
		Gen gen;
		gen.length = (float)(rand() % LENGHT_RANDOM_RANGE) / 10;
		gen.corner = (float)(rand() % 6291) / 1000;

		// Если первая точка то как предыдущую  берем начало.
		if (i == 0)
		{
			//	Вычислили где получится следующая точка.
			QPoint place;
			place.setX(m_startPosition.x() + sin(gen.corner) * gen.length);
			place.setY(m_startPosition.y() + cos(gen.corner) * gen.length);

			// Если точка вне области то берем другую случайную точку на поле.
			if (!isInPlace(place))
			{
				place.setX((float)(rand() % WIDE));
				place.setY((float)(rand() % HIGHT));

				// Вычисляем угол и длину отрезка до нее.
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

// Кроссинговер.
Path geneticPathFinder::crossingOver(Path pathLhs, Path pathRhs)
{
	Path newChild(m_gensLength);

	// Берем гены случайно или от одной родительской особи ли от другой.
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

		// Мутация случайная с заданной вероятностью.
		int randomTemp2 = rand() % 100;

		if (randomTemp2 <= m_mutationPropobility)
		{
			newChild.gens[i].length = (float)(rand() % LENGHT_RANDOM_RANGE) / 10;
			newChild.gens[i].corner = (float)(rand() % 6291) / 1000;
		}
	}
	return newChild;
}

// Устанавливаем размер популяции по изменение спинбокса.
void geneticPathFinder::setPopulationSize(int populationSize)
{
	m_populationSize = populationSize;
	on_resetButton_clicked();
}

// Устанавливаем маскимальное количество поколений по изменение спинбокса.
void geneticPathFinder::setMaxGenerations(int maxGenerations)
{
	m_maxGenerations = maxGenerations;
	on_resetButton_clicked();
}

// Устанавливаем коэффициент мутации по изменение спинбокса	.
void geneticPathFinder::setMutation(int mutationPropobility)
{
	m_mutationPropobility = mutationPropobility;
	on_resetButton_clicked();
}

// Устанавливаем коэффициент скрещивания.
void geneticPathFinder::setFactorMating(int factorMating)
{
	m_factorMating = factorMating;
	on_resetButton_clicked();
}

// Расстояние между двумя точками.
float geneticPathFinder::distance(QPoint l, QPoint r)
{
	return ::sqrt((float)((l.x() - r.x()) * (l.x() - r.x()) + (l.y() - r.y()) * (l.y() - r.y())));
}

// Устанавливаем первое поколение.
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

// Закращиваем сетку
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

// Вызывается при перемещении нажатой мыши
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

// Рисуем препядствия.
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

// Устанавливаем стартовую позицию.
void geneticPathFinder::setStartPosition()
{
	m_startPosition = m_setPosition;
}

// Устанавливаем конечную позицию.
void geneticPathFinder::setEndingPosition()
{
	m_endPosition = m_setPosition;
}

// Вызывается при нажатии кнопки мыши
void geneticPathFinder::mousePressEvent(QMouseEvent *event)
{
	// Если нажали в пределах поля
	if ((event->y() <= HIGHT) && (event->x() <= WIDE))
	{
		// Нажали правую кнопку отобразили контекстное меню
		if (event->button() == Qt::RightButton)
		{
			// Создаем меню.
			QMenu *contextMenu = new QMenu(tr("Context menu"), this);
		
			// Указали координаты меню.
			m_setPosition.setX(event->pos().x());
			m_setPosition.setY(event->pos().y());

			// Указали название пунктов и какие методы вызывать при нажатии на них.
			QAction* action1 = contextMenu->addAction("Start position", this, SLOT(setStartPosition()));
			QAction* action2 = contextMenu->addAction("Ending position", this, SLOT(setEndingPosition()));

			// Отобразили меню.
			contextMenu->exec(mapToGlobal(event->pos()));
		}
		// Если нажали левой кнопкой то создаем препядствие.
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

// Вычисляем возможный ли это путь (если не пересекает препядствий и не уходит за пределы видимого поля)
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

// Вычислем пересекает ли линия квадрат.
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

// Вычисляем пересекаются ли линии.
bool geneticPathFinder::isLineCrossed(QPointF a, QPointF b, QLineF line)
{
	QPointF c = line.p1();
	QPointF d = line.p2();
	return intersect_1(a.x(), b.x(), c.x(), d.x())
			&& intersect_1(a.y(), b.y(), c.y(), d.y())
			&& area(a, b, c) * area(a, b, d) <= 0
			&& area(c, d, a) * area(c, d, b) <= 0;
}

// Вычисляем площадь в квадрате по треугольника по координатам.
float geneticPathFinder::area(QPointF a, QPointF b, QPointF c)
{
	return (b.x() - a.x()) * (c.y() - a.y()) - (b.y() - a.y()) * (c.x() - a.x());
}

// Вспомогательные расчеты для определения пересечений по методу треугольников.
bool geneticPathFinder::intersect_1(qreal a, qreal b, qreal c, qreal d)
{
	if (a > b)  std::swap(a, b);
	if (c > d)  std::swap(c, d);
	return std::max(a, c) <= std::min(b, d);
}

// Очищаем поле препядствий.
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

// Выисляем находимся ли мы в видимом поле.
bool geneticPathFinder::isInPlace(QPoint point)
{
	return (((point.x() >= 0) && (point.x() <= WIDE)) &&
		((point.y() >= 0) && (point.y() <= HIGHT)));
}

// Дестркутор.
geneticPathFinder::~geneticPathFinder()
{

}