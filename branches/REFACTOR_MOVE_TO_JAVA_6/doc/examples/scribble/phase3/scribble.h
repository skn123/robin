#ifndef GTK_TUTORIAL_SCRIBBLE_H
#define GTK_TUTORIAL_SCRIBBLE_H

#include <vector>
#include <string>
#include <gtk/gtk.h>


template < typename Coordinate >
struct Point
{
  Coordinate x;
  Coordinate y;
};


class Scribble
{
public:
  Scribble();
  Scribble(const std::string& name);
  Scribble(const std::vector<std::string>& args);

  void run();

  void clear();
  void paint(int x, int y);
  void paint(Point<int> xy);
  void paint(const std::vector<Point<int> >& points);

private:
  void buildGUI(int argc, char *argv[]);

  GtkWidget *window;
  GtkWidget *drawing_area;
  GtkWidget *vbox;
};


#endif
