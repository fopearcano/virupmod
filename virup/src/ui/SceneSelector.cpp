/*
    Copyright (C) 2021 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "ui/SceneSelector.hpp"

SceneSelector::SceneSelector(Animator& animator)
    : animator(animator)
    , slider(Qt::Horizontal, this)
{
	show();
	setWindowTitle(tr("VIRUP Scenes"));
	setAttribute(Qt::WA_TransparentForMouseEvents, true);

	auto layout = new QVBoxLayout(this);

	auto w = new QWidget(this);
	layout->addWidget(w);
	auto hl = new QHBoxLayout(w);

	auto b = new QPushButton(w);
	b->setText(tr("RESTART"));
	connect(b, &QPushButton::clicked, this,
	        [&animator]() { animator.restart(); });
	hl->addWidget(b);

	b = new QPushButton(w);
	b->setText(tr("STOP"));
	connect(b, &QPushButton::clicked, this, [&animator]() { animator.stop(); });
	hl->addWidget(b);

	hl->addWidget(new QLabel(tr("EN (on) / JP (off) : ")));

	auto cb = new QCheckBox(this);
	cb->setCheckState(Qt::Checked);
	connect(cb, &QCheckBox::stateChanged,
	        [this](int state) { english = state != Qt::Unchecked; });
	hl->addWidget(cb);

	b = new QPushButton(w);
	b->setText(tr("Stop Voiceover"));
	connect(b, &QPushButton::clicked, this,
	        [&animator]() { animator.stopVoiceover(); });
	hl->addWidget(b);

	slider.setMaximum(1000);
	connect(&slider, &QSlider::sliderPressed,
	        [this]() { animateSlider = false; });
	connect(&slider, &QSlider::sliderReleased,
	        [this]() { animateSlider = true; });
	connect(&slider, &QSlider::sliderMoved, [&animator](int value) {
		animator.setWholeAnimationPercentage(value / 10.f);
	});
	layout->addWidget(&slider);

	layout->addWidget(new QLabel("Scenes :"));

	QStringList scenes = {"0:International Space Station",
	                      "1:Earth",
	                      "2:Moon",
	                      "3:Phobos",
	                      "4:Solar System",
	                      "5:AGORA",
	                      "6:Local Group",
	                      "7:IllustrisTNG",
	                      "8:SDSS"};
	for(int i(0); i < scenes.size(); ++i)
	{
		auto button = new QPushButton(scenes[i]);
		connect(button, &QPushButton::clicked, this,
		        [&animator, i]() { animator.setTransition(10 + 2 * i); });
		button->setFocusPolicy(Qt::NoFocus);
		layout->addWidget(button);
		buttons.push_back(button);
	}
	// layout->addWidget(new QLabel("Options :"));
	transitionsButton
	    = new QPushButton("Toggle transitions (only if user is sick, can "
	                      "introduce problems !)");
	connect(transitionsButton, &QPushButton::clicked, this,
	        [&animator]() { animator.toggleAnimations(); });
	transitionsButton->setFocusPolicy(Qt::NoFocus);
	transitionsButton->hide();
	layout->addWidget(transitionsButton);

	installEventFilters();
}

void SceneSelector::update()
{
	if(animateSlider)
	{
		slider.setValue(
		    static_cast<int>(10 * animator.getWholeAnimationPercentage()));
	}
	int currentScene((animator.getCurrentTransitionId() - 10) / 2);
	for(int i(0); i < static_cast<int>(buttons.size()); ++i)
	{
		auto button  = buttons[i];
		QPalette pal = button->palette();
		if(i == currentScene)
		{
			pal.setColor(QPalette::Button, QColor(Qt::green));
		}
		else
		{
			pal.setColor(QPalette::Button, QColor(255, 128, 128));
		}
		button->setAutoFillBackground(true);
		button->setPalette(pal);
		button->update();
	}
	QPalette pal = transitionsButton->palette();
	bool animationDisabled(
	    PythonQtHandler::getVariable("disableanimations").toBool());
	if(animationDisabled)
	{
		transitionsButton->setText("Transitions : DISABLED");
		pal.setColor(QPalette::Button, QColor(255, 128, 128));
	}
	else
	{
		transitionsButton->setText("Transitions : ENABLED");
		pal.setColor(QPalette::Button, QColor(Qt::green));
	}
	transitionsButton->setAutoFillBackground(true);
	transitionsButton->setPalette(pal);
	transitionsButton->update();
}
