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

#include <TranslatedFrame.h>

using namespace RZ;

TranslatedFrame::TranslatedFrame(
  std::string const &name,
  ReferenceFrame *parent,
  Vec3 const &d) : ReferenceFrame(name, parent)
{
  m_typeId   = RZ_REF_FRAME_TRANSLATION_ID;
  m_distance = d;

  // Add displacement vector 
  m_distanceIndex = parent->addAxis(name + ".displacement", d);
}

void
TranslatedFrame::setDistance(Vec3 const &d)
{
  m_distance = d;
  *parent()->getAxis(m_distanceIndex) = d;
}

void
TranslatedFrame::setDistanceX(Real val)
{
  m_distance.x = val;
  parent()->getAxis(m_distanceIndex)->x = val;
}

void
TranslatedFrame::setDistanceY(Real val)
{
  m_distance.y = val;
  parent()->getAxis(m_distanceIndex)->y = val;
}

void
TranslatedFrame::setDistanceZ(Real val)
{
  m_distance.z = val;
  parent()->getAxis(m_distanceIndex)->z = val;
}

void
TranslatedFrame::recalculateFrame()
{
  setOrientation(parent()->getOrientation());
  setCenter(parent()->getCenter() + parent()->getOrientation().t() * m_distance);
}
