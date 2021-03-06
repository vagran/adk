/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
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

        shape_t
        GetShape()
        {
            return _curShape;
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
        auto SetProps = [this](const std::string s, double d, bool b) {
            ADK_INFO("clicked %s", s.c_str());//XXX
            bool f = _props["sample/Item5"].Val<bool>();
            bool exists = _props["sample/cat"];
            std::string s1 = s + " " + (f ? "true" : "false");
            auto trans = _props.OpenTransaction();
            trans->Modify("sample/Item3", s1);
            trans->Modify("Item2", d);
            trans->Modify("sample/Item4", b);
            if (exists) {
                trans->Delete("sample/cat");
            } else {
                trans->Add("sample/cat");
                trans->Add("sample/cat/a", 1);
                trans->Add("sample/cat/b", true);
                trans->Add("sample/cat/c", "aaa");
            }
            trans->Commit();
        };

        CDrawingArea::shape_t newShape = CDrawingArea::SHAPE_RECTANGLE;
        if (_rbRect->get_active()) {
            newShape = CDrawingArea::SHAPE_RECTANGLE;
        } else if (_rbEllipse->get_active()) {
            newShape = CDrawingArea::SHAPE_ELLIPSE;
        } else if (_rbTriangle->get_active()) {
            newShape = CDrawingArea::SHAPE_TRIANGLE;
        }

        if (newShape == _drawingArea->GetShape()) {
            return;
        }
        _drawingArea->SetShape(newShape);

        switch (newShape) {
        case CDrawingArea::SHAPE_RECTANGLE:
            SetProps("Rectangle", 3.14, true);
            break;
        case CDrawingArea::SHAPE_ELLIPSE:
            SetProps("Ellipse", 42.42, false);
            break;
        case CDrawingArea::SHAPE_TRIANGLE:
            SetProps("Triangle", 33.33, true);
            break;
        }
    }

    /** "quit" action handler. */
    void
    OnQuit()
    {
        hide();
    }

    MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder):
        Gtk::Window(cobject), _builder(builder), _propView(_props, false, true)
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
        _paned->add2(_propView.GetWidget());
        _paned->show();
        _propView.Show();

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
