/*
    File: energyStretch.cc
*/
/*
Open Source License
Copyright (c) 2016, Christian E. Schafmeister
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 
This is an open source license for the CANDO software from Temple University, but it is not the only one. Contact Temple University at mailto:techtransfer@temple.edu if you would like a different license.
*/
/* -^- */
#define	DEBUG_LEVEL_FULL

#include <cando/chem/energyStretch.h>
#include <clasp/core/numerics.h>
#include <cando/chem/energyAtomTable.h>
#include <cando/chem/energyFunction.h>
#include <clasp/core/profiler.h>
#include <cando/chem/matter.h>
#include <cando/chem/bond.h>
#include <cando/chem/atom.h>
#include <cando/chem/residue.h>
#include <cando/chem/aggregate.h>
#include <cando/chem/nVector.h>
#include <cando/chem/ffBaseDb.h>
#include <cando/chem/ffTypesDb.h>
#include <cando/chem/ffStretchDb.h>
#include <cando/chem/ffAngleDb.h>
#include <cando/chem/largeSquareMatrix.h>
#include <clasp/core/wrappers.h>


namespace chem {

    EnergyStretch::EnergyStretch( )
    {
      this->_Atom1 = _Nil<core::T_O>();
      this->_Atom2 = _Nil<core::T_O>();
    }

    EnergyStretch::~EnergyStretch()
    {
    }



#if XML_ARCHIVE
    void	EnergyStretch::archive(core::ArchiveP node)
    {
	node->attribute("kb",this->term.kb);
	node->attribute("r0",this->term.r0);
	node->attribute("I1",this->term.I1);
	node->attribute("I2",this->term.I2);
	node->attribute("a1",this->_Atom1);
	node->attribute("a2",this->_Atom2);
#if TURN_ENERGY_FUNCTION_DEBUG_ON //[
	node->attributeIfDefined("calcForce",this->_calcForce,this->_calcForce);
	node->attributeIfDefined("calcDiagonalHessian",this->_calcDiagonalHessian,this->_calcDiagonalHessian);
	node->attributeIfDefined("calcOffDiagonalHessian",this->_calcOffDiagonalHessian,this->_calcOffDiagonalHessian);
#include <cando/chem/energy_functions/_Stretch_debugEvalSerialize.cc>
#endif //]
    }
#endif



    void EnergyStretch::defineFrom( FFStretch_sp stretch, EnergyAtom *ea1, EnergyAtom *ea2, double scale)
    {
	LOG(BF("Defining EnergyStretch with kb=%lf r0=%lf")
	    % this->term.kb
	    % this->term.r0  );
//    this->_K3 = stretch->_K3;
//    this->_K4 = stretch->_K4;
//    this->_Bci = stretch->_Bci;
	this->_Atom1 = ea1->atom();
	this->_Atom2 = ea2->atom();
	LOG(BF(" Adding stretch between %s - %s")
	    % this->_Atom1->description() % this->_Atom2->description() );
	this->term.I1 = ea1->coordinateIndexTimes3();
	this->term.I2 = ea2->coordinateIndexTimes3();
	this->term.kb = stretch->getKb_kCalPerAngstromSquared();
	this->term.r0 = stretch->getR0_Angstrom();
    }

#if 0
    adapt::QDomNode_sp	EnergyStretch::asXml()
    {
	adapt::QDomNode_sp	node;
	Vector3	vdiff;
	node = adapt::QDomNode_O::create(env,"EnergyStretch");
	node->addAttributeHex("address",(unsigned long)(this));
	node->addAttributeString("atom1Name",this->_Atom1->getName());
	node->addAttributeString("atom2Name",this->_Atom2->getName());
//    node->addAttributeInt("idx1",this->_Atom1->_CoordinateIndex);
//    node->addAttributeInt("idx2",this->_Atom2->_CoordinateIndex);
	node->addAttributeInt("I1",this->term.I1);
	node->addAttributeInt("I2",this->term.I2);
	node->addAttributeDoubleScientific("r0",this->term.r0);
	node->addAttributeDoubleScientific("kb",this->term.kb);
#if TURN_ENERGY_FUNCTION_DEBUG_ON
	adapt::QDomNode_sp xml = adapt::QDomNode_O::create(env,"Evaluated");
	xml->addAttributeBool("calcForce",this->_calcForce );
	xml->addAttributeBool("calcDiagonalHessian",this->_calcDiagonalHessian );
	xml->addAttributeBool("calcOffDiagonalHessian",
			      this->_calcOffDiagonalHessian );


//#include <cando/chem/energy_functions/_Stretch_debugEvalXml.cc>


	node->addChild(xml);
#endif
	return node;
    }
#endif

    void	EnergyStretch::parseFromXmlUsingAtomTable(adapt::QDomNode_sp	xml, AtomTable_sp atomTable )
    {
	int	i1, i2;
	i1 = xml->getAttributeInt("idx1");
	i2 = xml->getAttributeInt("idx2");
	LOG(BF("parse i1=%d i2=%d") % i1 % i2  );
	this->_Atom1 = atomTable->findEnergyAtomWithCoordinateIndex(i1)->atom();
	this->_Atom2 = atomTable->findEnergyAtomWithCoordinateIndex(i2)->atom();
	LOG(BF("_Atom1 = %d  _Atom2 = %d") % i1 % i2  );
	this->term.r0 = xml->getAttributeDouble("r0");
	this->term.kb = xml->getAttributeDouble("kb");
	this->term.I1 = xml->getAttributeInt("I1");
	this->term.I2 = xml->getAttributeInt("I2");
    }



    double	EnergyStretch::getR()
    {
	Vector3	pos1, pos2, diff;
	pos1 = this->getAtom1()->getPosition();
	pos2 = this->getAtom2()->getPosition();
	diff = pos1.sub(pos2);
	return diff.length();
    }




    double	_evaluateEnergyOnly_Stretch (
	double x1,
	double y1,
	double z1,
	double x2,
	double y2,
	double z2,
	double r0,
	double kb )
    {
#undef	STRETCH_SET_PARAMETER
#define	STRETCH_SET_PARAMETER(x)	{}
#undef	STRETCH_SET_POSITION
#define	STRETCH_SET_POSITION(x,ii,of)	{}
#undef	STRETCH_ENERGY_ACCUMULATE
#define	STRETCH_ENERGY_ACCUMULATE(e) {}
#undef	STRETCH_FORCE_ACCUMULATE
#define	STRETCH_FORCE_ACCUMULATE(i,o,v) {}
#undef	STRETCH_DIAGONAL_HESSIAN_ACCUMULATE
#define	STRETCH_DIAGONAL_HESSIAN_ACCUMULATE(i1,o1,i2,o2,v) {}
#undef	STRETCH_OFF_DIAGONAL_HESSIAN_ACCUMULATE
#define	STRETCH_OFF_DIAGONAL_HESSIAN_ACCUMULATE(i1,o1,i2,o2,v) {}
#undef	STRETCH_CALC_FORCE	// Don't calculate FORCE or HESSIAN


#pragma clang diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <cando/chem/energy_functions/_Stretch_termDeclares.cc>
#pragma clang diagnostic pop
#pragma clang diagnostic pop

#if !USE_EXPLICIT_DECLARES
	double fx1 = 0.0; 
	double fy1 = 0.0; 
	double fz1 = 0.0;
	double fx2 = 0.0;
	double fy2 = 0.0;
	double fz2 = 0.0;
#endif
#include <cando/chem/energy_functions/_Stretch_termCode.cc>

	return Energy;
    }




    void	EnergyStretch_O::setupHessianPreconditioner(
	NVector_sp nvPosition,
	AbstractLargeSquareMatrix_sp m )
    {
	bool		calcForce = true;
	bool		calcDiagonalHessian = true;
	bool		calcOffDiagonalHessian = true;
#undef	STRETCH_SET_PARAMETER
#define	STRETCH_SET_PARAMETER(x)	{x = si->term.x;}
#undef	STRETCH_SET_POSITION
#define	STRETCH_SET_POSITION(x,ii,of)	{x = nvPosition->getElement(ii+of);}
#undef	STRETCH_ENERGY_ACCUMULATE
#define	STRETCH_ENERGY_ACCUMULATE(e) {}
#undef	STRETCH_FORCE_ACCUMULATE
#define	STRETCH_FORCE_ACCUMULATE(i,o,v) {}
#undef	STRETCH_DIAGONAL_HESSIAN_ACCUMULATE
#define	STRETCH_DIAGONAL_HESSIAN_ACCUMULATE(i1,o1,i2,o2,v) {	\
	    m->addToElement((i1)+(o1),(i2)+(o2),v);		\
	}
#undef	STRETCH_OFF_DIAGONAL_HESSIAN_ACCUMULATE
#define	STRETCH_OFF_DIAGONAL_HESSIAN_ACCUMULATE(i1,o1,i2,o2,v) {	\
	    m->addToElement((i1)+(o1),(i2)+(o2),v);			\
	}
#define STRETCH_CALC_FORCE
#define STRETCH_CALC_DIAGONAL_HESSIAN
#define STRETCH_CALC_OFF_DIAGONAL_HESSIAN

	if ( this->isEnabled() ) {
#pragma clang diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <cando/chem/energy_functions/_Stretch_termDeclares.cc>
#pragma clang diagnostic pop
#if !USE_EXPLICIT_DECLARES
	    double fx1 = 0.0;
	    double fy1 = 0.0;
	    double fz1 = 0.0;
	    double fx2 = 0.0;
	    double fy2 = 0.0;
	    double fz2 = 0.0;
#endif
	    double x1,y1,z1,x2,y2,z2,kb,r0;
	    int I1, I2;
//	stretchScale = this->getScale();
	    for ( gctools::Vec0<EnergyStretch>::iterator si=this->_Terms.begin();
		  si!=this->_Terms.end(); si++ ) {
#include <cando/chem/energy_functions/_Stretch_termCode.cc>
	    }
	}
    }


CL_DEFMETHOD core::T_sp EnergyStretch_O::stretchTermBetweenAtoms(Atom_sp x, Atom_sp y)
{
  for ( auto si=this->_Terms.begin();
        si!=this->_Terms.end(); si++ ) {
    if ( (si->_Atom1 == x && si->_Atom2 == y) ||
         (si->_Atom1 == y && si->_Atom2 == x ) ) {
      return core::Cons_O::createList(core::Cons_O::create(INTERN_(kw,kb), core::clasp_make_double_float(si->term.kb)),
                                      core::Cons_O::create(INTERN_(kw,r0), core::clasp_make_double_float(si->term.r0)));
    }
  }
  return _Nil<core::T_O>();
}
  
    void EnergyStretch_O::evaluateAll(
	NVector_sp 	pos,
	bool 		calcForce,
	gc::Nilable<NVector_sp> 	force,
	bool		calcDiagonalHessian,
	bool		calcOffDiagonalHessian,
	gc::Nilable<AbstractLargeSquareMatrix_sp>	hessian,
	gc::Nilable<NVector_sp>	hdvec,
        gc::Nilable<NVector_sp> dvec)
    { 
	if ( this->_DebugEnergy ) 
	{
	    LOG_ENERGY_CLEAR();
	    LOG_ENERGY(BF("%s {\n")% this->className());
	}

	ANN(force);
	ANN(hessian);
	ANN(hdvec);
	ANN(dvec);
	bool	hasForce = force.notnilp();
	bool	hasHessian = hessian.notnilp();
	bool	hasHdAndD = (hdvec.notnilp())&&(dvec.notnilp());

#define STRETCH_CALC_FORCE
#define STRETCH_CALC_DIAGONAL_HESSIAN
#define STRETCH_CALC_OFF_DIAGONAL_HESSIAN
#undef	STRETCH_SET_PARAMETER
#define	STRETCH_SET_PARAMETER(x)	{x = si->term.x;}
#undef	STRETCH_SET_POSITION
#define	STRETCH_SET_POSITION(x,ii,of)	{x = pos->getElement(ii+of);}
#undef	STRETCH_ENERGY_ACCUMULATE
#define	STRETCH_ENERGY_ACCUMULATE(e) this->_TotalEnergy += (e);
#undef	STRETCH_FORCE_ACCUMULATE
#undef	STRETCH_DIAGONAL_HESSIAN_ACCUMULATE
#undef	STRETCH_OFF_DIAGONAL_HESSIAN_ACCUMULATE
#define	STRETCH_FORCE_ACCUMULATE 		ForceAcc
#define	STRETCH_DIAGONAL_HESSIAN_ACCUMULATE 	DiagHessAcc
#define	STRETCH_OFF_DIAGONAL_HESSIAN_ACCUMULATE OffDiagHessAcc

	if ( this->isEnabled() ) 
	{ _BLOCK_TRACE("Stretch");
#pragma clang diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <cando/chem/energy_functions/_Stretch_termDeclares.cc>
#pragma clang diagnostic pop
	    fx1 = 0.0; fy1 = 0.0; fz1 = 0.0;
	    fx2 = 0.0; fy2 = 0.0; fz2 = 0.0;
	    double x1,y1,z1,x2,y2,z2,kb,r0;
	    int I1, I2,i;
            gctools::Vec0<EnergyStretch>::iterator si;
	    for ( i=0,si=this->_Terms.begin();
		  si!=this->_Terms.end(); si++,i++ ) {
#ifdef DEBUG_CONTROL_THE_NUMBER_OF_TERMS_EVALAUTED
		if ( this->_Debug_NumberOfTermsToCalculate > 0 ) {
		    if ( i>= this->_Debug_NumberOfTermsToCalculate ) {
			break;
		    }
		}
#endif

		// ************* Evaluate the stretch energy/force/hessian
		// using code generated by Mathematica ***************
#include <cando/chem/energy_functions/_Stretch_termCode.cc>
#if TURN_ENERGY_FUNCTION_DEBUG_ON //[
		si->_calcForce = calcForce;
		si->_calcDiagonalHessian = calcDiagonalHessian;
		si->_calcOffDiagonalHessian = calcOffDiagonalHessian;
#undef EVAL_SET
#define EVAL_SET(var,val) {si->eval.var=val;}
#include <cando/chem/energy_functions/_Stretch_debugEvalSet.cc>
#endif //]
		if ( this->_DebugEnergy ) {
		    LOG_ENERGY(BF( "MEISTER stretch %d args cando\n")% (i+1) );
		    LOG_ENERGY(BF( "MEISTER stretch %d address %x \n")% (i+1) % (&(*si)) );
		    LOG_ENERGY(BF( "MEISTER stretch %d r0 %5.3lf\n")% (i+1) % r0 );
		    LOG_ENERGY(BF( "MEISTER stretch %d kb %5.1lf\n")% (i+1) % kb );
		    LOG_ENERGY(BF( "MEISTER stretch %d x1 %5.3lf %d\n")% (i+1) % x1 % (I1/3+1) );
		    LOG_ENERGY(BF( "MEISTER stretch %d y1 %5.3lf %d\n")% (i+1) % y1 % (I1/3+1) );
		    LOG_ENERGY(BF( "MEISTER stretch %d z1 %5.3lf %d\n")% (i+1) % z1 % (I1/3+1) );
		    LOG_ENERGY(BF( "MEISTER stretch %d x2 %5.3lf %d\n")% (i+1) % x2 % (I2/3+1) );
		    LOG_ENERGY(BF( "MEISTER stretch %d y2 %5.3lf %d\n")% (i+1) % y2 % (I2/3+1) );
		    LOG_ENERGY(BF( "MEISTER stretch %d z2 %5.3lf %d\n")% (i+1) % z2 % (I2/3+1) );
		    LOG_ENERGY(BF( "MEISTER stretch %d results\n")% (i+1) );
		    LOG_ENERGY(BF( "MEISTER stretch %d Energy %lf\n")% (i+1) % Energy );
		    if ( calcForce ) {
			LOG_ENERGY(BF( "MEISTER stretch %d fx1 %lf %d\n")% (i+1) % fx1 % (I1/3+1) );
			LOG_ENERGY(BF( "MEISTER stretch %d fy1 %lf %d\n")% (i+1) % fy1 % (I1/3+1) );
			LOG_ENERGY(BF( "MEISTER stretch %d fz1 %lf %d\n")% (i+1) % fz1 % (I1/3+1) );
			LOG_ENERGY(BF( "MEISTER stretch %d fx2 %lf %d\n")% (i+1) % fx2 % (I2/3+1) );
			LOG_ENERGY(BF( "MEISTER stretch %d fy2 %lf %d\n")% (i+1) % fy2 % (I2/3+1) );
			LOG_ENERGY(BF( "MEISTER stretch %d fz2 %lf %d\n")% (i+1) % fz2 % (I2/3+1) );
		    }
		    LOG_ENERGY(BF( "MEISTER stretch %d stop\n")% (i+1) );
		}
		/* Add the forces */

//		_lisp->profiler().eventCounter(core::forcesGreaterThan10000).recordCallAndProblem(fx1>10000.0);
//		_lisp->profiler().eventCounter(core::forcesGreaterThan10000).recordCallAndProblem(fy1>10000.0);
//		_lisp->profiler().eventCounter(core::forcesGreaterThan10000).recordCallAndProblem(fz1>10000.0);
//		_lisp->profiler().eventCounter(core::forcesGreaterThan10000).recordCallAndProblem(fx2>10000.0);
//		_lisp->profiler().eventCounter(core::forcesGreaterThan10000).recordCallAndProblem(fy2>10000.0);
//		_lisp->profiler().eventCounter(core::forcesGreaterThan10000).recordCallAndProblem(fz2>10000.0);
	    }
	}
	if ( this->_DebugEnergy ) 
	{
	    LOG_ENERGY(BF("%s }")% this->className());
	}
    }







    void	EnergyStretch_O::compareAnalyticalAndNumericalForceAndHessianTermByTerm( NVector_sp 	pos)
    {_OF();
	int	fails = 0;
	bool	calcForce = true;
	bool	calcDiagonalHessian = true;
	bool	calcOffDiagonalHessian = true;
	{

#define STRETCH_CALC_FORCE
#define STRETCH_CALC_DIAGONAL_HESSIAN
#define STRETCH_CALC_OFF_DIAGONAL_HESSIAN
#undef	STRETCH_SET_PARAMETER
#define	STRETCH_SET_PARAMETER(x)	{x = si->term.x;}
#undef	STRETCH_SET_POSITION
#define	STRETCH_SET_POSITION(x,ii,of)	{x = pos->getElement(ii+of);}
#undef	STRETCH_ENERGY_ACCUMULATE
#define	STRETCH_ENERGY_ACCUMULATE(e) {}
#undef	STRETCH_FORCE_ACCUMULATE
#define	STRETCH_FORCE_ACCUMULATE(i,o,v) {}
#undef	STRETCH_DIAGONAL_HESSIAN_ACCUMULATE
#define	STRETCH_DIAGONAL_HESSIAN_ACCUMULATE(i1,o1,i2,o2,v) {}
#undef	STRETCH_OFF_DIAGONAL_HESSIAN_ACCUMULATE
#define	STRETCH_OFF_DIAGONAL_HESSIAN_ACCUMULATE(i1,o1,i2,o2,v) {}

	    if ( this->isEnabled() ) {
		_BLOCK_TRACE("StretchEnergy finiteDifference comparison");
#pragma clang diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <cando/chem/energy_functions/_Stretch_termDeclares.cc>
#pragma clang diagnostic pop
		fx1 = 0.0; fy1 = 0.0; fz1 = 0.0;
		fx2 = 0.0; fy2 = 0.0; fz2 = 0.0;
		double x1,y1,z1,x2,y2,z2,kb,r0;
		int I1, I2,i;
//	    stretchScale = this->getScale();
                gctools::Vec0<EnergyStretch>::iterator si;
		for ( i=0,si=this->_Terms.begin();
		      si!=this->_Terms.end(); si++,i++ ) {
		    // ************* Evaluate the stretch energy/force/hessian
		    // using code generated by Mathematica ***************
#include <cando/chem/energy_functions/_Stretch_termCode.cc>
		    LOG(BF("Energy = %le") % Energy  );
		    int index = i;
		    LOG(BF("x1 = %le") % x1  );
		    LOG(BF("y1 = %le") % y1  );
		    LOG(BF("z1 = %le") % z1  );
		    LOG(BF("x2 = %le") % x2  );
		    LOG(BF("y2 = %le") % y2  );
		    LOG(BF("z2 = %le") % z2  );
#include <cando/chem/energy_functions/_Stretch_debugFiniteDifference.cc>
		}
	    }
	}
    }



    int	EnergyStretch_O::checkForBeyondThresholdInteractions(
	stringstream& info, NVector_sp pos )
    {_OF();
	int	fails = 0;
//	bool	calcForce = false;
//	bool	calcDiagonalHessian = false;
//	bool	calcOffDiagonalHessian = false;

	this->_BeyondThresholdTerms.clear();

#undef STRETCH_CALC_FORCE
#undef STRETCH_CALC_DIAGONAL_HESSIAN
#undef STRETCH_CALC_OFF_DIAGONAL_HESSIAN
#undef	STRETCH_SET_PARAMETER
#define	STRETCH_SET_PARAMETER(x)	{x = si->term.x;}
#undef	STRETCH_SET_POSITION
#define	STRETCH_SET_POSITION(x,ii,of)	{x = pos->getElement(ii+of);}
#undef	STRETCH_ENERGY_ACCUMULATE
#define	STRETCH_ENERGY_ACCUMULATE(e) {}
#undef	STRETCH_FORCE_ACCUMULATE
#define	STRETCH_FORCE_ACCUMULATE(i,o,v) {}
#undef	STRETCH_DIAGONAL_HESSIAN_ACCUMULATE
#define	STRETCH_DIAGONAL_HESSIAN_ACCUMULATE(i1,o1,i2,o2,v) {}
#undef	STRETCH_OFF_DIAGONAL_HESSIAN_ACCUMULATE
#define	STRETCH_OFF_DIAGONAL_HESSIAN_ACCUMULATE(i1,o1,i2,o2,v) {}

	if ( this->isEnabled() ) {
	    _BLOCK_TRACE("StretchEnergy finiteDifference comparison");
#pragma clang diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <cando/chem/energy_functions/_Stretch_termDeclares.cc>
#pragma clang diagnostic pop
	    fx1 = 0.0; fy1 = 0.0; fz1 = 0.0;
	    fx2 = 0.0; fy2 = 0.0; fz2 = 0.0;
	    double x1,y1,z1,x2,y2,z2,kb,r0;
	    int I1, I2,i;
//	    stretchScale = this->getScale();
            gctools::Vec0<EnergyStretch>::iterator si;
	    for ( i=0,si=this->_Terms.begin();
		  si!=this->_Terms.end(); si++,i++ ) {
		// ************* Evaluate the stretch energy/force/hessian
		// using code generated by Mathematica ***************
#include <cando/chem/energy_functions/_Stretch_termCode.cc>
		ASSERT(r0 != 0.0);
		if ( fabs(StretchDeviation)/r0 > this->_ErrorThreshold ) {
		    Atom_sp a1, a2;
		    a1 = (*si)._Atom1;
		    a2 = (*si)._Atom2;
		    info<< "StretchDeviation ";
		    info<< "value " << fabs(StretchDeviation)/r0 << " ";
		    info<<"threshold " << this->_ErrorThreshold << " Atoms(";
		    info << a1->getName() << " ";
		    info << a2->getName() << ")";
		    info << std::endl;
		    this->_BeyondThresholdTerms.push_back(*si);
		    fails++;
		}
	    }
	}
	return fails;
    }




    void	EnergyStretch_O::dumpTerms()
    {_OF();
        gctools::Vec0<EnergyStretch>::iterator	esi;
	string				as1,as2,as3,as4;
	string				str1, str2, str3, str4;
	LOG(BF("Dumping EnergyStretch terms"));
	uint idx;
	for ( idx = 0, esi=this->_Terms.begin(); esi!=this->_Terms.end(); esi++, idx++ )
	{
	    as1 = atomLabel(esi->_Atom1);
	    as2 = atomLabel(esi->_Atom2);
	    if ( as1 < as2 ) {
		str1 = as1;
		str2 = as2;
	    } else {
		str2 = as1;
		str1 = as2;
	    }
	    _lisp->print(BF("TERM 1BND %-9s - %-9s %8.2lf %8.2lf") 
			 % str1 
			 % str2
			 % esi->term.kb
			 % esi->term.r0 );
	    LOG(BF("TERM[#%d] 1BND %-9s - %-9s %8.2lf %8.2lf ; I1=%d I2=%d")
		% idx
		% str1 
		% str2
		% esi->term.kb
		% esi->term.r0
		% esi->term.I1
		% esi->term.I2 );
	}
    }










    void EnergyStretch_O::initialize()
    {
	this->Base::initialize();
	this->setErrorThreshold(0.05);
    }

#ifdef XML_ARCHIVE
    void EnergyStretch_O::archiveBase(core::ArchiveP node)
    {
	this->Base::archiveBase(node);
	archiveEnergyComponentTerms<EnergyStretch_O,EnergyStretch>(node,*this);
    }
#endif


    void EnergyStretch_O::addTerm(const EnergyStretch& term)
    {
	this->_Terms.push_back(term);
    }

    string EnergyStretch_O::beyondThresholdInteractionsAsString()
    {
	return component_beyondThresholdInteractionsAsString<EnergyStretch_O,EnergyStretch>(*this);
    }



};
