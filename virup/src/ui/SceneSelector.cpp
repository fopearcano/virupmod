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
    : VIRUPDialog3D({0.f, 0.f})
    , animator(animator)
    , animationTimeSlider(Qt::Horizontal, this)
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
	b->setText(tr("PAUSE"));
	connect(b, &QPushButton::clicked, this,
	        [&animator]() { animator.pause(); });
	hl->addWidget(b);

	b = new QPushButton(w);
	b->setText(tr("RESUME"));
	connect(b, &QPushButton::clicked, this, [&animator]() { animator.play(); });
	connect(&animator, &Animator::paused, this,
	        [b]() { b->setDisabled(false); });
	connect(&animator, &Animator::resumed, this,
	        [b]() { b->setDisabled(true); });
	connect(&animator, &Animator::stopped, this,
	        [b]() { b->setDisabled(true); });
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

	hl->addWidget(new QLabel(tr("User height : ")));
	auto sb = new QDoubleSpinBox(this);
	sb->setValue(animator.getPersonHeight());
	connect(sb,
	        static_cast<void (QDoubleSpinBox::*)(double)>(
	            &QDoubleSpinBox::valueChanged),
	        [&animator](double v) { animator.setPersonHeight(v); });
	connect(&animator, &Animator::personHeightChanged,
	        [sb](float v)
	        {
		        QSignalBlocker blocker{sb};
		        sb->setValue(v);
	        });
	hl->addWidget(sb);

	animationTimeSlider.setMaximum(1000);
	connect(&animationTimeSlider, &QSlider::sliderPressed,
	        [&animator]() { animator.pause(); });
	connect(&animationTimeSlider, &QSlider::sliderReleased,
	        [&animator]() { animator.play(); });
	connect(&animationTimeSlider, &TimeProgressSlider::userPickedTime,
	        [&animator](int value)
	        { animator.setWholeAnimationPercentage(value / 10.f); });
	layout->addWidget(&animationTimeSlider);

	layout->addWidget(new QLabel("Scenes :"));

	auto buttonsWidget = new QWidget(this);
	layout->addWidget(buttonsWidget);
	buttonsWidget->setLayout(&buttonsLayout);
	connect(&animator, &Animator::transitionsModified, this,
	        &SceneSelector::updateButtons);

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
	animationTimeSlider.updateTime(
	    static_cast<int>(10 * animator.getWholeAnimationPercentage()));

	int id(animator.getCurrentTransitionId());
	QString currentScene
	    = id < 0 ? "" : animator.getTransitions()[id].getName();
	for(auto button : buttons)
	{
		QPalette pal = button->palette();
		if(button->text() == currentScene)
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

void SceneSelector::updateButtons()
{
	for(auto button : buttons)
	{
		delete button;
		buttons.clear();
	}

	auto const& transitions(animator.getTransitions());
	for(unsigned int i(0); i < transitions.size(); ++i)
	{
		if(transitions[i].getName().isEmpty())
		{
			continue;
		}
		auto button = new QPushButton(transitions[i].getName());
		connect(button, &QPushButton::clicked, this,
		        [this, i]() { animator.setTransition(i); });
		button->setFocusPolicy(Qt::NoFocus);
		buttonsLayout.addWidget(button);
		buttons.push_back(button);
	}
}
