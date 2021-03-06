/// @file Renderer.cpp
/// @brief Where all the calculations happen

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "Renderer.h"
#include "Ray.h"
#include "Material.h"
#include <ngl/Vec3.h>
#include <ngl/Types.h>

// from https://www.ross.click/2011/02/creating-a-progress-bar-in-c-or-any-other-console-app/  ROSS HEMSLEY
// Process has done i out of n rounds,
// and we want a bar of width w and resolution r.

/// The following section is from :-
/// Hemsley, R.(2011). Creating a progress bar in C/C++ (or any other console app). [online] [Accessed 2015].
/// Available from: <https://www.ross.click/2011/02/creating-a-progress-bar-in-c-or-any-other-console-app/>.
inline void Renderer::loadBar(int x, int n, int r, int w)
{
    // Only update r times.
    if ( x % (n/r +1) != 0 ) return;

    // Calculuate the ratio of complete-to-incomplete.
    float ratio = x/(float)n;
    int   c     = ratio * w;

    // Show the percentage complete.
    printf("%3d%% [", (int)(ratio*100) );

    // Show the load bar.
    for (int x=0; x<c; x++)
       printf("=");

    for (int x=c; x<w; x++)
       printf(" ");

    // ANSI Control codes to go back to the previous line and clear it.
    printf("]\n\033[F\033[J");
}
/// end of Citation
///
Renderer::Renderer() {}

Renderer::~Renderer()
{
  for(unsigned int i = 0; i < m_scene->m_objects.size(); i++)
  {
    delete m_scene->m_objects.at(i);
  }
  for(unsigned int i = 0; i < m_scene->m_lights.size(); i++)
  {
    delete m_scene->m_lights.at(i);
  }
}

void Renderer::bind(Scene *_scene, Film *_film, Camera *_camera, int _depth, int _anti_aliasing, std::string _image_name)
{
  m_scene = _scene;
  m_film = _film;
  m_camera = _camera;
  m_width = m_film->m_width;
  m_height = m_film->m_height;
  m_anti_aliasing = _anti_aliasing;
  m_max_depth = _depth;
  m_bg_colour = ngl::Vec3(0,0,0);
  m_image_name = _image_name;
}

int Renderer::getIndexClosest(std::vector<double> _interxs)
{
  int index_min_val;
  if(_interxs.size() == 0) {return -1;}
  else if(_interxs.size() == 1)
  {
    if(_interxs.at(0) > 0) {return 0;}
    else {return -1;}
  }
  else
  {
    double max = 0;
    for (unsigned int i = 0; i < _interxs.size(); i++)
    {
      if(max < _interxs.at(i)) {max = _interxs.at(i);}
    }
    if (max > 0)
    {
      for (unsigned int i = 0; i < _interxs.size(); i++)
      {
        if(_interxs.at(i) > 0 && _interxs.at(i) <= max)
        {
          max = _interxs.at(i);
          index_min_val = i;
        }
      }
      return index_min_val;
    }
    else {return -1;}
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * RAYCASTLGORITHM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool Renderer::raycast(ngl::Vec3 _from, int _avoid)
{
  bool shadowed = false;
  for(unsigned int i = 0; i < m_scene->m_lights.size(); i++)
  {
    // create vector that will store intersection values for parameter t in the primary ray
    std::vector<double> intersections;

    ngl::Vec3 dir = m_scene->m_lights.at(i)->m_pos - _from;
    float distance = dir.length();
    dir.normalize();
    geo::Ray fire_ray(_from, dir);

    // iterate over objects in the scene and find intersections
    for(unsigned int j = 0; j < m_scene->m_objects.size(); j++)
    {
      intersections.push_back( m_scene->m_objects.at(j)->getIntersection(fire_ray));
    }

    for(unsigned int k = 0; k < intersections.size(); k++)
    {
      if(intersections.at(k) < 0.1) continue;
      if(intersections.at(k) < -1) continue;
      int closest_index = getIndexClosest(intersections);
      if (closest_index == -1 || closest_index == _avoid) continue;
      if(intersections.at(k) > distance) continue;
      shadowed = true;
    }
  }
    return shadowed;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * TRACE ALGORITHM * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ngl::Colour Renderer::trace(ngl::Vec3 _from, ngl::Vec3 _direction, int depth)
{
  // create vector that will store intersection values for parameter t in the primary ray
  // a primary ray has a form like R = O + t * D where O is the origin vector, and D direction.
  std::vector<double> intersections;
  geo::Ray cam_ray(_from,_direction);

  // iterate over objects in the scene and find intersections
  for(unsigned int i = 0; i < m_scene->m_objects.size(); i++)
  {
    // each Shape subclass (Sphere, Plane...) has its own method for calculating intersections
    intersections.push_back( m_scene->m_objects.at(i)->getIntersection(cam_ray));
  }

  // find closest object
  int closest_index = getIndexClosest(intersections);

  // if no intersections are found RETURN black =
  if(closest_index == -1) {return ngl::Colour(0,0,0,1);}

  // calculate pHit (position of the new intersection) and nHit (normal at hit point)
  ngl::Vec3 pHit = _from + intersections.at(closest_index) * _direction;
  ngl::Vec3 nHit = m_scene->m_objects.at(closest_index)->getNormalAt(pHit);

  // calculate if we are inside or outside
  bool inside = false;
  if(_direction.dot(nHit) > 0)
  {
    nHit = -nHit;
    inside = true;
  }

 float bias = 0.01;
 // calculate if point is obscured or shadowed
 bool isObscured = raycast(pHit + nHit * bias, closest_index);

                      // // // // // // // // // // // // //
                      //  put all contributions together  //
                      // // // // // // // // // // // // //

  // is the object reflective or refractive???
  if ((m_scene->m_objects.at(closest_index)->getMaterial()->isReflective() ||
      m_scene->m_objects.at(closest_index)->getMaterial()->isRefractive()) &&
      depth < m_max_depth)
  {
    ngl::Colour crfr(0,0,0,1);
    ngl::Colour crfl(0,0,0,1);
    // check whether it is REFLECTIVE
    if (m_scene->m_objects.at(closest_index)->getMaterial()->isReflective())
    {
      // calculate reflection dir
      float bias = 0.01;
      ngl::Vec3 refl_dir = _direction - nHit * 2 * _direction.dot(nHit);
      refl_dir.normalize();

      // fire ray along reflection direction from hit point
      crfl = trace(pHit + bias*nHit, refl_dir, depth+1);
    }

    // check whether it is REFRACTIVE
    if (m_scene->m_objects.at(closest_index)->getMaterial()->isRefractive())
    {
      // calculate refrection dir (transmission ray)
      float ior = m_scene->m_objects.at(closest_index)->getMaterial()->getIOR();
      float eta = inside;
      float bias = 0.01;
      float cosi = -nHit.dot(_direction);

      if (eta == true) // we are inside
      {
        eta = ior;
      }
      else // we are outside
      {
        eta = 1 / ior;
      }

      float k = 1 - eta * eta * (1 - cosi * cosi);
      ngl::Vec3 refr_dir = _direction * eta + nHit * (eta * cosi - sqrt(k));
      refr_dir.normalize();
      crfr = trace(pHit - nHit * bias, refr_dir, depth+1);
    }

    ngl::Colour surfaceColor = m_scene->m_objects.at(closest_index)->getColour(pHit);
    float cosineFactor = std::max(-nHit.dot(cam_ray.getDirection()),(float)0);
    float attenuation;

    ngl::Colour Ka(1,0,0.4,1);
    ngl::Colour Kd;
    ngl::Colour Ks;

    float ambient_intensity = 0.05;

    ngl::Colour ambient_contrib = Ka * surfaceColor * ambient_intensity;
    ngl::Colour diffuse_contrib(0,0,0,1);
    ngl::Colour specular_contrib(0,0,0,1);

    for(unsigned int m = 0; m < m_scene->m_lights.size(); m++)
    {
      ngl::Vec3 v_distance = m_scene->m_lights.at(m)->m_pos - pHit;
      float distance = v_distance.length();
      float radius = 8;
      attenuation = 1 - pow(distance/radius,2);

      Kd = m_scene->m_lights.at(m)->m_diff_col;
      Ks = m_scene->m_lights.at(m)->m_spec_col;

      ngl::Vec3 L = m_scene->m_lights.at(m)->m_pos - pHit;
      L.normalize();
      ngl::Vec3 N = nHit;
      ngl::Vec3 R = 2 * (L.dot(N) * N) - L;
      R.normalize();

      diffuse_contrib  += (surfaceColor * (Kd * pow(std::max(L.dot(N),(float)0),2) * m_scene->m_lights.at(m)->m_diff_int))*attenuation;
      specular_contrib += ((Ks * pow(std::max(R.dot(-_direction),(float)0),900)*400 * m_scene->m_lights.at(m)->m_spec_int))*attenuation;
    }

    specular_contrib.clamp(0,0.8);

    ngl::Colour s01 = crfl * m_scene->m_objects.at(closest_index)->getMaterial()->getReflIntensity();
    ngl::Colour s02 = crfr * m_scene->m_objects.at(closest_index)->getMaterial()->getTransparency();
    ngl::Colour s03 = s01 + s02;
    ngl::Colour diffuseColor = m_scene->m_objects.at(closest_index)->getColour(pHit) * cosineFactor * m_scene->m_objects.at(closest_index)->getMaterial()->getDiffuseIntensity();

    // Do PHONG MODEL calculations stuff. By now I keep it VERY VERY simple
    ngl::Colour outRadiance = diffuseColor + s03 + specular_contrib + ambient_contrib;
    outRadiance.clamp(0,1);

    return isObscured ? outRadiance * 0.7f : outRadiance;
  }

  // if it is not REFLECTIVE nor REFRACTIVE
  else
  {
    ngl::Colour surfaceColor = m_scene->m_objects.at(closest_index)->getColour(pHit);
    float attenuation;

    ngl::Colour Ka(1,0,0.4,1);
    ngl::Colour Kd;
    ngl::Colour Ks;

    float ambient_intensity = 0.05;

    ngl::Colour ambient_contrib = Ka * surfaceColor * ambient_intensity;
    ngl::Colour diffuse_contrib(0,0,0,1);
    ngl::Colour specular_contrib(0,0,0,1);

    for(unsigned int m = 0; m < m_scene->m_lights.size(); m++)
    {
      ngl::Vec3 v_distance = m_scene->m_lights.at(m)->m_pos - pHit;
      float distance = v_distance.length();
      float radius = 8;
      attenuation = 1 - pow(distance/radius,2);

      Kd = m_scene->m_lights.at(m)->m_diff_col;
      Ks = m_scene->m_lights.at(m)->m_spec_col;

      ngl::Vec3 L = m_scene->m_lights.at(m)->m_pos - pHit;
      L.normalize();
      ngl::Vec3 N = nHit;
      ngl::Vec3 R = 2 * (L.dot(N) * N) - L;
      R.normalize();

      float spec_hardness = m_scene->m_objects.at(closest_index)->getMaterial()->m_spec_hardness;

      diffuse_contrib  += (surfaceColor * (Kd * pow(std::max(L.dot(N),(float)0),2) * m_scene->m_lights.at(m)->m_diff_int))*attenuation;
      specular_contrib += ((Ks * pow(std::max(R.dot(-_direction),(float)0),spec_hardness) * m_scene->m_lights.at(m)->m_spec_int))*attenuation;
    }

    diffuse_contrib.clamp(0,1);
    specular_contrib.clamp(0,1);

    ngl::Colour outRadiance = diffuse_contrib + specular_contrib + ambient_contrib;

    outRadiance.clamp(0,1);

    return isObscured ? outRadiance * 0.7f : outRadiance;
  }
}

void Renderer::render()
{
  std::vector<ngl::Colour> colourStack;
  for(int y = 0; y < m_film->m_height; y++)
  {
    for(int x = 0; x < m_film->m_width; x++)
    {
      int current_pixel = y*m_film->m_height + x;
      int total_number_of_pixels = m_film->m_height * m_film->m_width;
      int r = 90;
      int w = 45;

      loadBar(current_pixel, total_number_of_pixels, r, w);

      ngl::Colour finalColour;
      if(m_anti_aliasing)
      {
        for(int aay = 0; aay < m_anti_aliasing; aay++)
        {
          for(int aax = 0; aax < m_anti_aliasing; aax++)
          {
            // calculate the primary ray
            float x_amount = ( (float)x + ((float)aax / (float)m_anti_aliasing) + 0.5f * ((float)aax / (float)m_anti_aliasing) ) / (float)m_film->m_width;
            float y_amount = ( (float)y + ((float)aay / (float)m_anti_aliasing) + 0.5f * ((float)aay / (float)m_anti_aliasing) ) / (float)m_film->m_width;

            ngl::Vec3 cam_ray_dir = m_camera->m_dir + (m_camera->m_right * (x_amount - 0.5) + (m_camera->m_down * (y_amount - 0.5)));
            cam_ray_dir.normalize();

            // fire the ray and store its colour into a variable
            ngl::Colour col = trace(m_camera->m_pos, cam_ray_dir, 0);

            colourStack.push_back(col);
          }
        }

        // AVERAGE COLOURS
        float cRed   = 0.0f;
        float cGreen = 0.0f;
        float cBlue  = 0.0f;

        for(int i = 0; i < m_anti_aliasing; i++)
        {
          cRed    += colourStack.at(i).m_r;
          cGreen  += colourStack.at(i).m_g;
          cBlue   += colourStack.at(i).m_b;
        }

        cRed   /= (float)m_anti_aliasing;
        cGreen /= (float)m_anti_aliasing;
        cBlue  /= (float)m_anti_aliasing;

        finalColour = ngl::Colour(cRed, cGreen, cBlue, 1);

        // FLUSH VECTOR
        colourStack.clear();
      }

      else // there is anti-aliasing
      {
        float x_amount = (x+0.5)/(float)m_film->m_width;
        float y_amount = ((y) + 0.5)/(float)m_film->m_height;

        ngl::Vec3 cam_ray_dir = m_camera->m_dir + (m_camera->m_right * (x_amount - 0.5) + (m_camera->m_down * (y_amount - 0.5)));
        cam_ray_dir.normalize();

        // fire the ray and store its colour into a variable
        finalColour = trace(m_camera->m_pos, cam_ray_dir, 0);
      }

      // write pixel into the Film object associated to the Render object
      m_film->writePixel(finalColour);
    }
  }


  // write the file into disk afterwards and display it
  std::string imageFileName = m_image_name + ".ppm";
  m_film->writeFile(imageFileName.c_str());

}
