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

#ifndef _FT2_FACADE_H
#define _FT2_FACADE_H

#include <ft2build.h>
#include <string>
#include <map>
#include FT_FREETYPE_H  

namespace RZ {
  class FT2Facade {
      FT_Library m_ft;
      std::map<std::string, FT_Face> m_faces;

      FT_Face cachedFace(std::string const &name) const;

    public:
      FT2Facade();

      FT_Face loadFace(std::string const &path, FT_Error &err);
      FT_Face loadFace(std::string const &name, const void *, size_t, FT_Error &err);
  };
}

#endif // _FT2_FACADE_H
