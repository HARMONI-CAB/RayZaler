//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program.  If not, see
//  <http://www.gnu.org/licenses/>
//

#ifndef _GLUT_ENGINE_H
#define _GLUT_ENGINE_H

#include <GLRenderEngine.h>
#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <IncrementalRotation.h>

#define RZ_GLUT_ENGINE_MOUSE_ROT_DELTA 2e-1
#define RZ_GLUT_ENGINE_KBD_ROT_DELTA   5

namespace RZ {
  class GLUTEngine : public GLRenderEngine {
      static GLUTEngine *currentInstance;
      bool    m_fixedLight = false;
      int     m_width;
      int     m_height;
      int     m_hWnd = -1;
      
      bool    m_newViewPort = false;
      bool    m_dragging = false;
      GLfloat m_dragStart[2] = {0, 0};

      GLfloat m_oldCenterCenter[2] = {0, 0};

      bool    m_rotating = false;
      GLfloat m_prevRotX, m_prevRotY;
      GLfloat m_rotStart[2] = {0, 0};
      GLfloat m_curAzEl[3] = {0, 0};
      GLfloat m_oldRot[2] = {0, 0};
      
      void adjustViewPort();
      void mouseClick(int button, int state, int x, int y);
      void mouseMotion(int x, int y);
      void keyPress(int c, int x, int y);
      void showScreen();

      static void staticMouseClick(int, int, int, int);
      static void staticMouseMotion(int, int);
      static void staticDisplayFunc();
      static void staticKeyboardFunc(int c, int x, int y);

    private:
      GLUTEngine();
      ~GLUTEngine();

    public:
      static GLUTEngine *instance();
      void setFixedLight(bool);
      void start();
  };
}

#endif // _GLUT_ENGINE_H
