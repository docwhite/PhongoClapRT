#ifndef NGLSCENE_H__
#define NGLSCENE_H__

/// @file Camera.h
/// @author Ramon Blanquer
/// @brief Implements camera functionability, no transformations, just explicit definition.
/// @todo Implement transformations.

#include <ngl/Vec3.h>
#include <ngl/Mat4.h>

//----------------------------------------------------------------------------------------------------------------------
/// @class Camera
/// @brief Holds camera functions which will be accessed by the Render class.
/// @author Ramon Blanquer
//----------------------------------------------------------------------------------------------------------------------
class Camera    
{
public:
  // -------------------------------------------------------------------------------------------------------------------
  /// @brief Camera constructor.
  // -------------------------------------------------------------------------------------------------------------------
  Camera();
  //--------------------------------------------------------------------------------------------------------------------
  /// @brief Blank destructor.
  //-------------------------------------------------------------------------------------------------------------------
  ~Camera();
  //-------------------------------------------------------------------------------------------------------------------
  /// @brief Initialises the camera settings.
  /// @param[in] m_pos   Camera position.
  /// @param[in] m_dir   Camera aim vector.
  /// @param[in] m_down  Down vector.
  /// @param[in] m_right Right vector.
  //-------------------------------------------------------------------------------------------------------------------
  void setParameters(ngl::Vec3 _pos, ngl::Vec3 _dir, ngl::Vec3 _right, ngl::Vec3 _down);

  // allows to Renderer class access to private interface
  friend class Renderer;

private:
  //--------------------------------------------------------------------------------------------------------------------
  /// @brief Camera position vector.
  // -------------------------------------------------------------------------------------------------------------------
  ngl::Vec3 m_pos;
  //--------------------------------------------------------------------------------------------------------------------
  /// @brief Camera aim vector.
  // -------------------------------------------------------------------------------------------------------------------
  ngl::Vec3 m_dir;
  //--------------------------------------------------------------------------------------------------------------------
  /// @brief Camera down vector.
  // -------------------------------------------------------------------------------------------------------------------
  ngl::Vec3 m_down;
  //--------------------------------------------------------------------------------------------------------------------
  /// @brief Camera right vector.
  // -------------------------------------------------------------------------------------------------------------------
  ngl::Vec3 m_right;
};

#endif // Camera.h
