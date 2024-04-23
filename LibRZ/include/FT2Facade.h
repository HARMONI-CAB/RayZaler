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
