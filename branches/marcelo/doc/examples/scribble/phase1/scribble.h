#ifndef GTK_TUTORIAL_SCRIBBLE_H
#define GTK_TUTORIAL_SCRIBBLE_H

#include <vector>
#include <string>


class Scribble
{
public:
  Scribble();
  Scribble(const std::string& name);
  Scribble(const std::vector<std::string>& args);

  void run();

private:
  void buildGUI(int argc, char *argv[]);
};


#endif
