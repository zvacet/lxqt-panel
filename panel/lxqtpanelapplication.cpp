/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
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


#include "lxqtpanelapplication.h"
#include "lxqtpanel.h"
#include <LXQt/Settings>
#include <QtDebug>
#include <QUuid>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QScreen>
#include <xcb/randr.h>
#include <xcb/xcb.h>
#include <QX11Info>
#endif

#include <X11/Xlib.h>

LxQtPanelApplication::LxQtPanelApplication(int& argc, char** argv, const QString &configFile)
    : LxQt::Application(argc, argv)
{
    if (configFile.isEmpty())
        mSettings = new LxQt::Settings("panel", this);
    else
        mSettings = new LxQt::Settings(configFile, QSettings::IniFormat, this);

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    checkXRandR();

    qApp->installNativeEventFilter(this);
#endif

    QStringList panels = mSettings->value("panels").toStringList();

    if (panels.isEmpty())
    {
        panels << "panel1";
    }

    Q_FOREACH(QString i, panels)
    {
        addPanel(i);
    }
}


LxQtPanelApplication::~LxQtPanelApplication()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    qApp->removeNativeEventFilter(this);
#endif
    qDeleteAll(mPanels);
}

void LxQtPanelApplication::addNewPanel()
{
    QString name("panel_" + QUuid::createUuid().toString());
    addPanel(name);
    
    QStringList panels = mSettings->value("panels").toStringList();
    panels << name;
    mSettings->setValue("panels", panels);
}

void LxQtPanelApplication::addPanel(const QString &name)
{
    LxQtPanel *panel = new LxQtPanel(name);
    mPanels << panel;
    connect(panel, SIGNAL(deletedByUser(LxQtPanel*)),
            this, SLOT(removePanel(LxQtPanel*)));
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)

void LxQtPanelApplication::checkXRandR()
{
    has_randr_extension = false;
    xrandr_first_event = 0;
    xcb_query_extension_cookie_t cookie = xcb_query_extension(QX11Info::connection(), 5, "RANDR");
    xcb_generic_error_t* error = NULL;
    xcb_query_extension_reply_t* reply = xcb_query_extension_reply(QX11Info::connection(), cookie, &error);
    if(!error && reply)
    {
        has_randr_extension = true;
        xrandr_first_event = reply->first_event;
    }
    if(reply)
        free(reply);
    if(error)
        free(error);
}

void LxQtPanelApplication::handleXRandRChange()
{
    // The XCB platform plugin will call QXcbConnection::updateScreens() and
    // this will destroy screens that do not exist anymore.
    // Existing QWindow belonging to the destroyed screens will be destroyed and
    // re-created on the primary screen by Qt.
    // Our native event filter intercept this XRandR event before Qt handle it.
    // By detecting screens which no longer exists, we can move the windows in it
    // to another screen which is not changed. This can prevent Qt from destroying
    // the window and hence can workaround Qt bug # 40681
    // More details: https://bugreports.qt-project.org/browse/QTBUG-40681
  
    // find out screens which no longer exist
    Q_FOREACH(QScreen* screen, screens())
        qDebug() << "current screen:" << screen->name();
  
    const xcb_setup_t* setup = xcb_get_setup(QX11Info::connection());
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
    while (it.rem)
    {
        xcb_screen_t *xcbScreen = it.data;
        qDebug() << "XCB screen" << it.rem << xcbScreen;

        xcb_generic_error_t *error = NULL;
        xcb_randr_get_screen_resources_cookie_t resourcesCookie = xcb_randr_get_screen_resources(QX11Info::connection(), xcbScreen->root);
        xcb_randr_get_screen_resources_reply_t *resources = xcb_randr_get_screen_resources_reply(QX11Info::connection(), resourcesCookie, &error);

        xcb_timestamp_t timestamp = resources->config_timestamp;
        int outputCount = xcb_randr_get_screen_resources_outputs_length(resources);
        xcb_randr_output_t *outputs = xcb_randr_get_screen_resources_outputs(resources);
        qDebug() << outputCount;

        for (int i = 0; i < outputCount; i++) {
            xcb_randr_get_output_info_reply_t *output = xcb_randr_get_output_info_reply(QX11Info::connection(),
                        xcb_randr_get_output_info_unchecked(QX11Info::connection(), outputs[i], timestamp), NULL);
            if (output == NULL)
                continue;
            QString outputName = QString::fromUtf8((const char*)xcb_randr_get_output_info_name(output),
                                        xcb_randr_get_output_info_name_length(output));
            qDebug() << outputName << " connected:" << (output->crtc != XCB_NONE);
        }
        xcb_screen_next(&it);
    }
}

// Qt5 uses native event filter
bool LxQtPanelApplication::nativeEventFilter(const QByteArray & eventType, void * message, long * result)
{
    if(eventType != "xcb_generic_event_t") // We only want to handle XCB events
        return false;
    xcb_generic_event_t* event = reinterpret_cast<xcb_generic_event_t*>(message);
    int event_type = event->response_type & ~0x80;

    if(has_randr_extension && event_type == xrandr_first_event + XCB_RANDR_SCREEN_CHANGE_NOTIFY)
    {
        xcb_randr_screen_change_notify_event_t *change_event = (xcb_randr_screen_change_notify_event_t *)event;
        handleXRandRChange();
    }

    foreach(LxQtPanel *i, mPanels)
        i->x11EventFilter(event);
    return false;
}

#else

// This X11 event is no longer supported in Qt5
bool LxQtPanelApplication::x11EventFilter(XEvent * event)
{
    foreach(LxQtPanel *i, mPanels)
        i->x11EventFilter(event);
    return false;
}

#endif // Qt5

void LxQtPanelApplication::removePanel(LxQtPanel* panel)
{
    Q_ASSERT(mPanels.contains(panel));

    mPanels.removeAll(panel);
    
    QStringList panels = mSettings->value("panels").toStringList();
    panels.removeAll(panel->name());
    mSettings->setValue("panels", panels);
    
    panel->deleteLater();
}
