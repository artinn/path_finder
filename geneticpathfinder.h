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
	QTimer* m_timer;			// ������.
	QPoint m_startPosition;		// ��������� �������.
	QPoint m_endPosition;		// �������� �������.
	QPoint m_setPosition;
	QColor m_masterColor;		// ���� �����.
	int m_gensLength;			// ���������� �����.
	int m_populationSize;		// ������ ���������.
	int m_generationNumber;		// ���������� ���������.
	int m_universeSize;			// ������� ���� �����������.
	Generation m_generation;	// ��������� �����.
	Generation m_generationShadow; // ��������� ����� ��������� �����.
	bool* m_universe; // map
	float m_bestPathLength;		// ����� ������� ����.
	Path m_bestPath;			// ������ ����.
	int m_maxGenerations;		// ����������� ���������� ���������.
	int m_mutationPropobility;	// ����������� �������.
	int m_factorMating;			// ����.  �����������.
	bool m_isFindBest;			// ������ �� ����.
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
