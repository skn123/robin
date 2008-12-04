#ifndef GTK_TUTORIAL_SCRIBBLE_H
#define GTK_TUTORIAL_SCRIBBLE_H

#include <vector>
#include <string>
#include <gtk/gtk.h>
#include "container.h"


template < typename Coordinate >
struct Point
{
  Coordinate x;
  Coordinate y;
};


class PointContainer : public Container<Point<int> >
{
public:
  PointContainer() { }
};


class Scribble
{
public:
  Scribble();
  Scribble(const std::string& name);
  Scribble(const std::vector<std::string>& args);

  void setContainer(PointContainer *container);

  void run();

  void clear();
  void paint(int x, int y);
  void paint(Point<int> xy);
  void paint(const std::vector<Point<int> >& points);


  void draw_brush(GtkWidget *widget, gdouble x, gdouble y);

private:
  void buildGUI(int argc, char *argv[]);

  GtkWidget *window;
  GtkWidget *drawing_area;
  GtkWidget *vbox;

  PointContainer *m_points;
};


#endif
