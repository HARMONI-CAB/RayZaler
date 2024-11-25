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

#ifndef _TOP_LEVEL_MODEL_H
#define _TOP_LEVEL_MODEL_H

#include <OMModel.h>
#include <GenericCompositeModel.h>
#include <map>

namespace RZ {
  class ApertureStop;

  class TopLevelModel : public GenericCompositeModel, public OMModel {
      std::map<std::string, ReferenceFrame *> m_ports;

    protected:
      // Interface methods
      virtual void registerDof(
        std::string const &name, 
        GenericModelParam *) override;

      virtual void registerParam(
        std::string const &name, 
        GenericModelParam *)  override;

      virtual void registerOpticalPath(
        std::string const &name,
        std::list<std::string> &params) override;
      
      virtual GenericEvaluator *allocateEvaluator(
        std::string const &expr,
        const GenericEvaluatorSymbolDict *dict,
        std::list<GenericCustomFunction *> const &functions,
        ExprRandomState *) override;
    
      void exposePort(
          std::string const &,
          ReferenceFrame *) override;

    public:
      TopLevelModel(Recipe *recipe);
      ~TopLevelModel();

      std::list<std::string> focalPlanes() const;
      std::list<std::string> ports() const;
      std::list<std::string> apertureStops() const;

      ApertureStop   *getApertureStop(std::string const &) const;
      ReferenceFrame *getFocalPlane(std::string const &) const;
      ReferenceFrame *getPort(std::string const &) const;

      virtual void notifyDetector(
        std::string const &preferredName,
        Detector *det) override;

      static TopLevelModel *fromFile(
        std::string const &path,
        std::list<std::string> const &searchPaths = std::list<std::string>());
  };
}

#endif // _TOP_LEVEL_MODEL_H
