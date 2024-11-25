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

#ifndef _RZ_GL_MODEL_H
#define _RZ_GL_MODEL_H

#include <GLModel.h>
#include <list>

namespace RZ {
  class Element;
  class OMModel;

  class RZGLModel : public GLModel {
      OMModel *m_model = nullptr; // Borrowed
      bool     m_showApertures = false;
      bool     m_showElements = true;

      void displayModel(OMModel *);

    public:
      virtual void display() override;
      void pushOptoMechanicalModel(OMModel *);
      void setShowApertures(bool);
      void setShowElements(bool);
  };
}

#endif // _RZ_GL_MODEL_H
