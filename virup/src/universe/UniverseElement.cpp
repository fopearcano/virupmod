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

#include "universe/UniverseElement.hpp"

Vector3 UniverseElement::getAbsoluteBBoxCenter() const
{
	return Utils::fromQt(getRelToAbsTransform() * getBoundingBox().mid);
}

QMatrix4x4 UniverseElement::getRelToAbsTransform() const
{
	QMatrix4x4 relToAbsTransform;
	relToAbsTransform.scale(unit);
	relToAbsTransform.translate(properRotation.inverted()
	                            * Utils::toQt(-1.0 * solarsystemPosition));
	relToAbsTransform = transform(referenceFrame, ReferenceFrame::ECLIPTIC)
	                    * properRotation * relToAbsTransform;

	return relToAbsTransform;
}

void UniverseElement::setVisibility(float visibility)
{
	if(this->visibility != visibility)
	{
		this->visibility = visibility;
		emit visibilityChanged(visibility);
	}
}

void UniverseElement::setProperRotationFromCustomZAxis(
    QVector3D const& customZAxis)
{
	if(customZAxis == QVector3D())
	{
		return;
	}

	QVector3D customXAxis(1.f, 0.f, 0.f), customYAxis(0.f, 1.f, 0.f);

	customYAxis
	    = QVector3D::crossProduct(customZAxis.normalized(), customXAxis);
	customXAxis
	    = QVector3D::crossProduct(customYAxis, customZAxis.normalized());

	properRotation.setColumn(0, QVector4D(customXAxis, 0.0));
	properRotation.setColumn(1, QVector4D(customYAxis, 0.0));
	properRotation.setColumn(2, QVector4D(customZAxis, 0.0));
	properRotation.setColumn(3, QVector4D(0.0, 0.0, 0.0, 1.0));
}

QMatrix4x4 const& UniverseElement::equatorialToEcliptic()
{
	static QMatrix4x4 equatorialToEcliptic
	    = QMatrix4x4(1.0, 6.19344636e-05, 2.6982713e-05, 0.0,      //
	                 -6.74978101e-05, 0.91747101, 0.39780267, 0.0, //
	                 -1.01181804e-07, -0.39780266, 0.917471, 0.0,  //
	                 0.0, 0.0, 0.0, 1.0);

	return equatorialToEcliptic;
}

QMatrix4x4 const& UniverseElement::galacticToEcliptic()
{
	static QMatrix4x4 galacticToEcliptic
	    = QMatrix4x4(-0.05494273, 0.49410207, -0.86766607, 0.0,  //
	                 -0.99382033, -0.11100021, -0.00027913, 0.0, //
	                 -0.09644906, 0.86228889, 0.49714731, 0.0,   //
	                 0.0, 0.0, 0.0, 1.0);

	return galacticToEcliptic;
}

QMatrix4x4 UniverseElement::transform(ReferenceFrame from, ReferenceFrame to)
{
	if(to == from)
	{
		return {};
	}
	if(to == ReferenceFrame::ECLIPTIC)
	{
		if(from == ReferenceFrame::EQUATORIAL)
		{
			return equatorialToEcliptic();
		}
		return galacticToEcliptic();
	}
	if(from == ReferenceFrame::ECLIPTIC)
	{
		return transform(to, from).inverted();
	}
	return transform(from, ReferenceFrame::ECLIPTIC)
	       * transform(ReferenceFrame::ECLIPTIC, to);
}

QList<QPair<QString, QWidget*>>
    UniverseElement::getLauncherFields(QWidget* parent, QJsonObject* jsonObj)
{
	QList<QPair<QString, QWidget*>> result;

	auto sbox = new SciDoubleSpinBox(parent);
	QObject::connect(sbox,
	                 static_cast<void (QDoubleSpinBox::*)(double)>(
	                     &QDoubleSpinBox::valueChanged),
	                 [jsonObj](double v) { (*jsonObj)["unit"] = v; });
	sbox->setValue((*jsonObj)["unit"].toDouble(1.0));

	result.append({QObject::tr("Data unit (in kpc):"), sbox});

	auto cbox = new QComboBox(parent);
	QStringList entries({QObject::tr("Equatorial"), QObject::tr("Galactic"),
	                     QObject::tr("Ecliptic")});
	QStringList entriesIds({"equatorial", "galactic", "ecliptic"});
	for(auto const& entry : entries)
	{
		cbox->addItem(entry);
	}
	QObject::connect(cbox, &QComboBox::currentTextChanged,
	                 [jsonObj, entries, entriesIds](QString const& text) {
		                 (*jsonObj)["referenceframe"]
		                     = entriesIds[entries.indexOf(text)];
	                 });
	if(jsonObj->keys().indexOf("referenceframe") >= 0)
	{
		cbox->setCurrentText(entries[entriesIds.indexOf(
		    (*jsonObj)["referenceframe"].toString())]);
	}
	else
	{
		(*jsonObj)["referenceframe"] = entriesIds[0];
	}

	result.append({QObject::tr("Reference frame:"), cbox});

	Vector3 stored((*jsonObj)["solarsyslocalpos"].toObject());
	auto w                                  = new QWidget(parent);
	auto layout                             = new QHBoxLayout(w);
	std::array<SciDoubleSpinBox*, 3> sboxes = {{nullptr, nullptr, nullptr}};
	std::array<QString, 3> componentLabels
	    = {{QObject::tr("x"), QObject::tr("y"), QObject::tr("z")}};
	unsigned int i(0);
	for(auto& sbox : sboxes)
	{
		sbox = new SciDoubleSpinBox(parent);
	}
	for(auto& sbox : sboxes)
	{
		QObject::connect(sbox,
		                 static_cast<void (QDoubleSpinBox::*)(double)>(
		                     &QDoubleSpinBox::valueChanged),
		                 [jsonObj, sboxes](double)
		                 {
			                 (*jsonObj)["solarsyslocalpos"]
			                     = Vector3(sboxes[0]->value(),
			                               sboxes[1]->value(),
			                               sboxes[2]->value())
			                           .getJSONRepresentation();
		                 });
		sbox->setValue(stored[i]);
		layout->addWidget(new QLabel(componentLabels.at(i) + " :", parent));
		layout->addWidget(sbox);
		++i;
	}

	result.append({QObject::tr("Solar System local position:"), w});

	stored          = (*jsonObj)["customzaxis"].toObject();
	w               = new QWidget(parent);
	layout          = new QHBoxLayout(w);
	sboxes          = {{nullptr, nullptr, nullptr}};
	componentLabels = {{QObject::tr("x"), QObject::tr("y"), QObject::tr("z")}};
	i               = 0;
	for(auto& sbox : sboxes)
	{
		sbox = new SciDoubleSpinBox(parent);
	}
	for(auto& sbox : sboxes)
	{
		QObject::connect(sbox,
		                 static_cast<void (QDoubleSpinBox::*)(double)>(
		                     &QDoubleSpinBox::valueChanged),
		                 [jsonObj, sboxes](double)
		                 {
			                 (*jsonObj)["customzaxis"]
			                     = Vector3(sboxes[0]->value(),
			                               sboxes[1]->value(),
			                               sboxes[2]->value())
			                           .getJSONRepresentation();
		                 });
		sbox->setValue(stored[i]);
		layout->addWidget(new QLabel(componentLabels.at(i) + " :", parent));
		layout->addWidget(sbox);
		++i;
	}

	result.append({QObject::tr("Custom z-axis:"), w});

	sbox = new SciDoubleSpinBox(parent);
	QObject::connect(sbox,
	                 static_cast<void (QDoubleSpinBox::*)(double)>(
	                     &QDoubleSpinBox::valueChanged),
	                 [jsonObj](double v) { (*jsonObj)["brightnessmul"] = v; });
	sbox->setValue((*jsonObj)["brightnessmul"].toDouble(1.0));

	result.append({QObject::tr("Brightness multiplier:"), sbox});
	return result;
}

bool& UniverseElement::useBrightnessMultiplier()
{
	static bool useBrightnessMultiplier(true);
	return useBrightnessMultiplier;
}

void UniverseElement::getModelAndCampos(Camera const& camera, QMatrix4x4& model,
                                        QVector3D& campos)
{
	auto relToAbsTransform(getRelToAbsTransform());
	model = camera.dataToWorldTransform() * relToAbsTransform;

	campos
	    = relToAbsTransform.inverted() * Utils::toQt(camera.getTruePosition());
}
