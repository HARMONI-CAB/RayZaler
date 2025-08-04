//
//  Copyright (c) 2025 Gonzalo José Carracedo Carballal
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

#ifndef _EM_FIELDS_EM_SOLVER_H
#define _EM_FIELDS_EM_SOLVER_H

#include <EMInterface.h>
#include <Matrix4.h>
#include <Helpers.h>

#define COPYDIR(rolesfx) \
  dirs.JOIN(i, rolesfx) = JOIN(i, rolesfx); \
  dirs.JOIN(f, rolesfx) = JOIN(f, rolesfx); \
  dirs.JOIN(g, rolesfx) = JOIN(g, rolesfx);

namespace RZ {
  enum RayBreakMask {
    NoRays = 0,
    ReflectedOrdinary = 1,
    ReflectedExtraordinary = 2,
    TransmittedOrdinary = 4,
    TransmittedExtraordinary = 8,
    AllRays = ReflectedOrdinary | ReflectedExtraordinary | TransmittedOrdinary | TransmittedExtraordinary
  };

  struct EMFields {
    Complex Dx;
    Complex Dy;
    Vec3   vDx;
    Vec3   vDy;
  };

    // The EMSolver code represents the EM field by means of scalar and vector
    // quantities such that, once multiplied by the appropriate amplitudes of
    // D, produce the vectors of D, E, H and B
    //
    // Throughout the code, these quantities use the following naming scheme:
    //
    //   <CLASS>[<ROLE>][<DISCRIMINATOR>]
    //
    // Where:
    //   CLASS: What kind of quantity this vector is describing.
    //      B: Magnetic induction
    //      D: Electric displacement
    //      E: Electric field
    //      H: Magnetic field
    //      S: Poynting vector
    //      f: Direction and scaling of the electric field (E) (eps^{-1} i)
    //      g: Direction and scaling of the magnetic field (H) (i x u) / n
    //      i: Direction of the electric displacement (D)
    //      k: Wave vector, whose norm is always the effective refractive index.
    //      u: Wave normal. This is just the normalized wavevector.
    //
    //  ROLE (optional): What role this vector refers to
    //      i: Incident ray field
    //      o: Ordinary ray field
    //      e: Extraordinary ray field
    //      s: Field parallel to the ws component
    //      t: Field perpendicular to the ws component
    //      Unspecified: total field at a given side of the interface
    //
    //  DISCRIMINATOR (optional): Used to distinguish between different roles
    //      1: First medium (reflected rays)
    //      2: Second medium (transmitted rays)
    //      R: Real (in-phase) field
    //      I: Imaginary (quadrature) field
    //      Unspecified: no discrimination needed
    //
    //  For instance, the vector "fe1" is used to derive the electric field
    //  vector from the dielectric displacement amplitude of the reflected
    //  extraordinary ray:
    //
    //    Ee1 = De1 * fe1
    //    He1 = De1 * ge1
    //    Se1 = Ee1 x He1
    //

#define CDV(vcls) \
  JOIN(vcls, o1) = Vec3::zero(); \
  JOIN(vcls, e1) = Vec3::zero(); \
  JOIN(vcls, o2) = Vec3::zero(); \
  JOIN(vcls, e2) = Vec3::zero(); \

#define DECLDV(vcls)                                   \
  Vec3 JOIN(vcls, iR);                                 \
  Vec3 JOIN(vcls, iI);                                 \
  union { Vec3 JOIN(vcls, o1); Vec3 JOIN(vcls, s1); }; \
  union { Vec3 JOIN(vcls, e1); Vec3 JOIN(vcls, t1); }; \
  union { Vec3 JOIN(vcls, o2); Vec3 JOIN(vcls, s2); }; \
  union { Vec3 JOIN(vcls, e2); Vec3 JOIN(vcls, t2); }

  struct EMFieldDirections {
    DECLDV(i);
    DECLDV(f);
    DECLDV(g);

    Vec3 DiR, DiI; // Real and imaginary amplitudes of Di
    Vec4 DR,  DI;  // Real and imaginary amplitudes of Do1, De1, Do2, De2

    inline EMFieldDirections()
    {
      CDV(i);
      CDV(f);
      CDV(g);
    }
  };

#undef DECLDV
#undef CDV

  struct EMSolver {
    // Interface
    const ReferenceFrame *frame = nullptr;
    bool  debug = false; // Calculate intermediate fields

    Vec3  normal;
    const EMMedium *m1 = nullptr;
    const EMMedium *m2 = nullptr;

    // Incident ray
    Vec3 viDx; // Direction of the X component of D, wrt ui
    Vec3 viDy; // Direction of the Y component of D, wrt ui
    Complex Dx = 0;
    Complex Dy = 0;

    // Wave vectors
    Vec3 ki;

    Vec3 ko1, ke1;
    Vec3 ko2, ke2;

    // Precalculated quantities
    Vec3 ui;           // Direction of the incident ray
    Real ni = 1;       // Effective refractive index of the incident ray
    Real n2ton1sq = 1; // Square ratio of N2 to N1

    Vec3 uo1;          // Direction of the ordinary reflected ray
    Vec3 ue1;          // Direction of the extraordinary reflected ray
    Real nee1 = 1;     // Effective refractive index of the extraordinary reflected ray
    Vec3 ax1;          // Optical axis of the first medium
    Matrix3 iep1;      // Inverse dielectric tensor for the first medium

    Vec3 uo2;          // Direction of the ordinary transmitted ray
    Vec3 ue2;          // Direction of the extraordinary transmitted ray
    Real nee2 = 1.5;   // Effective refractive index of the extraordinary transmitted ray
    Vec3 ax2;          // Optical axis of the second medium
    Matrix3 iep2;      // Inverse dielectric tensor for the second medium

    EMFieldDirections dirs;

    RayBreakMask rayMask = NoRays;
    Vec3 DReal, iiR;
    Vec3 DImag, iiI;
    Real DampR, DampI;

    // Incidence reference frame. Triad is ws, wq, normal (or w2, w1, eta)
    Vec3 ws; // Perpendicular to the incidence plane ("w1")
    Vec3 wq; // Parallel to the incidence plane ("w2")
    
    inline void
    setReferenceFrame(const ReferenceFrame *frame)
    {
      this->frame = frame;
    }

    inline void
    setMedia(const EMMedium *m1, const EMMedium *m2) {
      this->m1 = m1;
      this->m2 = m2;

      if (m1->isotropic() && m2->isotropic()) {
        Real n1   = m1->n;
        Real n2   = m2->n;

        Real n1sq = n1 * n1;
        Real n2sq = n2 * n2;

        n2ton1sq  = n2sq / n1sq;
      }

      // Calculate dielectric tensor for first medium
      if (!m1->isotropic()) {
        ax1 = frame->toRelativeVec(m1->frame->fromRelativeVec(m1->axis));
        m1->ieps(iep1, ax1);
      }

      // Calculate dielectric tensor for second medium
      if (!m2->isotropic()) {
        ax2 = frame->toRelativeVec(m2->frame->fromRelativeVec(m2->axis));
        m2->ieps(iep2, ax2);
      }
    }


    // Calculate reference systems
    inline void
    calcIncidentFrame() {
      ws = ui.cross(normal);
  
      if (ws.isNull()) {
        auto v1 = normal.cross(Vec3::eX());
        auto v2 = normal.cross(Vec3::eY());

        if (v1 * v1 > v2 * v2)
          ws = v1;
        else
          ws = v2;
      }

      ws = ws.normalized();
      wq = normal.cross(ws).normalized();
    }

    inline bool
    setIncidentRay(
      Vec3 const &k_i,
      const Vec3 &normal,
      Complex Dx,
      Complex Dy,
      Vec3 const viDx) {
      bool direct  = true;
      ki           = k_i;
      ni           = k_i.norm();
      ui           = k_i / ni;

      // Ray is coming from behind! Need to invert roles
      if (ui * normal > 0) {
        setMedia(m2, m1);
        this->normal = -normal;
        direct = false;
      } else {
        this->normal = normal;
      }

      this->Dx   = Dx;
      this->Dy   = Dy;
      this->viDx = viDx;
      this->viDy = ui.cross(viDx);
      
      DReal = Dx.real() * viDx + Dy.real() * viDy;
      DImag = Dx.imag() * viDx + Dy.imag() * viDy;

      DampR = DReal.norm();
      DampI = DImag.norm();

      iiR = DReal / DampR;
      iiI = DImag / DampI;

      calcIncidentFrame();
    }

    // "Reflection" coefficients.
    // @bfr: Term before the radical
    // @rad: Radical term
    // @returns: true if radical term is real, false if it is imaginary
    inline bool G(
      Real &bfr,
      Real &rad,
      Real n_o,
      Real n_e,
      const Vec3 *axis) {
      
      Real a, b, c;
      auto kieta = -ki * normal;
      auto k_i2  = ki * ki;

      if (axis != nullptr) {
        auto qj = (n_e * n_e - n_o * n_o) / (n_o * n_o);
        auto etaaj = -normal * *axis;
        
        auto kiaj  = ki * *axis;

        a     = 1 + qj * etaaj * etaaj;
        b     = 2 * (kieta + qj * kiaj * etaaj);
        c     = k_i2 - n_e * n_e + qj * kiaj * kiaj;
      } else {
        a = 1;
        b = 2 * kieta;
        c = k_i2  - n_o * n_o;
      }

      auto inv2a = .5 / a;
      auto D     = b * b - 4 * a * c;

      bfr = -b * inv2a;

      if (D < 0) {
        rad = sqrt(-D) * inv2a;
        return false;
      } else {
        rad = sqrt(D) * inv2a;
        return true;
      }
    }

    // Break rays
    inline RayBreakMask
    rayBreak() {
      Real B, R;

      uint32_t mask = NoRays;

      // Calculate reflected ordinary
      if (G(B, R, m1->no, m1->no, nullptr)) {
        ko1   = ki - (B - R) * normal;
        uo1   = ko1 / m1->no;
        mask |= ReflectedOrdinary;
      }

      // Calculate transmitted ordinary
      if (G(B, R, m2->no, m2->no, nullptr)) {
        ko2   = ki - (B + R) * normal;
        uo2   = ko2 / m2->no;
        mask |= TransmittedOrdinary;
      }

      // Calculare reflected extraordinary
      if (!m1->isotropic() && G(B, R, m1->no, m1->ne, &m1->axis)) {
        ke1   = ki - (B - R) * normal;
        nee1  = ke1.norm();
        ue1   = ke1 / nee1;
        mask |= ReflectedExtraordinary;
      }

      // Calculare transmitted extraordinary
      if (!m2->isotropic() && G(B, R, m2->no, m2->ne, &m2->axis)) {
        ke2   = ki - (B + R) * normal;
        nee2  = ke2.norm();
        ue2   = ke2 / nee2;
        mask |= TransmittedExtraordinary;
      }

      rayMask = static_cast<RayBreakMask>(mask);
      return rayMask;
    }


    // Solve fields for the isotropic-to-isotropic case
    inline void
    solveIsoIso(EMFields &reflected, EMFields &transmitted) {
      Real n1   = m1->no;
      Real n2   = m2->no;

      auto uin  = ui  * normal;
      auto utn  = uo2 * normal;

      /////////////////// Reflection and transmission coefficients /////////////////
      // Secant component
      auto rs  = (n1 * uin - n2 * utn) / (n1 * uin + n2 * utn);
      auto ts  = (2 * n1 * uin)        / (n1 * uin + n2 * utn);

      // Parallel component
      auto rp  = (n2 * uin - n1 * utn) / (n2 * uin + n1 * utn);
      auto tp  = (2 * n1 * uin)        / (n2 * uin + n1 * utn);

      // Deduction of the parallel components of each ray
      auto wip = ui.cross(ws).normalized();
      auto wrp = uo1.cross(ws).normalized();
      auto wtp = uo2.cross(ws).normalized();

      auto Dis  = Complex(DReal * ws,  DImag * ws);
      auto Dip  = Complex(DReal * wip, DImag * wip);

      // Calculation of the field amplitudes of the transmitted ray, in the SxP plane
      // The n2ton1sq "undoes" the effect of the refractive index on the
      // Fresnel equations.
      transmitted.Dx  = ts * Dis * n2ton1sq;
      transmitted.Dy  = tp * Dip * n2ton1sq;
      transmitted.vDx = ws;
      transmitted.vDy = wtp;

      // Calculation of the field amplitudes of the reflected ray, in the SxP plane
      // The n2ton1sq term is not needed here, as both the incident and reflected
      // rays lie on the same medium.
      reflected.Dx  = rs * Dis;
      reflected.Dy  = rp * Dip;
      reflected.vDx = ws;
      reflected.vDy = wrp;

      // Copy debug vectors, if requested to do so
      if (debug) {
        auto n1sq = n1 * n1;
        auto n2sq = n2 * n2;

        dirs.DiR  = DReal;
        dirs.DiI  = DImag;

        dirs.DR   = Vec4(reflected.Dx.real(), reflected.Dy.real(), transmitted.Dx.real(), transmitted.Dy.real());
        dirs.DI   = Vec4(reflected.Dx.imag(), reflected.Dy.imag(), transmitted.Dx.imag(), transmitted.Dy.imag());

        dirs.iiR  = iiR;
        dirs.iiI  = iiI;
        dirs.is1  = ws;
        dirs.it1  = wrp;
        dirs.is2  = ws;
        dirs.it2  = wtp;
        
        dirs.fiR  = dirs.iiR / n1sq;
        dirs.fiI  = dirs.iiI / n1sq;
        dirs.fs1  = dirs.is1 / n1sq;
        dirs.ft1  = dirs.it1 / n1sq;
        dirs.fs2  = dirs.is2 / n2sq;
        dirs.ft2  = dirs.it2 / n2sq;

        dirs.giR  = dirs.iiR.cross(ui) / n1;
        dirs.giI  = dirs.iiI.cross(ui) / n1;
        dirs.gs1  = dirs.is1.cross(uo1) / n1;
        dirs.gt1  = dirs.it1.cross(uo1) / n1;
        dirs.gs2  = dirs.is2.cross(uo2) / n2;
        dirs.gt2  = dirs.it2.cross(uo2) / n2;
      }
    }

    // Solve fields for the anisotropic-to-anisotropic case
    inline void
    solveAnisoAniso(EMFields &ro, EMFields &re, EMFields &to, EMFields &te)
    {
      auto no1 = m1->no;
      auto no2 = m2->no;

      // Directions of the electric displacement vectors
      auto io1 = uo1.cross(ax1).normalized();
      auto ie1 = ue1.cross(ax1).cross(ue1).normalized();
      auto io2 = uo2.cross(ax2);
      auto ie2 = ue2.cross(ax2).cross(ue2).normalized();

      // D-to-E vectors
      auto fiR = iep1 * iiR;
      auto fiI = iep1 * iiI;
      auto fo1 = iep1 * io1;
      auto fe1 = iep1 * ie1;
      auto fo2 = iep2 * io2;
      auto fe2 = iep2 * ie2;

      // D-to-H vectors
      auto giR = iiR.cross(ui)  / ni;
      auto giI = iiI.cross(ui)  / ni;
      auto go1 = io1.cross(uo1) / no1;
      auto ge1 = ie1.cross(ue1) / nee1;
      auto go2 = io2.cross(uo2) / no2;
      auto ge2 = ie2.cross(ue2) / nee2;

      // System matrix
      auto M = Matrix4(
        Vec4(wq * fo1, wq * fe1, -wq * fo2, -wq * fe2),
        Vec4(ws * fo1, ws * fe1, -ws * fo2, -ws * fe2),
        Vec4(wq * go1, wq * ge1, -wq * go2, -wq * ge2),
        Vec4(ws * go1, ws * ge1, -ws * go2, -ws * ge2));

      Matrix4 invM;
      if (!M.invert(invM))
        throw std::runtime_error("Failed to invert electric displacement matrix.");

      // Independent term(s) are calculated from the incident field
      auto EiR = DampR * fiR;
      auto EiI = DampI * fiI;

      auto HiR = DampR * giR;
      auto HiI = DampI * giI;

      auto bR = Vec4(-wq * EiR, -ws * EiR, -wq * HiR, -ws * HiR);
      auto bI = Vec4(-wq * EiI, -ws * EiI, -wq * HiI, -ws * HiI);

      // Calculate fields and save them
      auto DR = invM * bR; // Real amplitudes of Do1, De1, Do2, De2
      auto DI = invM * bI; // Imaginary amplitudes of Do1, De1, Do2, De2

      ro.vDx = io1;
      ro.vDy = uo1.cross(io1);
      ro.Dx  = Complex(DR.coords[0], DI.coords[0]);
      ro.Dy  = Complex(0., 0.);

      re.vDx = ie1;
      re.vDy = ue1.cross(ie1);
      re.Dx  = Complex(DR.coords[1], DI.coords[1]);
      re.Dy  = Complex(0., 0.);

      to.vDx = io2;
      to.vDy = uo2.cross(io2);
      to.Dx  = Complex(DR.coords[2], DI.coords[2]);
      to.Dy  = Complex(0., 0.);

      te.vDx = ie2;
      te.vDy = ue2.cross(ie2);
      te.Dx  = Complex(DR.coords[3], DI.coords[3]);
      te.Dy  = Complex(0., 0.);
      
      // Copy debug vectors, if requested to do so
      if (debug) {
        dirs.DiR = DReal;
        dirs.DiI = DImag;

        dirs.DR  = DR;
        dirs.DI  = DI;

        COPYDIR(iR); COPYDIR(iI);
        COPYDIR(o1); COPYDIR(e1);
        COPYDIR(o2); COPYDIR(e2);
      }
    }

    // Solve fields for the anisotropic-to-isotropic case. We rely on the
    // ws, wt, u triad for field directions of the isotropic ray
    inline void
    solveAnisoIso(EMFields &ro, EMFields &re, EMFields &transmitted)
    {
      auto no1  = m1->no;
      auto n2   = m2->n;
      auto iep2 = 1. / (n2 * n2);

      // Directions of the electric displacement vectors
      auto io1 = uo1.cross(ax1).normalized();
      auto ie1 = ue1.cross(ax1).cross(ue1).normalized();
      auto is2 = ws;
      auto it2 = uo2.cross(is2);

      // D-to-E vectors
      auto fiR = iep1 * iiR;
      auto fiI = iep1 * iiI;
      auto fo1 = iep1 * io1;
      auto fe1 = iep1 * ie1;
      auto fs2 = iep2 * is2;
      auto ft2 = iep2 * it2;

      // D-to-H vectors
      auto giR = iiR.cross(ui)  / ni;
      auto giI = iiI.cross(ui)  / ni;
      auto go1 = io1.cross(uo1) / no1;
      auto ge1 = ie1.cross(ue1) / nee1;
      auto gs2 = is2.cross(uo2) / n2;
      auto gt2 = it2.cross(uo2) / n2;

      // System matrix
      auto M = Matrix4(
        Vec4(wq * fo1, wq * fe1,        0.,  -wq * ft2),
        Vec4(ws * fo1, ws * fe1, -ws * fs2,         0.),
        Vec4(wq * go1, wq * ge1, -wq * gs2,         0.),
        Vec4(ws * go1, ws * ge1,        0.,  -ws * gt2));
      
      Matrix4 invM;
      if (!M.invert(invM))
        throw std::runtime_error("Failed to invert electric displacement matrix.");

      // Independent term(s) are calculated from the incident field
      auto EiR = DampR * fiR;
      auto EiI = DampI * fiI;

      auto HiR = DampR * giR;
      auto HiI = DampI * giI;

      auto bR = Vec4(-wq * EiR, -ws * EiR, -wq * HiR, -ws * HiR);
      auto bI = Vec4(-wq * EiI, -ws * EiI, -wq * HiI, -ws * HiI);

      // Calculate fields and save them
      auto DR = invM * bR; // Real amplitudes of      Do1, De1, Ds2, Dt2
      auto DI = invM * bI; // Imaginary amplitudes of Do1, De1, Ds2, Dt2

      ro.vDx = io1;
      ro.vDy = uo1.cross(io1);
      ro.Dx  = Complex(DR.coords[0], DI.coords[0]);
      ro.Dy  = Complex(0., 0.);

      re.vDx = ie1;
      re.vDy = ue1.cross(ie1);
      re.Dx  = Complex(DR.coords[1], DI.coords[1]);
      re.Dy  = Complex(0., 0.);

      transmitted.vDx = is2;
      transmitted.vDy = it2;
      transmitted.Dx  = Complex(DR.coords[2], DI.coords[2]);
      transmitted.Dy  = Complex(DR.coords[3], DI.coords[3]);

      // Copy debug vectors, if requested to do so
      if (debug) {
        dirs.DiR = DReal;
        dirs.DiI = DImag;

        dirs.DR  = DR;
        dirs.DI  = DI;

        COPYDIR(iR); COPYDIR(iI);
        COPYDIR(o1); COPYDIR(e1);
        COPYDIR(s2); COPYDIR(t2);
      }
    }

    // Solve fields for the isotropic-to-anisotropic case
    inline void
    solveIsoAniso(EMFields &reflected, EMFields &to, EMFields &te)
    {
      auto n1   = m1->n;
      auto iep1 = 1. / (n1 * n1);
      auto no2  = m2->no;
      
      // Directions of the electric displacement vectors
      auto is1 = ws;
      auto it1 = uo1.cross(is1);
      auto io2 = uo2.cross(ax2);
      auto ie2 = ue2.cross(ax2).cross(ue2).normalized();

      // D-to-E vectors
      auto fiR = iep1 * iiR;
      auto fiI = iep1 * iiI;
      auto fs1 = iep1 * is1;
      auto ft1 = iep1 * it1;
      auto fo2 = iep2 * io2;
      auto fe2 = iep2 * ie2;

      // D-to-H vectors
      auto giR = iiR.cross(ui)  / ni;
      auto giI = iiI.cross(ui)  / ni;
      auto gs1 = is1.cross(uo1) / n1;
      auto gt1 = it1.cross(uo1) / n1;
      auto go2 = io2.cross(uo2) / no2;
      auto ge2 = ie2.cross(ue2) / nee2;

      // System matrix
      auto M = Matrix4(
        Vec4(       0, wq * ft1, -wq * fo2, -wq * fe2),
        Vec4(ws * fs1,        0, -ws * fo2, -ws * fe2),
        Vec4(wq * gs1,        0, -wq * go2, -wq * ge2),
        Vec4(       0, ws * gt1, -ws * go2, -ws * ge2));
      
      Matrix4 invM;
      if (!M.invert(invM))
        throw std::runtime_error("Failed to invert electric displacement matrix.");

      // Independent term(s) are calculated from the incident field
      auto EiR = DampR * fiR;
      auto EiI = DampI * fiI;

      auto HiR = DampR * giR;
      auto HiI = DampI * giI;

      auto bR = Vec4(-wq * EiR, -ws * EiR, -wq * HiR, -ws * HiR);
      auto bI = Vec4(-wq * EiI, -ws * EiI, -wq * HiI, -ws * HiI);

      // Calculate fields and save them
      auto DR = invM * bR; // Real amplitudes of Ds1, Dt1, Do2, De2
      auto DI = invM * bI; // Imaginary amplitudes of Ds1, Dt1, Do2, De2

      reflected.vDx = is1;
      reflected.vDy = it1;
      reflected.Dx  = Complex(DR.coords[0], DI.coords[0]);
      reflected.Dy  = Complex(DR.coords[1], DI.coords[1]);

      to.vDx = io2;
      to.vDy = uo2.cross(io2);
      to.Dx  = Complex(DR.coords[2], DI.coords[2]);
      to.Dy  = Complex(0., 0.);

      te.vDx = ie2;
      te.vDy = ue2.cross(ie2);
      te.Dx  = Complex(DR.coords[3], DI.coords[3]);
      te.Dy  = Complex(0., 0.);

      // Copy debug vectors, if requested to do so
      if (debug) {
        dirs.DiR = DReal;
        dirs.DiI = DImag;

        dirs.DR  = DR;
        dirs.DI  = DI;

        COPYDIR(iR); COPYDIR(iI);
        COPYDIR(s1); COPYDIR(t1);
        COPYDIR(o2); COPYDIR(e2);
      }
    }
  };

  // Other helper functions

  static inline void
  calcEdirfromDdir(
    Vec3 &fx, Vec3 &fy,
    Matrix3 &ieps,
    Vec3 const &vDx, Vec3 const &vDy,
    Vec3 const &direction,
    const EMMedium *medium,
    const EMMedium *&prevMedium,
    const ReferenceFrame *frame)
  {
    Real insq;

    switch (medium->type) {
      case EMMediumVacuum:
        fx = vDx;
        fy = vDy;
        break;

      case EMMediumIsotropic:
        insq = 1 / (medium->n * medium->n);
        fx = vDx * insq;
        fy = vDy * insq;
        break;

      case EMMediumUniaxial:
        // E = eps^{-1} D
        if (prevMedium != medium) {
          prevMedium = medium;

          if (frame == nullptr)
            medium->ieps(ieps);
          else
            medium->ieps(ieps, frame);
        }

        fx  = ieps * vDx;
        fy  = ieps * vDy;
        break;
    }
  }

  static inline void
  calcPoyntingVector(
    Vec3 &S,
    Matrix3 &ieps,
    Complex Dx, Complex Dy,
    Vec3 const &vDx, Vec3 const &vDy,
    Vec3 const &direction,
    const EMMedium *medium,
    const EMMedium *&prevMedium,
    const ReferenceFrame *frame)
  {
    Real DxR, DyR;
    Real DxI, DyI;
    
    Vec3 fx,  fy;
    Vec3 gx,  gy;
    Vec3 SR, SI;

    Real invn3;

    switch (medium->type) {
      case EMMediumVacuum:
        // n = 1, eps0 = 1, then E = D
        S = ((Dx * std::conj(Dx) + Dy * std::conj(Dy)).real()) * direction;
        break;

      case EMMediumIsotropic:
        // D = n^2 eps0 E -> E = D/(eps0 n^2)
        // H \propto D / n
        // |S| = |E x H| \propto D/n^3
        
        invn3 = 1 / (medium->n * medium->n * medium->n);

        S = invn3 *((Dx * std::conj(Dx) + Dy * std::conj(Dy)).real()) * direction;
        break;

      case EMMediumUniaxial:
        // E = eps^{-1} D
        if (prevMedium != medium) {
          prevMedium = medium;

          if (frame == nullptr)
            medium->ieps(ieps);
          else
            medium->ieps(ieps, frame);
        }

        fx  = ieps * vDx;
        fy  = ieps * vDy;

        gx  = direction.cross(vDx);
        gy  = direction.cross(vDy);
        
        DxR = Dx.real();
        DyR = Dy.real();

        DxI = Dx.imag();
        DyI = Dy.imag();

        auto SR  = (DxR * fx + DyR * fy).cross(DxR * gx + DyR * gy);
        auto SI  = (DxI * fx + DyI * fy).cross(DxI * gx + DyI * gy);

        S = SR + SI;
        break;
    }
  }


}

#undef COPYDIR
#endif // _EM_FIELDS_EM_SOLVE_H
