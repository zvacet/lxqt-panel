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



LxQtPanelApplication::LxQtPanelApplication(int& argc, char** argv, const QString &configFile)
    : LxQt::Application(argc, argv),
      isFirst(1)
{
    if (configFile.isEmpty())
        mSettings = new LxQt::Settings("panel", this);
    else
        mSettings = new LxQt::Settings(configFile, QSettings::IniFormat, this);

    QStringList panels = mSettings->value("panels").toStringList();

    if (panels.isEmpty())
    {
        panels << "panel1";
    }

//    XChangeWindowAttributes(xmap.display,QX11Info::appRootWindow(),0,);


//    XSetWindowAttributes setwinattr;
//    unsigned long valuemask;
//    /* (Must open display, create window) */
//    valuemask = CWBackPixel | CWBorderPixel;
//    valuemask = panels
//    setwinattr.background_pixel = WhitePixel(display, screen_num);
//    setwinattr.border_pixel = BlackPixel(display, screen_num);
//    XChangeWindowAttributes(XOpenDisplay(NULL), QX11Info::appRootWindow(), valuemask, &setwinattr);


    //event_mask = StructureNotifyMask | MapNotify | MapRequest;
    //XSelectInput(XOpenDisplay(NULL),QX11Info::appRootWindow(),event_mask);

    Q_FOREACH(QString i, panels)
    {
        addPanel(i);
        qDebug() << "///////////////////////////////////// PANEL ADDED" << i << XOpenDisplay(NULL);
    }


}


LxQtPanelApplication::~LxQtPanelApplication()
{
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

bool LxQtPanelApplication::x11EventFilter(XEvent * event)
{
   // XCreateWindowEvent asd1 = event->xcreatewindow;

    //XMapEvent xmap = event->xmap;
    //XMappingEvent xmapping = event->xmapping;
    //XUnmapEvent xunmapping = event->xunmap;
  //  Window xwin = xmap.window;

    //xdisp = xmap.display;
    //XAnyEvent xany = event->xany;

    window_name = "asd";
    int a;
//    static char *window_id_format = "0x%lx";
    XWindowAttributes attr;

    switch (event->type)
    {
        case MotionNotify:
break;
        xmot = event->xmotion;

         if (isFirst) {
          // XSelectInput(xmot.display,xmot.window,event_mask);
            isRealMain = xmot.window;
            isFirst = 0;
            qDebug () << " OK OK OK O KO KO KO K OK OK OK OK OK O K REAL MAIN ==========" << isRealMain;
          }


          if (!isMain)
          {
            isMain = xmot.window;
          }



          if (xmot.window != isMain)
          {
              qDebug () << "\n\n";

              qDebug() << " --- Window changed from " << isMain << "to " << xmot.window << " -- " << *window_name << " / " << xmot.display;
                a = XFetchName(xmot.display,xmot.window,&window_name);
               qDebug() << a << "ok" << window_name << " -- " << xparent;
               isMain = xmot.window;

               qDebug() << "Rootwindow: " << QX11Info::appRootWindow();

               //XQueryTree(xmot.display, QX11Info::appRootWindow(), &r, &p, &kids, &numkids);
               XQueryTree(xmot.display, isRealMain, &r, &p, &kids, &numkids);
               qDebug() << "1 - kids: " << numkids << "parent: " << p << "root: " << r;
               XQueryTree(xmot.display, p, &r, &p, &kids, &numkids);
               qDebug() << "2 - kids: " << numkids << "parent: " << p << "root: " << r;


               unsigned int mapped;

//mPanels

               //xf = xfitMan();
//               QList<Window> winList = xf.getClientList();
//               int current = winList.indexOf(xf.getActiveAppWindow());
//               int delta = event->delta() < 0 ? 1 : -1;

//               for (int ix = current + delta; 0 <= ix && ix < winList.size(); ix += delta)
//               {
//                   Window window = winList.at(ix);
//                   if (xf.acceptWindow(window) && windowOnActiveDesktop(window))
//                   {
//                       xf.raiseWindow(window);
//                       break;
//                   }
//               }




               for (int i = 0; i < numkids; ++i)
               {
                   XGetWindowAttributes(xmot.display, kids[i], &attr);

                   if (attr.map_state == IsViewable) {
                       ++mapped;


                       a = XFetchName(xmot.display, kids[i], &win_name);
                       qDebug() << "AAA " << xmot.root << " - " << xmot.window << "-" << xmot.subwindow << "/" << kids[i] << " -- " << win_name;
                       qDebug() << "bbb" << attr.root << " - " << attr.depth << " - " << attr.map_installed << " - " << attr.map_state << " - " << attr.width;
                       //printf("%s",win_name);
                   }
               }




          }


         // XGetWindowAttributes(xmot.display,xmot.window)


        //   qt_x11_getX11InfoForWindow()
        //  qt_x11_getX11InfoForWindow(QX11Info * xinfo, const QX11WindowAttributes &a);

          //XContextualDrawing
            //      XListHosts()
              //    xNumChildren


           // qDebug() << " --- MAIN WINDOW ASSIGNED ----------------------------------" << *event->pad << " --- " << isMain << xdisp;
            //XSelectInput(mDisplay, mInterClientCommunicationWindow, SubstructureNotifyMask);
           // XSelectInput(xdisp,xany.window,SubstructureNotifyMask);




//          if (xany.window == *isMain)
          {
//qDebug() << " --- AND THIS SHOULD HAVE BEEN THE FINAL MOTION";
          }

          //if (isMain!=xmot.window) {
           // qDebug() << " --- OH BABY THIS REALLY SHOULD WORK ";// << isMain.cursor;
          //}
          //isMain = xmot.window;

        break;


        case MapNotify:
            xmap = event->xmap;
           // xmap.subwindow;
        //qDebug() << " --- MAP" << event->pad << " -- " << xmap.window << " - " << xmap.event;
        //XQueryTree(xmap.display, xmap.window, &r, &p, &kids, &numkids);
        //qDebug() << "kids: " << numkids << "parent:" << p << " root: " << r;


       // qDebug() << " --- MAPING" << event->pad << " -- " << xmapping.window;

//        if (xmapping.window == *isMain)
     //       qDebug() << " ++++++++++++++++++++++++++ ON THIS WINDOW SAVE";

//XUnmapSubwindows(xdisp,*isMain);
//XUnmapWindow(xdisp,xmapping.window);

       // qDebug() << "Verify" << xmapping.count << " - " << xmapping.first_keycode << " - " << xmapping.request << " - " << xmapping.send_event << " - " << xmapping.serial ;
//        if (isMain == &xmap.window)
//            qDebug() << " THIS IS WAYNE ";

       //     isMain = xmap.window;
  //          XQueryTree(xdisp,isMain,&xrootWindow, &xparent, &xchildren, &xNumChildren);
   //         qDebug() << " --- MAP" << event->type << " -- " << event->pad << " -- " << xNumChildren << " - " << xmapping.count;

break;
    case UnmapNotify:
     //   xunmap = event->xunmap;
      //  a = XFetchName(xunmap.display,xunmap.window,&window_name);
       // qDebug() << a << "UNMAPX" << xunmap.window << " - " << window_name << " -- " << xparent;
        //if (xunmapping.event == xunmapping.window)
        //    qDebug() << " THIS IS WAYNE UNMAP";

         //   qDebug() << " --- UNMAP" << event->type << " -- " << event->pad << " -- " << " - " << xunmap.window << " - " << xunmap.event;

//            if (xmapping.window == *isMain)
 //           {
//qDebug() << " --- AND THIS SHOULD HAVE BEEN THE FINAL UMAP";
   //         }


            break;
        default:
            break;
    }


    //XQueryTree()

    //qDebug() << "SPECIAL ++++++++++++++++++++++++++++++++ " << add.aaid;

    //foreach(LxQtPanel *i, mPanels)
    //    i->x11EventFilter(event);
    return false;
}


void LxQtPanelApplication::removePanel(LxQtPanel* panel)
{
    Q_ASSERT(mPanels.contains(panel));

    mPanels.removeAll(panel);
    
    QStringList panels = mSettings->value("panels").toStringList();
    panels.removeAll(panel->name());
    mSettings->setValue("panels", panels);
    
    panel->deleteLater();
}
