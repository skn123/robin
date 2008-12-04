#ifndef GTK_TUTORIAL_SCRIBBLE_H
#define GTK_TUTORIAL_SCRIBBLE_H

#include <vector>
#include <string>
#include <gtk/gtk.h>


struct Point
{
  int x;
  int y;
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
  void paint(Point xy);

private:
  void buildGUI(int argc, char *argv[]);

  GtkWidget *window;
  GtkWidget *drawing_area;
  GtkWidget *vbox;
};


#endif
