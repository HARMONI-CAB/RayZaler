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
  RZ_REGISTER_DATATYPE(RepresentationProperties);
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
