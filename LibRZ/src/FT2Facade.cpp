#include <FT2Facade.h>
#include <Logger.h>

using namespace RZ;

// TODO: Raise exception?
FT2Facade::FT2Facade()
{
  FT_Error error = FT_Init_FreeType(&m_ft);

  if (error != FT_Err_Ok)
    RZError("Failed to initialize Freetype2: %s\n", FT_Error_String(error));
}

FT_Face
FT2Facade::cachedFace(std::string const &name) const
{
  auto it = m_faces.find(name);
  if (it != m_faces.end())
    return const_cast<FT_Face>(it->second);

  return nullptr;
}

FT_Face
FT2Facade::loadFace(std::string const &path, FT_Error &err)
{
  FT_Face face = cachedFace(path);
  FT_Face newFace;

  if (face != nullptr)
    return face;

  err = FT_New_Face(m_ft, path.c_str(), 0, &newFace);

  if (err == FT_Err_Ok)
    m_faces[path] = newFace;

  return cachedFace(path);
}

FT_Face
FT2Facade::loadFace(
  std::string const &name,
  const void *buf,
  size_t size,
  FT_Error &err)
{
  FT_Face face = cachedFace(name);
  FT_Face newFace;

  if (face != nullptr)
    return face;

  if (buf == nullptr || size == 0) {
    err = FT_Err_Cannot_Open_Resource;
    return nullptr;
  }

  err = FT_New_Memory_Face(
    m_ft,
    reinterpret_cast<const FT_Byte *>(buf),
    size,
    0,
    &newFace);
  
  if (err == FT_Err_Ok)
    m_faces[name] = newFace;

  return cachedFace(name);
}

