/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 RC 1        *
*                (c) 2006-2011 INRIA, USTL, UJF, CNRS, MGH                    *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU General Public License as published by the Free  *
* Software Foundation; either version 2 of the License, or (at your option)   *
* any later version.                                                          *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for    *
* more details.                                                               *
*                                                                             *
* You should have received a copy of the GNU General Public License along     *
* with this program; if not, write to the Free Software Foundation, Inc., 51  *
* Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.                   *
*******************************************************************************
*                            SOFA :: Applications                             *
*                                                                             *
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/


#include <sofa/component/interactionforcefield/StiffSpringForceField.h>
#include "ForceField_test.h"
#include <sofa/defaulttype/RigidTypes.h>
//#include <cstdlib>

namespace sofa {

template <class DataTypes>
struct RigidTransform
{
    typedef typename DataTypes::Real Real;
    typedef typename defaulttype::StdRigidTypes<DataTypes::spatial_dimensions,Real>::Coord Transform;
    typedef typename DataTypes::Coord Point;
    typedef typename DataTypes::Deriv Vec;
    Transform transform;

    void randomize( Real amplitude )
    {
        transform = Transform::rand(amplitude);
    }

    void projectPoint( Point& p )
    {
        p = transform.projectPoint(p);
    }

    void projectVector( Vec& v )
    {
        v = transform.projectVector(v);
    }

};


/**  Test suite for StiffSpringForceField.
  *  The test cases are defined in the #Test_Cases member group.
  *
  * @author Francois Faure, 2014
  */
template <typename _StiffSpringForceField>
struct StiffSpringForceField_test : public ForceField_test<_StiffSpringForceField>
{

    typedef _StiffSpringForceField ForceType;
    typedef typename ForceType::DataTypes DataTypes;

    typedef typename ForceType::VecCoord VecCoord;
    typedef typename ForceType::VecDeriv VecDeriv;
    typedef typename ForceType::Coord Coord;
    typedef typename ForceType::Deriv Deriv;
    typedef typename Coord::value_type Real;
    typedef helper::Vec<3,Real> Vec3;

    typedef ForceType Spring;
    typedef component::container::MechanicalObject<DataTypes> DOF;


    /** @name Test_Cases
      For each of these cases, we check if the accurate forces are computed
      */
    ///@{

    /** Two particles
    */
    void test_2particles( Real stiffness, Real dampingRatio, Real restLength,
                          Vec3 x0, Vec3 v0,
                          Vec3 x1, Vec3 v1,
                          Vec3 f0)
    {
        // set position and velocity vectors, using DataTypes::set to cope with tests in dimension 2
        VecCoord x(2);
        DataTypes::set( x[0], x0[0],x0[1],x0[2]);
        DataTypes::set( x[1], x1[0],x1[1],x1[2]);
        VecDeriv v(2);
        DataTypes::set( v[0], v0[0],v0[1],v0[2]);
        DataTypes::set( v[1], v1[0],v1[1],v1[2]);
        VecDeriv f(2);
        DataTypes::set( f[0],  f0[0], f0[1], f0[2]);
        DataTypes::set( f[1], -f0[0],-f0[1],-f0[2]);

        // randomly rotate and translate the scene
        /// @todo functions to apply the transform to VecCoord and VecDeriv
        RigidTransform<DataTypes> transform;
        transform.randomize(1.1);
        transform.projectPoint( x[0]);
        transform.projectVector(v[0]);
        transform.projectPoint( x[1]);
        transform.projectVector(v[1]);
        transform.projectVector(f[0]);
        transform.projectVector(f[1]);


        // tune the force field
        this->force->addSpring(0,1,stiffness,dampingRatio,restLength);

        //        cout<<"test_2particles, x = " << x << endl;
        //        cout<<"                 v = " << v << endl;
        //        cout<<"                 f = " << f << endl;

        // and run the test
        this->run_test( x, v, f );
    }

    ///@}
};



// ========= Define the list of types to instanciate.
//using testing::Types;
typedef testing::Types<
component::interactionforcefield::StiffSpringForceField<defaulttype::Vec2Types>,  // 2D
component::interactionforcefield::StiffSpringForceField<defaulttype::Vec3Types>   // 3D
> TestTypes; // the types to instanciate.



// ========= Tests to run for each instanciated type
TYPED_TEST_CASE(StiffSpringForceField_test, TestTypes);

// first test case: extension, no velocity
TYPED_TEST( StiffSpringForceField_test , extension )
{
    this->errorMax = 1000;
    this->deltaMax = 1000;
    this->debug = false;

    SReal
            k = 1.0,  // stiffness
            d = 0.1,  // damping ratio
            l0 = 1.0; // rest length

    typename TestFixture::Vec3
            x0(0,0,0), // position of the first particle
            v0(0,0,0), // velocity of the first particle
            x1(2,0,0), // position of the second particle
            v1(0,0,0), // velocity of the second particle
            f0(1,0,0); // expected force on the first particle


    // use the parent  class to automatically test the functions
    this->test_2particles(k,d,l0, x0,v0, x1,v1, f0);
}


//// velocity, no extension
TYPED_TEST( StiffSpringForceField_test , viscosity )
{
    this->errorMax = 1000;
    this->deltaMax = 1000;
    this->debug = false;

    SReal
            k = 1.0,  // stiffness
            d = 0.1,  // damping ratio
            l0 = 1.0; // rest length

    typename TestFixture::Vec3
            x0( 0,0,0), // position of the first particle
            v0(-1,0,0), // velocity of the first particle
            x1( 1,0,0), // position of the second particle
            v1( 1,0,0), // velocity of the second particle
            f0(0.2,0,0); // expected force on the first particle

    this->test_2particles(k,d,l0, x0,v0, x1,v1, f0);
}

} // namespace sofa