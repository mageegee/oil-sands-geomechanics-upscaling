#pragma once

#include "modelaniso.h"  // Ensure this path is correct
#include <array>
#include <vector>

namespace models {

    class ModelMohr : public ModelAnisotropic {
    public:
        explicit ModelMohr(unsigned short option = 0);
        virtual ~ModelMohr() {}

        // ---------------------------------------------------------------------
        // Basic Model Info
        virtual String getName() const { return L"Anisotropic Mohr-Coulomb"; }
        virtual String getFullName() const { return L"Anisotropic Mohr-Coulomb"; }
        virtual String getPluginName() const { return getName(); }
        virtual UInt getMinorVersion() const override;
        virtual String getProperties() const override;
        virtual String getStates() const override { return L""; } // Customize as needed
        virtual bool isPropertyAdvanced(UInt i) const override;

        // ---------------------------------------------------------------------
        // Property Interface
        virtual Variant getProperty(UInt index) const override;
        virtual void setProperty(UInt index, const Variant& p, UInt restoreVersion = 0) override;
        virtual ModelMohr* clone() const override { return new ModelMohr(*this); }
        virtual void copy(const ConstitutiveModel* mod) override;

        // ---------------------------------------------------------------------
        // Model Execution
        virtual void run(UByte d, State* s) override;
        virtual void initialize(UByte d, State* s) override;
        

        // ---------------------------------------------------------------------
        // Mohr–Coulomb Special Methods
        virtual bool supportsStressStrengthRatio() const override { return true; }
        virtual bool supportsPropertyScaling()      const override { return true; }
        virtual Double getStressStrengthRatio(const SymTensor &st) const override;

    protected:
        // Correction Functions:
        // apexCorrection limits the principal stresses based on an "apex" value computed
        // from the anisotropic cohesion and friction parameters. (No single friction parameter is used.)
        virtual void apexCorrection(State* s, DVect3* prin, UInt* iPlasticity, int globalOrder[3],
            Double friction[3], Double cohesion[3], Double tensionArr[3], bool brittleArr[3]);

        // tensionCorrection applies tension-related corrections using the anisotropic tension
        // and brittle values. The parameter ftz is the tension failure function value.
        virtual void tensionCorrection(State* s, DVect3* prin, UInt* iPlasticity, const Double& ft, UInt dirMax,
            int principalToGlobal[3], Double tension[3], Double e1[3], Double e2[3], bool brittle[3]);

        // shearCorrection applies shear corrections based on a failure function value (fs).
        virtual void shearCorrection(State* s, DVect3* prin, UInt* iPlasticity,
            const Double& fs, int matIdx, Double sc1[3], Double sc2[3], Double sc3[3]);

        // Scaling of properties, if needed:
        virtual void scaleProperties(const Double &scale, const std::vector<UInt> &props) override;

        // ---------------------------------------------------------------------
        // Parameter Updates
        virtual bool updateParameters();  // Computes nph_, csn_, etc.
        virtual bool updateParameters(bool bEUpdated, Double* sf1_x_, Double* sf3_x_, Double* sf1_y_, Double* sf3_y_, Double* sf1_z_, Double* sf3_z_);

        // ---------------------------------------------------------------------
        // Shear Correction & Modulus Reduction
        // moduliReduction computes the reduced shear moduli for each direction.
        virtual std::array<Double, 3> moduliReduction(const Double &factor);

        // ---------------------------------------------------------------------
        // Mohr–Coulomb data for each principal direction
        Double cohesionX_ = 0;
        Double cohesionY_ = 0;
        Double cohesionZ_ = 0;

        Double frictionX_ = 0;
        Double frictionY_ = 0;
        Double frictionZ_ = 0;

        Double dilationX_ = 0;
        Double dilationY_ = 0; 
        Double dilationZ_ = 0;

        Double tensionX_ = 0;
        Double tensionY_ = 0;
        Double tensionZ_ = 0;

        bool brittleX_ = false;
        bool brittleY_ = false;
        bool brittleZ_ = false;

        Double csn_x_ = 0;
        Double csn_y_ = 0;
        Double csn_z_ = 0;
       
        Double nps_x_ = 0;
        Double nps_y_ = 0;
        Double nps_z_ = 0;
       
        Double nph_x_ = 0;
        Double nph_y_ = 0;
        Double nph_z_ = 0;
       
        Double rc_x_ = 0;
        Double rc_y_ = 0;
        Double rc_z_ = 0;
       
        Double sc1_x_ = 0;
        Double sc1_y_ = 0;
        Double sc1_z_ = 0;

        Double sc2_x_ = 0;
        Double sc2_y_ = 0;
        Double sc2_z_ = 0;
        
        Double sc3_x_ = 0;
        Double sc3_y_ = 0;
        Double sc3_z_ = 0;

        //Double sf1_x_ = 0;
        //Double sf1_y_ = 0;
        //Double sf1_z_ = 0;

        //Double sf3_x_ = 0;
        //Double sf3_y_ = 0;
        //Double sf3_z_ = 0;

        //std::array<Double, 3> sc1_ = { 0.0, 0.0, 0.0 };
        //std::array<Double, 3> sc2_ = { 0.0, 0.0, 0.0 };
        //std::array<Double, 3> sc3_ = { 0.0, 0.0, 0.0 };

        Double newG_x_ = 0;
        Double newG_y_ = 0;
        Double newG_z_ = 0;
    };

} // namespace models
