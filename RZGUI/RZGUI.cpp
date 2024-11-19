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

#include "RZGUI.h"
#include <QSettings>
#include <QCoreApplication>

#include <SimulationSession.h>
#include <QMetaType>

RZGUISingleton *RZGUISingleton::m_instance = nullptr;

#define STR_I(x) #x
#define STR(x) STR_I(x)

#define RZ_REGISTER_DATATYPE(type)                     \
  do {                                                 \
    qRegisterMetaType<type>(STR(type));                \
  } while (false)


RZGUISingleton::RZGUISingleton()
{
  QCoreApplication::setOrganizationName(RZ_ORGANIZATION_NAME);
  QCoreApplication::setOrganizationDomain(RZ_ORGANIZATION_DOMAIN);
  QCoreApplication::setApplicationName(RZ_APPLICATION_NAME);

  m_settings = new QSettings(RZ_ORGANIZATION_NAME, RZ_APPLICATION_NAME);

  RZ_REGISTER_DATATYPE(ColoringMode);
  RZ_REGISTER_DATATYPE(ColorSettings);
}

RZGUISingleton *
RZGUISingleton::instance()
{
  if (m_instance == nullptr)
    m_instance = new RZGUISingleton();

 return m_instance;
}

QSettings *
RZGUISingleton::settings() const
{
  return m_settings;
}

