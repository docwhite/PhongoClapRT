/// @file main.cpp
/// @brief The program starts here.

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <time.h>
#include <assert.h>
#include <ngl/Vec3.h>
#include <vector>
#include <boost/tokenizer.hpp>

#include "Singleton.h"
#include "Parser.h"
#include "Scene.h"
#include "Film.h"
#include "Shape.h"
#include "Sphere.h"
#include "Plane.h"
#include "Renderer.h"


void printOutTime(float _seconds)
{
  if (_seconds<60.0f)
  {
    std::cout << "Render time: "  << (int)_seconds << " seconds\n";
  }
  else if (_seconds > 60.0f && _seconds < 3600.0f)
  {
    int minutes = (int)_seconds / (int)60;
    float seconds = (int)_seconds % (int)60;

    std::cout << "Render time: "  << minutes << "min " << seconds << "seconds\n";
  }
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    std::cout << "ERROR, you must indicate the scene file you want to read from.\n"
                 "Usage: ./aidan [path_to_textfile]";
  }
  assert(argc == 2);

  // USER DEFINES THESE VARIABLES
  int width         = 0;
  int height        = 0;
  float camPosX     = 0;
  float camPosY     = 0;
  float camPosZ     = 0;
  float lookAtX     = 0;
  float lookAtY     = 0;
  float lookAtZ     = 0;
  int max_depth     = 0;
  int anti_aliasing = 0;

  std::vector<Light*> scene_lights;
  std::vector<geo::Shape*> scene_objects;
  std::string text_file = argv[1];
  std::string image_name;
  Parser scene_parser(image_name,
                      text_file,
                      width,
                      height,
                      camPosX,
                      camPosY,
                      camPosZ,
                      lookAtX,
                      lookAtY,
                      lookAtZ,
                      max_depth,
                      anti_aliasing,
                      scene_lights,
                      scene_objects);

  if(anti_aliasing < 1)
  {
    std::cout << "ERROR: Anti-aliasing should be 1 (no-antialising) or more." << std::endl;
    assert(anti_aliasing > 0);
  }

  // instanciate a scene object
  Scene* myScene_instance = Singleton<Scene>::Instance();

  // push all the object that the user specified into the scene
  for(unsigned int i = 0; i < scene_objects.size(); i++)
  {
    myScene_instance->addObject(scene_objects.at(i));
  }

  // push all the lights specified by the user into the scene
  for(unsigned int i = 0; i < scene_lights.size(); i++)
  {
    myScene_instance->addLight(scene_lights.at(i));
  }

  // instanciate film and set its dimensions
  Film* myFilm_instance = Singleton<Film>::Instance();
  myFilm_instance->setDimensions(width, height);

  // calculate camera right and down vectors for generating the camera rays properly in the renderer
  ngl::Vec3 Y (0,1,0);
  ngl::Vec3 campos(camPosX,camPosY,camPosZ);
  ngl::Vec3 lookat(lookAtX,lookAtY,lookAtZ);
  ngl::Vec3 diff_btw = campos - lookat; diff_btw.normalize();
  ngl::Vec3 camdir = -diff_btw;
  ngl::Vec3 camright = Y.cross(camdir);
  ngl::Vec3 camdown = camright.cross(camdir);

  // initialise camera
  Camera* myCamera_instance = Singleton<Camera>::Instance();
  myCamera_instance->setParameters(campos,camdir,camright,camdown);

  // instanciate a renderer and bind film and camera to it
  Renderer* renderer_instance = Singleton<Renderer>::Instance();
  renderer_instance->bind(myScene_instance, myFilm_instance, myCamera_instance, max_depth, anti_aliasing, image_name);

  // clock in so that we can now what the render time is after the render process takes place
  clock_t t;
  t = clock();
  std::cout << "Rendering...\n";

  // start the rendering process
  renderer_instance->render();

  // Calculate render time
  t = clock() - t;
  float seconds = (float)t/CLOCKS_PER_SEC;
  printOutTime(seconds);

  // destroy the renderer, camera, film and scene
  Singleton<Renderer>::destroyInstance();
  Singleton<Camera>::destroyInstance();
  Singleton<Film>::destroyInstance();
  Singleton<Scene>::destroyInstance();

  // Sucess message for the user and display the image
  std::cout << image_name << ".ppm has been written successfully.\n";
  std::string command = "display " + image_name + ".ppm";
  system(command.c_str());

  return 0;
}
