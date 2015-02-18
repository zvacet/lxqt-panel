/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2011 Razor team
 * Authors:
 *   Petr Vanek <petr@scribus.info>
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


#include <QToolButton>
#include <QtDebug>
#include <lxqt-globalkeys.h>
#include <QTimer>
#include "desktopswitchbutton.h"
#include <QApplication>

DesktopSwitchButton::DesktopSwitchButton(QWidget * parent, int index, const QString &path, const QString &shortcut, const QString &title)
    : QToolButton(parent)
    , m_shortcut(0)
    , mIndex(index),
      m_shortcutPath(path),
      m_shortcutKey(shortcut)
{
    setText(QString::number(index + 1));
    setCheckable(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    if (!title.isEmpty())
        setToolTip(title);
}

void DesktopSwitchButton::unregisterShortcut()
{
    GlobalKeyShortcut::Client::instance()->removeAction(QString("/desktop_switch/desktop_%1").arg(mIndex + 1));
}

void DesktopSwitchButton::showEvent(QShowEvent *e)
{
    QToolButton::showEvent(e);
    if(NULL == m_shortcut)
    {
        // slow stuff are initialized after the plugin becomes visible
        QTimer::singleShot(500, this, SLOT(registerShortcut()));
    }
}

// Because the initialization of global shortcuts is very slow, we delay it
// until the widget becomes visible.
void DesktopSwitchButton::registerShortcut() {
    qDebug("DesktopSwitchButton::registerShortcut");
    if (!m_shortcutKey.isEmpty())
    {
        QString description = tr("Switch to desktop %1").arg(mIndex + 1);
        QString title = toolTip();
        if (!title.isEmpty())
            description.append(QString(" (%1)").arg(title));

        m_shortcut = GlobalKeyShortcut::Client::instance()->addAction(QString(), m_shortcutPath, description, this);
        if (m_shortcut)
        {
            if (m_shortcut->shortcut().isEmpty())
                m_shortcut->changeShortcut(m_shortcutKey);
            connect(m_shortcut, SIGNAL(activated()), this, SIGNAL(activated()));
        }
    }
    qDebug("Finish DesktopSwitchButton::registerShortcut");
    qApp->processEvents();
}
