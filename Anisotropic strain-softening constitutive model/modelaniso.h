#pragma once

#include "models/src/conmodel.h"
//#include "models/base/src/modelweakplanetype.h"
#include <array>

namespace models {

    class ModelAnisotropic : public ConstitutiveModel {
    public:
        explicit ModelAnisotropic(unsigned short option = 0);
        virtual ~ModelAnisotropic() {}

        // Model Information
        virtual String getName() const { return L"Anisotropic Elastic"; }
        virtual String getFullName() const { return L"Anisotropic Elastic"; }
        virtual String getPluginName() const { return getName(); }
        virtual UInt getMinorVersion() const;
        virtual String getProperties() const;
        virtual String getStates() const;
        virtual Variant getProperty(UInt index) const;
        virtual void setProperty(UInt index, const Variant& p, UInt restoreVersion = 0);
        virtual ModelAnisotropic* clone() const { return new ModelAnisotropic(*this); }

        // Elasticity Properties
        virtual Double getConfinedModulus() const;
        virtual Double getShearModulus() const;
        virtual Double getBulkModulus() const;
        virtual void copy(const ConstitutiveModel* mod);

        // Core Model Operations
        virtual void run(UByte dim, State* s);
        virtual void initialize(UByte dim, State* s);
        virtual bool isModelAdvanced() const { return true; }

    protected:
        // Stiffness Matrix Operations
        //virtual void buildStiffnessMatrix();
        virtual void elasticTrial(State* s);
        //virtual void buildStiffnessMatrix();
       // static bool xmatinv(Double b[6][6]);
        //static void xmatmul(Double b1[6][6], Double b2[6][6], Double b3[6][6]);
        //virtual void buildMatrix(const Double& E_x, const Double& E_y, const Double& E_z,
           // const Double& nu_xy, const Double& nu_yz, const Double& nu_zx,
            //const Double& G_xy, const Double& G_yz, const Double& G_zx);



        virtual bool updateParameters();
        virtual std::array<Double, 3> moduliReduction(const Double &factor);

        // Helper function for updating directional moduli for anisotropic behavior.
        // This function recalculates K_x_, K_y_, K_z_, G_xy_, G_yz_, and G_zx_
        // based on the anisotropic Young's moduli and Poisson's ratios.
        //void updateDirectionalModuli();

        // Anisotropic Material Properties
        Double E_x_, E_y_, E_z_;      // Young's moduli in x, y, z directions
        Double G_xy_, G_yz_, G_zx_;    // Shear moduli in xy, yz, zx planes
        Double nu_xy_, nu_yz_, nu_zx_; // Poisson's ratios for xy, yz, zx planes
        Double K_x_, K_y_, K_z_;      // Bulk moduli in x, y, z directions

        // Elasticity Parameters for Anisotropic Behavior
        Double e1_x_ = 0;
        Double e1_y_ = 0;
        Double e1_z_ = 0;
        Double e2_x_ = 0;
        Double e2_y_ = 0;
        Double e2_z_ = 0;
        Double g2_x_ = 0;
        Double g2_y_ = 0;
        Double g2_z_ = 0;
        
        // Stiffness Matrix for Stress-Strain Relationship
    protected:
        Double A11_, A12_, A13_, A14_, A15_, A16_, A22_, A23_, A24_, A25_, A26_;
        Double A33_, A34_, A35_, A36_, A44_, A45_, A46_, A55_, A56_, A66_;
        Double newG_x_ = 0;
        Double newG_y_ = 0;
        Double newG_z_ = 0;
        Double stiffnessMatrix_[6][6];
    };

} // namespace models
