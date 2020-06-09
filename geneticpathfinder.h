#ifndef GENETICPATHFINDER_H
#define GENETICPATHFINDER_H

#include <QtWidgets/QMainWindow>
#include "ui_geneticpathfinder.h"
#include "Generation.h"

class geneticPathFinder : public QMainWindow
{
	Q_OBJECT

public:
	geneticPathFinder(QWidget *parent = 0);
	~geneticPathFinder();
private:
	Ui::geneticPathFInderClass ui;
	void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	bool isLineCrossed(QPointF lhs, QPointF rhs, QLineF line);
	Path createRandomPath();
	float distance(QPoint l, QPoint r);
	Path crossingOver(Path pathLhs, Path pathRhs);
	void setFirstGeneration();
	void paintGrid(QPainter &p);
	bool isPosiblePath(QLineF line);
	bool isCrossing(QRectF rect, QLineF line);
	bool isInPlace(QPoint point);
	float area(QPointF a, QPointF b, QPointF c);
	bool intersect_1(qreal a, qreal b, qreal c, qreal d);
private:
	QTimer* m_timer;			// Таймер.
	QPoint m_startPosition;		// Стартовая позиция.
	QPoint m_endPosition;		// Конечная позиция.
	QPoint m_setPosition;
	QColor m_masterColor;		// Цвет путей.
	int m_gensLength;			// Количество генов.
	int m_populationSize;		// Размер популяции.
	int m_generationNumber;		// Количество поколений.
	int m_universeSize;			// Битовое поел препядствий.
	Generation m_generation;	// Поколение путей.
	Generation m_generationShadow; // Временная копия поколения путей.
	bool* m_universe; // map
	float m_bestPathLength;		// Длина лучшего пути.
	Path m_bestPath;			// Лучший путь.
	int m_maxGenerations;		// Максимально количсевто поколений.
	int m_mutationPropobility;	// Вероятность мутации.
	int m_factorMating;			// Коэф.  скрещивания.
	bool m_isFindBest;			// Найден ли путь.
private slots:
	void on_start_clicked();
	void on_stop_clicked();
	void on_resetButton_clicked();
	void on_hideField_clicked();
	void nextStep();
	void setInterval(int interval);
	void setGensLength(int length);
	void setPopulationSize(int populationSize);
	void setStartPosition();
	void setEndingPosition();
	void setMaxGenerations(int maxGenerations);
	void setMutation(int mutationPropobility);
	void setFactorMating(int factorMating);
	void paintUniverse(QPainter &p);

signals:
	//when one of the cell has been changed,emit this signal to lock the universeSize
	void environmentChanged(bool ok);
};

#endif // GENETICPATHFINDER_H
