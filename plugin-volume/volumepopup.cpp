/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Razor - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2012 Razor team
 * Authors:
 *   Johannes Zellner <webmaster@nebulon.de>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "volumepopup.h"

#include "pulseaudiodevice.h"

#include <qtxdg/xdgicon.h>

#include <QtGui/QSlider>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtCore/QProcess>

VolumePopup::VolumePopup(QWidget* parent):
    QWidget(parent,  Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint | Qt::X11BypassWindowManagerHint),
    m_pos(0,0),
    m_anchor(Qt::TopLeftCorner),
    m_device(0)
{
    m_volumeSlider = new QSlider(Qt::Vertical, this);
    m_mixerButton = new QToolButton(this);
    m_mixerButton->setIcon(XdgIcon::fromTheme(QStringList() << "kmix"));

    m_volumeSlider->setSingleStep(m_volumeSlider->pageStep());

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setSpacing(0);
    l->setMargin(2);

    l->addWidget(m_volumeSlider, 0, Qt::AlignHCenter);
    l->addWidget(m_mixerButton);

    connect(m_mixerButton, SIGNAL(clicked()), this, SIGNAL(launchMixer()));
    connect(m_volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(handleSliderValueChanged(int)));
}

VolumePopup::~VolumePopup()
{
}

void VolumePopup::enterEvent(QEvent *event)
{
    emit mouseEnter();
}

void VolumePopup::leaveEvent(QEvent *event)
{
    emit mouseExit();
}

void VolumePopup::handleSliderValueChanged(int value)
{
    if (!m_device)
        return;

    m_device->setVolume(value);
}

void VolumePopup::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    realign();
}

void VolumePopup::open(QPoint pos, Qt::Corner anchor)
{
    m_pos = pos;
    m_anchor = anchor;
    realign();
    show();
}

void VolumePopup::handleWheelEvent(QWheelEvent *event)
{
    m_volumeSlider->event(reinterpret_cast<QEvent*>(event));
}

void VolumePopup::setDevice(PulseAudioDevice *device)
{
    if (device == m_device)
        return;

    // disconnect old device
    if (m_device) {
        disconnect(m_device, SIGNAL(volumeChanged(int)), m_volumeSlider, SLOT(setValue(int)));
        disconnect(m_volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(handleSliderValueChanged(int)));
    }

    m_device = device;
    m_volumeSlider->setValue(m_device->volume());

    connect(m_device, SIGNAL(volumeChanged(int)), m_volumeSlider, SLOT(setValue(int)));
    connect(m_volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(handleSliderValueChanged(int)));

    emit deviceChanged();
}

void VolumePopup::realign()
{
    QRect rect;
    rect.setSize(sizeHint());
    switch (m_anchor)
    {
    case Qt::TopLeftCorner:
        rect.moveTopLeft(m_pos);
        break;

    case Qt::TopRightCorner:
        rect.moveTopRight(m_pos);
        break;

    case Qt::BottomLeftCorner:
        rect.moveBottomLeft(m_pos);
        break;

    case Qt::BottomRightCorner:
        rect.moveBottomRight(m_pos);
        break;

    }

    QRect screen = QApplication::desktop()->availableGeometry(m_pos);

    if (rect.right() > screen.right())
        rect.moveRight(screen.right());

    if (rect.bottom() > screen.bottom())
        rect.moveBottom(screen.bottom());

    move(rect.topLeft());
}
