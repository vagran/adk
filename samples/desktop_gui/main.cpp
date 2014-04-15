/* /ADK/samples/desktop_gui/main.cpp
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file main.cpp
 * Sample desktop application with main window, simple controls and custom
 * drawn graphics.
 */

#include <adk.h>

#include "lib/sample_lib.h"

/** Main window class. */
class MainWindow: public Gtk::Window {
private:
    /** Subclass for drawing area. */
    class CDrawingArea: public Gtk::DrawingArea {
    public:
        typedef enum {
            SHAPE_RECTANGLE,
            SHAPE_ELLIPSE,
            SHAPE_TRIANGLE
        } shape_t;

    private:
        shape_t _curShape = SHAPE_RECTANGLE;

        /** Drawing event handler. */
        virtual bool
        on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
        {
            switch (_curShape) {
            case SHAPE_RECTANGLE:
                cr->rectangle(20, 20, 200, 100);
                cr->set_source_rgb(0, 0.8, 0);
                cr->fill_preserve();
                break;
            case SHAPE_ELLIPSE:
                cr->arc(150, 100, 90, 0, 2 * 3.14);
                cr->set_source_rgb(0.8, 0, 0);
                cr->fill_preserve();
                break;
            case SHAPE_TRIANGLE:
                cr->move_to(40, 40);
                cr->line_to(200, 40);
                cr->line_to(120, 160);
                cr->line_to(40, 40);
                cr->set_source_rgb(0.8, 0, 0.8);
                cr->fill_preserve();
                cr->set_line_cap(Cairo::LINE_CAP_ROUND);
                cr->set_line_join(Cairo::LINE_JOIN_ROUND);
                break;
            }

            cr->set_line_width(3);
            cr->set_source_rgb(0, 0, 0);
            cr->stroke();
            return true;
        }

    public:
        CDrawingArea(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder __UNUSED):
            Gtk::DrawingArea(cobject)
        {
        }

        void
        SetShape(shape_t shape)
        {
            if (_curShape != shape) {
                _curShape = shape;
                /* Request re-drawing. */
                queue_draw();
            }
        }
    };

    Glib::RefPtr<Gtk::Builder> _builder;
    Glib::RefPtr<Gtk::RadioButton> _rbRect, _rbEllipse, _rbTriangle;
    Glib::RefPtr<CDrawingArea> _drawingArea;
    Glib::RefPtr<Gtk::Paned> _paned;

    adk::Properties _props;
    adk::PropView _propView;
public:
    /** Signal handler which is called when any radio button is clicked. */
    void
    OnRadiobuttonClick()
    {
        if (_rbRect->get_active()) {
            _drawingArea->SetShape(CDrawingArea::SHAPE_RECTANGLE);
        } else if (_rbEllipse->get_active()) {
            _drawingArea->SetShape(CDrawingArea::SHAPE_ELLIPSE);
        } else if (_rbTriangle->get_active()) {
            _drawingArea->SetShape(CDrawingArea::SHAPE_TRIANGLE);
        }
    }

    /** "quit" action handler. */
    void
    OnQuit()
    {
        hide();
    }

    MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder):
        Gtk::Window(cobject), _builder(builder), _propView(_props)
    {
        _props.Load(adk::Xml().Load(adk::GetResource("props.xml").GetString()));

        /* Retrieve all widgets. */
        Gtk::RadioButton *rb;
        _builder->get_widget("rbRectangle", rb);
        _rbRect = Glib::RefPtr<Gtk::RadioButton>(rb);
        _builder->get_widget("rbEllipse", rb);
        _rbEllipse = Glib::RefPtr<Gtk::RadioButton>(rb);
        _builder->get_widget("rbTriangle", rb);
        _rbTriangle = Glib::RefPtr<Gtk::RadioButton>(rb);
        CDrawingArea *da;
        _builder->get_widget_derived("drawing_area", da);
        _drawingArea = Glib::RefPtr<CDrawingArea>(da);

        Gtk::Paned *paned;
        _builder->get_widget("leftPane", paned);
        _paned = Glib::RefPtr<Gtk::Paned>(paned);
        _paned->add2(*_propView.GetWidget());

        /* Connect signals. */
        _rbRect->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::OnRadiobuttonClick));
        _rbEllipse->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::OnRadiobuttonClick));
        _rbTriangle->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::OnRadiobuttonClick));
        /* Actions. */
        Glib::RefPtr<Gtk::Action>::cast_dynamic(_builder->get_object("action_quit"))->
            signal_activate().connect(sigc::mem_fun(*this, &MainWindow::OnQuit));
    }
};

int
main(int argc, char **argv)
{
    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv);
    Glib::RefPtr<Gtk::Builder> builder = g_sampleLib.Test();
    MainWindow *mainWindow = 0;
    builder->get_widget_derived("main_wnd", mainWindow);

    app->run(*mainWindow);
    delete mainWindow;
    return 0;
}
