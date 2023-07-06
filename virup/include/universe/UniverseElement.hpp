/*
    Copyright (C) 2020 Florian Cabot <florian.cabot@hotmail.fr>

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

#ifndef UNIVERSEELEMENT_HPP
#define UNIVERSEELEMENT_HPP

#include <QComboBox>
#include <QLabel>

#include "Camera.hpp"
#include "ToneMappingModel.hpp"
#include "gui/ColorSelector.hpp"
#include "gui/PathSelector.hpp"
#include "gui/SciDoubleSpinBox.hpp"
#include "math/Vector3.hpp"

/*! @brief Represents a graphical element of the Universe, usually the
 * visualization of a specific dataset or cosmological simulation.
 *
 * Any universe graphical element should inherit from it, so that it can be
 * inserted in the collection of elements of @ref Universe, that will manage it.
 */
class UniverseElement : public QObject
{
	Q_OBJECT

  public:
	/*! @brief Defines a universal reference frame.
	 *
	 * All these reference frames are centered on Earth, are orthonormal of
	 * arbitrary unit and right-hand defined. They differ only by orientation.
	 *
	 * VIRUP's absolute reference frame is @ref ECLIPTIC.
	 */
	enum class ReferenceFrame
	{
		/*! @brief Oriented so that the xOy plane is aligned with the equator. x
		 * points towards the First Point of Aries and z along the north pole.
		 */
		EQUATORIAL,
		/*! @brief Oriented so that the xOy plane is aligned with the ecliptic.
		 * x points towards the First Point of Aries and z points in the same
		 * direction as the north pole.
		 */
		ECLIPTIC,
		/*! @brief Oriented so that the xOy plane is aligned with the Milky Way
		 * plane. x points towards the center of the Milky Way and z along its
		 * north pole.
		 */
		GALACTIC,
	};

	/*! @brief Default constructor. See members for default values.
	 */
	UniverseElement() = default;
	/*! @brief Get the element Json representation.
	 */
	virtual QJsonObject getJson() const;
	/*! @brief Get the element Json representation.
	 */
	virtual void setJson(QJsonObject const& json);
	/*! @brief Returns the bounding box of the data in its relative reference
	 * frame.
	 */
	virtual BBox getBoundingBox() const = 0;
	/*! @brief Returns the bounding box center's coordinates in the global
	 * reference frame (ECLIPTIC, 1kpc).
	 */
	Vector3 getAbsoluteBBoxCenter() const;
	/*! @brief Returns the transformation matrix from the relative reference
	 * frame of the data to the global reference frame of VIRUP (ECLIPTIC,
	 * 1kpc).
	 */
	QMatrix4x4 getRelToAbsTransform() const;
	/*! @brief Returns the visibility of this element.
	 *
	 * Visibility is a multiplier on the luminance of the rendered element.
	 */
	float getVisibility() const { return visibility; };
	/*! @brief Sets the visibility of this element.
	 *
	 * Visibility is a multiplier on the luminance of the rendered element.
	 *
	 * @param visibility New value for visibility.
	 */
	void setVisibility(float visibility);
	/*! @brief Override this method to execute data updates before render.
	 *
	 * @param camera The current @ref Camera used for rendering.
	 */
	virtual void update(Camera const& /*camera*/){};
	/*! @brief Implement this method to render this element.
	 *
	 * @param camera The current @ref Camera used for rendering.
	 * @param tmm The current @ref ToneMappingModel used for rendering (can be
	 * ignored for physical-based rendering, it will be applied by the engine).
	 */
	virtual void render(Camera const& camera, ToneMappingModel const& tmm) = 0;
	/*! @brief Default destructor.
	 */
	virtual ~UniverseElement() = default;

	/*! @brief Adjusts the physical brightness of an element.
	 *
	 * Usually, the data is inputted in VIRUP with real physical brightnesses,
	 * so this value should remain as 1 for realistic rendering. (Adjust
	 * exposure to see dim data.)
	 *
	 * If however you want very differently lit objects to appear at the same
	 * time on a similar brightness scale, use this member to boost the
	 * brightness of the dimmest element. For example, if one wants to see the
	 * lit side of the Earth at the same time as the Milky Way, one should apply
	 * a brightness multiplier on the Milky Way to make it visible during
	 * daylight.
	 */
	float brightnessMultiplier = 1.f;

	QString name;
	/*! @brief Spatial unit of the day in kiloparsecs (kpc).
	 *
	 * For example, if the data uses parsecs as a spatial unit, set this member
	 * to 1e-3.
	 */
	double unit = 1.0;
	/*! @brief Position of the Solar System within the data in its relative
	 * coordinate system.
	 */
	Vector3 solarsystemPosition = Vector3(0.0, 0.0, 0.0);
	/*! @brief Relative reference frame of the spatial data.
	 */
	ReferenceFrame referenceFrame = ReferenceFrame::EQUATORIAL;
	QMatrix4x4 properRotation;

	void setProperRotationFromCustomZAxis(QVector3D const& customZAxis);

	/*! @brief Returns a transformation matrix from the EQUATORIAL @ref
	 * ReferenceFrame to the ECLIPTIC @ref ReferenceFrame.
	 */
	static QMatrix4x4 const& equatorialToEcliptic();
	/*! @brief Returns a transformation matrix from the GALACTIC @ref
	 * ReferenceFrame to the ECLIPTIC @ref ReferenceFrame.
	 */
	static QMatrix4x4 const& galacticToEcliptic();
	/*! @brief Returns a transformation matrix between two @ref ReferenceFrame .
	 *
	 * @param from Initial reference frame.
	 * @param to Final reference frame.
	 */
	static QMatrix4x4 transform(ReferenceFrame from, ReferenceFrame to);

	static QList<QPair<QString, QWidget*>>
	    getLauncherFields(QWidget& parent, QJsonObject& jsonObj);

	static bool& useBrightnessMultiplier();

  signals:
	/*! @brief Emitted when visibility is changed.
	 *
	 * @param newValue The value visibility was changed to.
	 */
	void visibilityChanged(float newValue);

  protected:
	/*! @brief Updates the model and camera position for the element based on
	 * the camera.
	 *
	 * Usually called within @ref update to prepare for rendering.
	 *
	 * @param camera The current @ref Camera used for rendering.
	 * @param model The model matrix to update. This is the complete model
	 * matrix to use for this element rendering.
	 * @param campos The camera position to update. This camera position is
	 * relative to the data, so it can be used without transforming the data
	 * first for distance computations for example.
	 */
	void getModelAndCampos(Camera const& camera, QMatrix4x4& model,
	                       QVector3D& campos);

  private:
	float visibility = 0.f;
};

#endif // UNIVERSEELEMENT_HPP
