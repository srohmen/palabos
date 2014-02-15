/* This file is part of the Palabos library.
 *
 * Copyright (C) 2011-2013 FlowKit Sarl
 * Route d'Oron 2
 * 1010 Lausanne, Switzerland
 * E-mail contact: contact@flowkit.com
 *
 * The most recent release of Palabos can be downloaded at 
 * <http://www.palabos.org/>
 *
 * The library Palabos is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * The library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Orestis Malaspinas designed some of the classes and concepts contained
 * in this file. */

#ifndef SMAGORINSKY_DYNAMICS_H
#define SMAGORINSKY_DYNAMICS_H

#include "core/globalDefs.h"
#include "core/hierarchicSerializer.h"
#include "basicDynamics/isoThermalDynamics.h"
#include "basicDynamics/externalForceDynamics.h"
#include "complexDynamics/variableOmegaDynamics.h"
#include "complexDynamics/mrtDynamics.h"

namespace plb {

template<typename T, template<typename U> class Descriptor>
class SmagorinskyDynamics : public OmegaFromPiDynamics<T,Descriptor> {
public:
    SmagorinskyDynamics(Dynamics<T,Descriptor>* baseDynamics_, T omega0_, T cSmago_,
                        bool automaticPrepareCollision=true);
    SmagorinskyDynamics(HierarchicUnserializer& unserializer);
    /// Clone the object on its dynamic type.
    SmagorinskyDynamics<T,Descriptor>* clone() const;
    /// Return a unique ID for this class.
    virtual int getId() const;
    /// Serialize the dynamics object.
    virtual void serialize(HierarchicSerializer& serializer) const;
    /// Un-Serialize the dynamics object.
    virtual void unserialize(HierarchicUnserializer& unserializer);
    /// Modify the value of omega, using the Smagorinsky algorithm based on omega0.
    virtual T getOmegaFromPiAndRhoBar(Array<T,SymmetricTensor<T,Descriptor>::n> const& PiNeq, T rhoBar) const;
    /// With this method, you can modify the constant value omega0 (not the actual value of omega,
    ///  which is computed during run-time from omega0 and the local strain-rate).
    virtual void setOmega(T omega_);
    /// Returns omega0.
    virtual T getOmega() const;
private:
    T omega0;    //< "Laminar" relaxation parameter, used when the strain-rate is zero.
    T cSmago;    //< Smagorinsky constant.
    T preFactor; //< A factor depending on the Smagorinky constant, used to compute the effective viscosity.
    static int id;
};

template<typename T, template<typename U> class Descriptor>
class SmagorinskyBGKdynamics : public IsoThermalBulkDynamics<T,Descriptor> {
public:
/* *************** Construction / Destruction ************************ */
    SmagorinskyBGKdynamics(T omega0_, T cSmago_);
    SmagorinskyBGKdynamics(HierarchicUnserializer& unserializer);
    /// Clone the object on its dynamic type.
    virtual SmagorinskyBGKdynamics<T,Descriptor>* clone() const;
    /// Return a unique ID for this class.
    virtual int getId() const;
    /// Serialize the dynamics object.
    virtual void serialize(HierarchicSerializer& serializer) const;
    /// Un-Serialize the dynamics object.
    virtual void unserialize(HierarchicUnserializer& unserializer);
    /// Implementation of the collision step
    virtual void collide(Cell<T,Descriptor>& cell,
                         BlockStatistics& statistics_);
    /// Implementation of the collision step, with imposed macroscopic variables
    virtual void collideExternal(Cell<T,Descriptor>& cell, T rhoBar,
                         Array<T,Descriptor<T>::d> const& j, T thetaBar, BlockStatistics& stat);
    /// Compute equilibrium distribution function
    virtual T computeEquilibrium(plint iPop, T rhoBar, Array<T,Descriptor<T>::d> const& j,
                                 T jSqr, T thetaBar=T()) const;
    /// With this method, you can modify the constant value omega0 (not the actual value of omega,
    ///  which is computed during run-time from omega0 and the local strain-rate).
    virtual void setOmega(T omega_);
    /// Returns omega0.
    virtual T getOmega() const;
private:
    T omega0;    //< "Laminar" relaxation parameter, used when the strain-rate is zero.
    T cSmago;    //< Smagorinsky constant.
    T preFactor; //< A factor depending on the Smagorinky constant, used to compute the effective viscosity.
    static int id;
};

template<typename T, template<typename U> class Descriptor>
class GuoExternalForceSmagorinskyBGKdynamics : public IsoThermalBulkDynamics<T,Descriptor> {
public:
/* *************** Construction / Destruction ************************ */
    GuoExternalForceSmagorinskyBGKdynamics(T omega0_, T cSmago_);
    GuoExternalForceSmagorinskyBGKdynamics(HierarchicUnserializer& unserializer);
    /// Clone the object on its dynamic type.
    virtual GuoExternalForceSmagorinskyBGKdynamics<T,Descriptor>* clone() const;
    /// Return a unique ID for this class.
    virtual int getId() const;
    /// Serialize the dynamics object.
    virtual void serialize(HierarchicSerializer& serializer) const;
    /// Un-Serialize the dynamics object.
    virtual void unserialize(HierarchicUnserializer& unserializer);
    /// Implementation of the collision step
    virtual void collide(Cell<T,Descriptor>& cell,
                         BlockStatistics& statistics_);
    /// Implementation of the collision step, with imposed macroscopic variables
    virtual void collideExternal(Cell<T,Descriptor>& cell, T rhoBar,
                         Array<T,Descriptor<T>::d> const& j, T thetaBar, BlockStatistics& stat);
    /// Compute the physical velocity
    virtual void computeVelocity (Cell<T,Descriptor> const& cell,
                                  Array<T,Descriptor<T>::d>& u ) const;
    /// Compute equilibrium distribution function
    virtual T computeEquilibrium(plint iPop, T rhoBar, Array<T,Descriptor<T>::d> const& j,
                                 T jSqr, T thetaBar=T()) const;
    /// With this method, you can modify the constant value omega0 (not the actual value of omega,
    ///  which is computed during run-time from omega0 and the local strain-rate).
    virtual void setOmega(T omega_);
    /// Returns omega0.
    virtual T getOmega() const;
    /// Set local value of any generic parameter.
    /// For the Smagorinsky constant cSmago, use dynamicParams::smagorinskyConstant.
    virtual void setParameter(plint whichParameter, T value);
private:
    T omega0;    //< "Laminar" relaxation parameter, used when the strain-rate is zero.
    T cSmago;    //< Smagorinsky constant.
    T preFactor; //< A factor depending on the Smagorinky constant, used to compute the effective viscosity.
    static int id;
};

template<typename T, template<typename U> class Descriptor>
class SmagorinskyIncBGKdynamics : public IsoThermalBulkDynamics<T,Descriptor> {
public:
/* *************** Construction / Destruction ************************ */
    SmagorinskyIncBGKdynamics(T omega0_, T cSmago_, T rho0=(T)1 );
    SmagorinskyIncBGKdynamics(HierarchicUnserializer& unserializer);
    /// Clone the object on its dynamic type.
    virtual SmagorinskyIncBGKdynamics<T,Descriptor>* clone() const;
    /// Return a unique ID for this class.
    virtual int getId() const;
    /// Serialize the dynamics object.
    virtual void serialize(HierarchicSerializer& serializer) const;
    /// Un-Serialize the dynamics object.
    virtual void unserialize(HierarchicUnserializer& unserializer);
    /// Implementation of the collision step
    virtual void collide(Cell<T,Descriptor>& cell,
                         BlockStatistics& statistics_);
    /// Implementation of the collision step, with imposed macroscopic variables
    virtual void collideExternal(Cell<T,Descriptor>& cell, T rhoBar,
                         Array<T,Descriptor<T>::d> const& j, T thetaBar, BlockStatistics& stat);
    /// Compute the physical velocity
    virtual void computeVelocity (Cell<T,Descriptor> const& cell,
                                  Array<T,Descriptor<T>::d>& u ) const;
    /// Compute equilibrium distribution function
    virtual T computeEquilibrium(plint iPop, T rhoBar, Array<T,Descriptor<T>::d> const& j,
                                 T jSqr, T thetaBar=T()) const;
    /// With this method, you can modify the constant value omega0 (not the actual value of omega,
    ///  which is computed during run-time from omega0 and the local strain-rate).
    virtual void setOmega(T omega_);
    /// Returns omega0.
    virtual T getOmega() const;

    /// Get local value of any generic parameter.
    /// For the density rho0, use parameter 110.
    /// For the Smagorinsky constant cSmago, use dynamicParams::smagorinskyConstant.
    virtual T getParameter(plint whichParameter) const;

    /// Set local value of any generic parameter.
    /// For the density rho0, use parameter 110.
    /// For the Smagorinsky constant cSmago, use dynamicParams::smagorinskyConstant.
    virtual void setParameter(plint whichParameter, T value);
    ///  Is it in the incompressible BGK model
    virtual bool velIsJ() const;
private:
    T omega0;    //< "Laminar" relaxation parameter, used when the strain-rate is zero.
    T cSmago;    //< Smagorinsky constant.
    T preFactor; //< A factor depending on the Smagorinky constant, used to compute the effective viscosity.
    T invRho0;
    static int id;
};

/// (Jonas) Attention: This class is highly suspicious. I copy-pasted it from
/// GuoExternalForceSmagorinskyBGKdynamics with some adaptations, but it needs
/// to be checked more carefully.
template<typename T, template<typename U> class Descriptor>
class IncGuoExternalForceSmagorinskyBGKdynamics : public IsoThermalBulkDynamics<T,Descriptor> {
public:
/* *************** Construction / Destruction ************************ */
    IncGuoExternalForceSmagorinskyBGKdynamics(T omega0_, T cSmago_, T rho0=(T)1 );
    IncGuoExternalForceSmagorinskyBGKdynamics(HierarchicUnserializer& unserializer);
    /// Clone the object on its dynamic type.
    virtual IncGuoExternalForceSmagorinskyBGKdynamics<T,Descriptor>* clone() const;
    /// Return a unique ID for this class.
    virtual int getId() const;
    /// Serialize the dynamics object.
    virtual void serialize(HierarchicSerializer& serializer) const;
    /// Un-Serialize the dynamics object.
    virtual void unserialize(HierarchicUnserializer& unserializer);
    /// Implementation of the collision step
    virtual void collide(Cell<T,Descriptor>& cell,
                         BlockStatistics& statistics_);
    /// Implementation of the collision step, with imposed macroscopic variables
    virtual void collideExternal(Cell<T,Descriptor>& cell, T rhoBar,
                         Array<T,Descriptor<T>::d> const& j, T thetaBar, BlockStatistics& stat);
    /// Compute the physical velocity
    virtual void computeVelocity (Cell<T,Descriptor> const& cell,
                                  Array<T,Descriptor<T>::d>& u ) const;
    /// Compute equilibrium distribution function
    virtual T computeEquilibrium(plint iPop, T rhoBar, Array<T,Descriptor<T>::d> const& j,
                                 T jSqr, T thetaBar=T()) const;
    /// With this method, you can modify the constant value omega0 (not the actual value of omega,
    ///  which is computed during run-time from omega0 and the local strain-rate).
    virtual void setOmega(T omega_);
    /// Returns omega0.
    virtual T getOmega() const;

    /// Get local value of any generic parameter.
    /// For the density rho0, use parameter 110.
    /// For the Smagorinsky constant cSmago, use dynamicParams::smagorinskyConstant.
    virtual T getParameter(plint whichParameter) const;

    /// Set local value of any generic parameter.
    /// For the density rho0, use parameter 110.
    /// For the Smagorinsky constant cSmago, use dynamicParams::smagorinskyConstant.
    virtual void setParameter(plint whichParameter, T value);
    ///  Is it in the incompressible BGK model
    virtual bool velIsJ() const;
private:
    T omega0;    //< "Laminar" relaxation parameter, used when the strain-rate is zero.
    T cSmago;    //< Smagorinsky constant.
    T invRho0;
    static int id;
};

template<typename T, template<typename U> class Descriptor>
class SmagorinskyRegularizedDynamics : public IsoThermalBulkDynamics<T,Descriptor> {
public:
    SmagorinskyRegularizedDynamics(T omega0_, T cSmago_);

    /// Clone the object on its dynamic type.
    virtual SmagorinskyRegularizedDynamics<T,Descriptor>* clone() const;
    /// Return a unique ID for this class.
    virtual int getId() const;
    /// Serialize the dynamics object.
    virtual void serialize(HierarchicSerializer& serializer) const;
    /// Un-Serialize the dynamics object.
    virtual void unserialize(HierarchicUnserializer& unserializer);
    /// Implementation of the collision step
    virtual void collide(Cell<T,Descriptor>& cell,
                         BlockStatistics& statistics_);
    /// Implementation of the collision step, with imposed macroscopic variables
    virtual void collideExternal(Cell<T,Descriptor>& cell, T rhoBar,
                         Array<T,Descriptor<T>::d> const& j, T thetaBar, BlockStatistics& stat);
    /// Compute equilibrium distribution function
    virtual T computeEquilibrium(plint iPop, T rhoBar, Array<T,Descriptor<T>::d> const& j,
                                 T jSqr, T thetaBar=T()) const;
    /// With this method, you can modify the constant value omega0 (not the actual value of omega,
    ///  which is computed during run-time from omega0 and the local strain-rate).
    virtual void setOmega(T omega_);
    /// Returns omega0.
    virtual T getOmega() const;
private:
    T omega0;    //< "Laminar" relaxation parameter, used when the strain-rate is zero.
    T cSmago;    //< Smagorinsky constant.
    T preFactor; //< A factor depending on the Smagorinky constant, used to compute the effective viscosity.
    static int id;
};

template<typename T, template<typename U> class Descriptor>
class SecuredSmagorinskyRegularizedDynamics : public IsoThermalBulkDynamics<T,Descriptor> {
public:
    SecuredSmagorinskyRegularizedDynamics(T omega0_, T cSmago_);
    SecuredSmagorinskyRegularizedDynamics(HierarchicUnserializer& unserializer);

    /// Clone the object on its dynamic type.
    virtual SecuredSmagorinskyRegularizedDynamics<T,Descriptor>* clone() const;
    /// Return a unique ID for this class.
    virtual int getId() const;
    /// Serialize the dynamics object.
    virtual void serialize(HierarchicSerializer& serializer) const;
    /// Un-Serialize the dynamics object.
    virtual void unserialize(HierarchicUnserializer& unserializer);
    /// Implementation of the collision step
    virtual void collide(Cell<T,Descriptor>& cell,
                         BlockStatistics& statistics_);
    /// Implementation of the collision step, with imposed macroscopic variables
    virtual void collideExternal(Cell<T,Descriptor>& cell, T rhoBar,
                         Array<T,Descriptor<T>::d> const& j, T thetaBar, BlockStatistics& stat);
    /// Compute equilibrium distribution function
    virtual T computeEquilibrium(plint iPop, T rhoBar, Array<T,Descriptor<T>::d> const& j,
                                 T jSqr, T thetaBar=T()) const;
    /// With this method, you can modify the constant value omega0 (not the actual value of omega,
    ///  which is computed during run-time from omega0 and the local strain-rate).
    virtual void setOmega(T omega_);
    /// Returns omega0.
    virtual T getOmega() const;
private:
    inline static void constrainValue(T& value, T softLimit, T hardLimit);
private:
    T omega0;    //< "Laminar" relaxation parameter, used when the strain-rate is zero.
    T cSmago;    //< Smagorinsky constant.
    T preFactor; //< A factor depending on the Smagorinky constant, used to compute the effective viscosity.
    static int id;
};

/// Implementation of the MRT collision step
template<typename T, template<typename U> class Descriptor>
class SmagorinskyMRTdynamics : public IsoThermalBulkDynamics<T,Descriptor> {
public:
/* *************** Construction / Destruction ************************ */
    SmagorinskyMRTdynamics(plint externalParam_, T cSmago_);
    SmagorinskyMRTdynamics(MRTparam<T,Descriptor>* param_, T cSmago_);
    SmagorinskyMRTdynamics(HierarchicUnserializer& unserializer);
    SmagorinskyMRTdynamics(SmagorinskyMRTdynamics<T,Descriptor> const& rhs);
    SmagorinskyMRTdynamics<T,Descriptor>& operator=(SmagorinskyMRTdynamics<T,Descriptor> const& rhs);
    virtual ~SmagorinskyMRTdynamics<T,Descriptor>();
    void swap(SmagorinskyMRTdynamics<T,Descriptor>& rhs);

    /// Clone the object on its dynamic type.
    virtual SmagorinskyMRTdynamics<T,Descriptor>* clone() const;

    /// Return a unique ID for this class.
    virtual int getId() const;

    /// Serialize the dynamics object.
    virtual void serialize(HierarchicSerializer& serializer) const;
    /// Un-Serialize the dynamics object.
    virtual void unserialize(HierarchicUnserializer& unserializer);

/* *************** Collision and Equilibrium ************************* */

    /// Implementation of the collision step
    virtual void collide(Cell<T,Descriptor>& cell,
                         BlockStatistics& statistics_);
    /// Implementation of the collision step, with imposed macroscopic variables
    virtual void collideExternal (
            Cell<T,Descriptor>& cell, T rhoBar,
            Array<T,Descriptor<T>::d> const& j, T thetaBar, BlockStatistics& stat);

    /// Compute equilibrium distribution function
    virtual T computeEquilibrium(plint iPop, T rhoBar, Array<T,Descriptor<T>::d> const& j,
                                 T jSqr, T thetaBar=T()) const;

    MRTparam<T,Descriptor> const& getMrtParameter() const;
private:
    MRTparam<T,Descriptor>* param;
    plint externalParam;
    T cSmago;
    static int id;
};

/// Implementation of the quasi incompressible Smagorinsky MRT collision step
template<typename T, template<typename U> class Descriptor>
class ConsistentSmagorinskyMRTdynamics : public IsoThermalBulkDynamics<T,Descriptor> {
public:
/* *************** Construction / Destruction ************************ */
    ConsistentSmagorinskyMRTdynamics(plint externalParam_, T cSmago_);
    ConsistentSmagorinskyMRTdynamics(MRTparam<T,Descriptor>* param_, T cSmago_);
    ConsistentSmagorinskyMRTdynamics(HierarchicUnserializer& unserializer);
    ConsistentSmagorinskyMRTdynamics(ConsistentSmagorinskyMRTdynamics<T,Descriptor> const& rhs);
    ConsistentSmagorinskyMRTdynamics<T,Descriptor>& operator=(ConsistentSmagorinskyMRTdynamics<T,Descriptor> const& rhs);
    virtual ~ConsistentSmagorinskyMRTdynamics<T,Descriptor>();
    void swap(ConsistentSmagorinskyMRTdynamics<T,Descriptor>& rhs);

    /// Clone the object on its dynamic type.
    virtual ConsistentSmagorinskyMRTdynamics<T,Descriptor>* clone() const;

    /// Return a unique ID for this class.
    virtual int getId() const;

    /// Serialize the dynamics object.
    virtual void serialize(HierarchicSerializer& serializer) const;
    /// Un-Serialize the dynamics object.
    virtual void unserialize(HierarchicUnserializer& unserializer);

/* *************** Collision and Equilibrium ************************* */

    /// Implementation of the collision step
    virtual void collide(Cell<T,Descriptor>& cell,
                         BlockStatistics& statistics_);
    /// Implementation of the collision step, with imposed macroscopic variables
    virtual void collideExternal (
            Cell<T,Descriptor>& cell, T rhoBar,
            Array<T,Descriptor<T>::d> const& j, T thetaBar, BlockStatistics& stat);

    /// Compute equilibrium distribution function
    virtual T computeEquilibrium(plint iPop, T rhoBar, Array<T,Descriptor<T>::d> const& j,
                                 T jSqr, T thetaBar=T()) const;

    MRTparam<T,Descriptor> const& getMrtParameter() const;
private:
    MRTparam<T,Descriptor>* param;
    plint externalParam;
    T cSmago;
    static int id;
};

/// Implementation of the quasi incompressible Smagorinsky MRT collision step
template<typename T, template<typename U> class Descriptor>
class ConsistentSmagorinskyQuasiIncMRTdynamics : public IsoThermalBulkDynamics<T,Descriptor> {
public:
/* *************** Construction / Destruction ************************ */
    ConsistentSmagorinskyQuasiIncMRTdynamics(plint externalParam_, T cSmago_);
    ConsistentSmagorinskyQuasiIncMRTdynamics(MRTparam<T,Descriptor>* param_, T cSmago_);
    ConsistentSmagorinskyQuasiIncMRTdynamics(HierarchicUnserializer& unserializer);
    ConsistentSmagorinskyQuasiIncMRTdynamics(ConsistentSmagorinskyQuasiIncMRTdynamics<T,Descriptor> const& rhs);
    ConsistentSmagorinskyQuasiIncMRTdynamics<T,Descriptor>& operator=(ConsistentSmagorinskyQuasiIncMRTdynamics<T,Descriptor> const& rhs);
    virtual ~ConsistentSmagorinskyQuasiIncMRTdynamics<T,Descriptor>();
    void swap(ConsistentSmagorinskyQuasiIncMRTdynamics<T,Descriptor>& rhs);

    /// Clone the object on its dynamic type.
    virtual ConsistentSmagorinskyQuasiIncMRTdynamics<T,Descriptor>* clone() const;

    /// Return a unique ID for this class.
    virtual int getId() const;

    /// Serialize the dynamics object.
    virtual void serialize(HierarchicSerializer& serializer) const;
    /// Un-Serialize the dynamics object.
    virtual void unserialize(HierarchicUnserializer& unserializer);

/* *************** Collision and Equilibrium ************************* */

    /// Implementation of the collision step
    virtual void collide(Cell<T,Descriptor>& cell,
                         BlockStatistics& statistics_);
    /// Implementation of the collision step, with imposed macroscopic variables
    virtual void collideExternal (
            Cell<T,Descriptor>& cell, T rhoBar,
            Array<T,Descriptor<T>::d> const& j, T thetaBar, BlockStatistics& stat);

    /// Compute equilibrium distribution function
    virtual T computeEquilibrium(plint iPop, T rhoBar, Array<T,Descriptor<T>::d> const& j,
                                 T jSqr, T thetaBar=T()) const;

    MRTparam<T,Descriptor> const& getMrtParameter() const;
private:
    MRTparam<T,Descriptor>* param;
    plint externalParam;
    T cSmago;
    static int id;
};

} // namespace plb

#endif  // SMAGORINSKY_DYNAMICS_H

