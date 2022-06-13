/*
    Copyright (C) 2022 Florian Cabot <florian.cabot@hotmail.fr>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef RENDERINGWINDOW_HPP
#define RENDERINGWINDOW_HPP

#include <QMatrix4x4>
#include <QSettings>
#include <QWindow>

#include "InputManager.hpp"
#include "PythonQtHandler.hpp"

class RenderingWindow : public QWindow
{
	Q_OBJECT
	/**
	 * @brief Wether the window is displayed in full screen or not.
	 *
	 * @accessors isFullscreen(), setFullscreen()
	 */
	Q_PROPERTY(bool fullscreen READ isFullscreen WRITE setFullscreen)
	/**
	 * @brief Horizontal angle shift in degrees compared to normal camera
	 * orientation.
	 *
	 * @accessors getHorizontalAngleShift(), setHorizontalAngleShift()
	 */
	Q_PROPERTY(double horizontalAngleShift READ getHorizontalAngleShift WRITE
	               setHorizontalAngleShift)
	/**
	 * @brief Vertical angle shift in degrees compared to normal camera
	 * orientation.
	 *
	 * @accessors getVerticalAngleShift(), setVerticalAngleShift()
	 */
	Q_PROPERTY(double verticalAngleShift READ getVerticalAngleShift WRITE
	               setVerticalAngleShift)
  public:
	struct Parameters
	{
		unsigned int width          = 1500;
		unsigned int height         = 800;
		bool fullscreen             = false;
		QString screenname          = "";
		double horizontalAngleShift = 0.0;
		double verticalAngleShift   = 0.0;
		bool forceleft              = false;
		bool forceright             = false;

		QString toStr() const
		{
			QStringList resultList;
			resultList << QString::number(width);
			resultList << QString::number(height);
			resultList << (fullscreen ? "true" : "false");
			resultList << screenname;
			resultList << QString::number(horizontalAngleShift);
			resultList << QString::number(verticalAngleShift);
			resultList << (forceleft ? "true" : "false");
			resultList << (forceright ? "true" : "false");
			return resultList.join(';');
		};
		void fromStr(QString const& str)
		{
			QStringList strList(str.split(';'));
			width      = strList.size() >= 1 ? strList[0].toUInt() : 1920;
			height     = strList.size() >= 2 ? strList[1].toUInt() : 1080;
			fullscreen = strList.size() >= 3 ? strList[2] == "true" : false;
			screenname = strList.size() >= 4 ? strList[3] : "";
			horizontalAngleShift
			    = strList.size() >= 5 ? strList[4].toDouble() : 0.0;
			verticalAngleShift
			    = strList.size() >= 6 ? strList[5].toDouble() : 0.0;
			forceleft  = strList.size() >= 7 ? strList[6] == "true" : false;
			forceright = strList.size() >= 8 ? strList[7] == "true" : false;
		}
	};

	RenderingWindow(unsigned int id = 0);
	/**
	 * @getter{fullscreen}
	 */
	bool isFullscreen() const;
	/**
	 * @setter{fullscreen, fullscreen}
	 */
	void setFullscreen(bool fullscreen);
	/**
	 * @getter{horizontalAngleShift}
	 */
	double getHorizontalAngleShift() const;
	/**
	 * @getter{verticalAngleShift}
	 */
	double getVerticalAngleShift() const;
	/**
	 * @setter{horizontalAngleShift}
	 */
	void setHorizontalAngleShift(double angleShift);
	/**
	 * @setter{verticalAngleShift}
	 */
	void setVerticalAngleShift(double angleShift);
	/** @brief Transformation matrix combining both @ref horizontalAngleShift
	 * and @ref verticalAngleShift
	 */
	QMatrix4x4 getAngleShiftMatrix() const { return angleShiftMat; };
	bool isForcedLeft() const { return params.forceleft; };
	bool isForcedRight() const { return params.forceright; };

  public slots:
	/**
	 * @toggle{fullscreen}
	 */
	void toggleFullscreen();

  protected:
	/**
	 * @brief Captures a Qt keyboard press event.
	 *
	 * See <a
	 * href="https://doc.qt.io/qt-5/qwidget.html#keyPressEvent">QWidget::keyPressEvent</a>.
	 * Make sure you call @ref AbstractMainWin#keyPressEvent if you override
	 * it.
	 *
	 * Also calls the Python function @e keyPressEvent.
	 */
	virtual void keyPressEvent(QKeyEvent* e) override;
	/**
	 * @brief Captures a Qt keyboard release event.
	 *
	 * See <a
	 * href="https://doc.qt.io/qt-5/qwidget.html#keyReleaseEvent">QWidget::keyReleaseEvent</a>.
	 * Make sure you call @ref AbstractMainWin#keyReleaseEvent if you override
	 * it.
	 *
	 * Also calls the Python function @e keyReleaseEvent.
	 */
	virtual void keyReleaseEvent(QKeyEvent* e) override;
	/**
	 * @brief Captures a @e BaseInputManager Action triggered by a QKeySequence.
	 *
	 * For a key press, @p pressed is true, for a key release, it is false.
	 */
	virtual void actionEvent(BaseInputManager::Action a, bool pressed);
	virtual void resizeEvent(QResizeEvent* ev) override;

  private:
	const InputManager inputManager;

	QMatrix4x4 angleShiftMat;
	const unsigned int id = 0;
	Parameters params;

	void updateAngleShiftMat();
	void saveParams() const;
};

#endif // RENDERINGWINDOW_HPP
