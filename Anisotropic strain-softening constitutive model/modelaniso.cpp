#include "modelaniso.h"
//#include "base/src/version.h"
#include <stdexcept>
#include <cmath>
#include <array>
#include "version.txt"

namespace models {

    ModelAnisotropic::ModelAnisotropic(unsigned short option)
        : ConstitutiveModel(option),
          K_x_(0.0), K_y_(0.0), K_z_(0.0),
          G_xy_(0.0), G_yz_(0.0), G_zx_(0.0),
          E_x_(0.0), E_y_(0.0), E_z_(0.0),
          nu_xy_(0.0), nu_yz_(0.0), nu_zx_(0.0),
        stiffnessMatrix_{
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } },
        
          A11_(0.0),A12_(0.0),A13_(0.0),A14_(0.0),A15_(0.0),A16_(0.0),
                    A22_(0.0),A23_(0.0),A24_(0.0),A25_(0.0),A26_(0.0),
                            A33_(0.0),A34_(0.0),A35_(0.0),A36_(0.0),
                                        A44_(0.0),A45_(0.0),A46_(0.0),
                                                A55_(0.0),A56_(0.0),
                                                            A66_(0.0){


      
    
    }

    UInt ModelAnisotropic::getMinorVersion() const {
        return UPDATE_VERSION;
    }

    String ModelAnisotropic::getProperties() const {
        return L"Ex,Ey,Ez,nuxy,nuyz,nuzx,Kx,Ky,Kz,Gxy,Gyz,Gzx";
    }

    String ModelAnisotropic::getStates() const {
        return L"";
    }

    Double ModelAnisotropic::getBulkModulus() const {
        Double M = (E_x_ > E_z_ ? E_x_ : E_z_);
        Double N = (nu_yz_ > nu_zx_ ? nu_yz_ : nu_zx_);
        return M / (3.0 * (1.0 - 2.0 * N));
    }
    Double ModelAnisotropic::getShearModulus() const {
        Double Gmax = std::max({ G_xy_, G_yz_, G_zx_ });
        return Gmax;
    }

    Double ModelAnisotropic::getConfinedModulus() const {
        return getBulkModulus() + d4d3 * getShearModulus();
    }
    

    Variant ModelAnisotropic::getProperty(UInt index) const {
        switch (index) {
        case 1: return E_x_;
        case 2: return E_y_;
        case 3: return E_z_;
        case 4: return nu_xy_;
        case 5: return nu_yz_;
        case 6: return nu_zx_;
        case 7: return K_x_;
        case 8: return K_y_;
        case 9: return K_z_;
        case 10: return G_xy_;
        case 11: return G_yz_;
        case 12: return G_zx_;
        default: throw std::runtime_error("Invalid property index Anisotropic.");
        }
        return 0.0;
    }

    void ModelAnisotropic::setProperty(UInt index, const Variant& p, UInt restoreVersion) {
        // First, store the input value.
        ConstitutiveModel::setProperty(index, p, restoreVersion);
        switch (index) {
            // The independent parameters:
        case 1: E_x_ = p.toDouble(); break;
        case 2: E_y_ = p.toDouble(); break;
        case 3: E_z_ = p.toDouble(); break;
        case 4: nu_xy_ = p.toDouble(); break;
        case 5: nu_yz_ = p.toDouble(); break;
        case 6: nu_zx_ = p.toDouble(); break;
        case 7: K_x_ = p.toDouble(); break;
        case 8: K_y_ = p.toDouble(); break;
        case 9: K_z_ = p.toDouble(); break;
        case 10: G_xy_ = p.toDouble(); break;
        case 11: G_yz_ = p.toDouble(); break;
        case 12: G_zx_ = p.toDouble(); break;

        default:
            throw std::runtime_error("Invalid property index for input parameters Anisotropic.");
        }
    }


    void ModelAnisotropic::copy(const ConstitutiveModel* m) {
        ConstitutiveModel::copy(m);
        const ModelAnisotropic* am = dynamic_cast<const ModelAnisotropic*>(m);
        if (!am)
            throw std::runtime_error("Internal error: constitutive model dynamic cast failed.");

        // Copy anisotropic moduli and related properties.

        E_x_ = am->E_x_;
        E_y_ = am->E_y_;
        E_z_ = am->E_z_;
        nu_xy_ = am->nu_xy_;
        nu_yz_ = am->nu_yz_;
        nu_zx_ = am->nu_zx_;
        K_x_ = am->K_x_;
        K_y_ = am->K_y_;
        K_z_ = am->K_z_;
        G_xy_ = am->G_xy_;
        G_yz_ = am->G_yz_;
        G_zx_ = am->G_zx_;

    }

    void ModelAnisotropic::initialize(UByte d, State* s) {
        ConstitutiveModel::initialize(d, s);
        updateParameters();
        // Validate independent parameters.
        if (E_x_ <= 0.0 || E_y_ <= 0.0 || E_z_ <= 0.0)
            throw std::runtime_error("Anisotropic Model - Invalid Young's moduli.");
        if (G_xy_ <= 0.0 || G_yz_ <= 0.0 || G_zx_ <= 0.0)
            throw std::runtime_error("Anisotropic Model - Invalid shear moduli.");
        if (nu_xy_ >= 0.5 || nu_xy_ < -1.0 ||
            nu_yz_ >= 0.5 || nu_yz_ < -1.0 ||
            nu_zx_ >= 0.5 || nu_zx_ < -1.0)
            throw std::runtime_error("Anisotropic Model - Invalid Poisson ratios.");
        //buildMatrix(E_x_, E_y_, E_z_, nu_yz_, nu_zx_, nu_xy_, G_yz_, G_zx_, G_xy_);
        
    }

    void ModelAnisotropic::run(UByte d, State* s) {
        // Perform base class run operations.
        ConstitutiveModel::run(d, s);

        // If a modulus reduction factor is specified, update the moduli accordingly.
        if (s->modulus_reduction_factor_ > 0.0)
            moduliReduction(s->modulus_reduction_factor_);

        // Update the stress state based on the current strain.
        elasticTrial(s);

        // Indicate that viscous effects are present (or active) for this model.
        s->viscous_ = true;
    }


    void ModelAnisotropic::elasticTrial(State* s) {
        Double DS11 = s->stnE_.s11();
        Double DS22 = s->stnE_.s22();
        Double DS33 = s->stnE_.s33();
        Double DS12 = s->stnE_.s12();
        Double DS13 = s->stnE_.s13();
        Double DS23 = s->stnE_.s23();
        Double nu_yx = (E_y_ / E_x_) * nu_xy_;
        Double nu_zy = (E_z_ / E_y_) * nu_yz_;
        Double nu_xz = (E_x_ / E_z_) * nu_zx_;
        Double D = 1.0 - nu_xy_ * nu_yx - nu_yz_ * nu_zy - nu_zx_ * nu_xz - 2.0 * nu_xy_ * nu_yz_ * nu_zx_;
        // Compute the normal stiffness components (Cij) based on the above formulas:
        Double C11 = E_x_ * (1.0 - nu_yz_ * nu_zy) / D;
        Double C22 = E_y_ * (1.0 - nu_zx_ * nu_xz) / D;
        Double C33 = E_z_ * (1.0 - nu_xy_ * nu_yx) / D;
        Double C12 = E_x_ * (nu_xy_ + nu_zx_ * nu_zy) / D; // equals C21
        Double C13 = E_x_ * (nu_zx_ + nu_xy_ * nu_yz_) / D; // equals C31
        Double C23 = E_y_ * (nu_yz_ + nu_xy_ * nu_zx_) / D; // equals C32

        // Shear stiffness components (assumed to be used with engineering shear strains)
        Double C44 = G_xy_;
        Double C55 = G_zx_;
        Double C66 = G_yz_;

        // Update the stress components using the full anisotropic stiffness matrix.
        // Note: For shear, we use engineering shear strains (which are 2x the tensorial strains).
        s->stnS_.rs11() += C11 * DS11 + C12 * DS22 + C13 * DS33;
        s->stnS_.rs22() += C12 * DS11 + C22 * DS22 + C23 * DS33;
        s->stnS_.rs33() += C13 * DS11 + C23 * DS22 + C33 * DS33;

        s->stnS_.rs12() += C44 * (2.0 * DS12); // engineering shear strain: gamma_12 = 2 * e_12
        s->stnS_.rs13() += C55 * (2.0 * DS13);
        s->stnS_.rs23() += C66 * (2.0 * DS23);
    }
    



    bool ModelAnisotropic::updateParameters() {
        // Update moduli for each direction
        e1_x_ = K_x_ + G_xy_ * (4.0 / 3.0);
        e2_x_ = K_x_ - G_xy_ * (2.0 / 3.0);
        g2_x_ = 2.0 * G_xy_;

        e1_y_ = K_y_ + G_yz_ * (4.0 / 3.0);
        e2_y_ = K_y_ - G_yz_ * (2.0 / 3.0);
        g2_y_ = 2.0 * G_yz_;

        e1_z_ = K_z_ + G_zx_ * (4.0 / 3.0);
        e2_z_ = K_z_ - G_zx_ * (2.0 / 3.0);
        g2_z_ = 2.0 * G_zx_;

        return true;
    }

    std::array<Double, 3> ModelAnisotropic::moduliReduction(const Double &factor) {
        // Use the member variable 'newG' to store the reduced shear moduli.
         Double newG[3] = { newG_x_ ,newG_y_ ,newG_z_ };
        newG_x_ = G_xy_ * factor;
        newG_y_ = G_yz_ * factor;
        newG_z_ = G_zx_ * factor;

        // Update directional moduli accordingly.
        e1_x_ = K_x_ + newG_x_ * (4.0 / 3.0);
        e2_x_ = K_x_ - newG_x_ * (2.0 / 3.0);
        g2_x_ = 2.0 * newG_x_;

        e1_y_ = K_y_ + newG_y_ * (4.0 / 3.0);
        e2_y_ = K_y_ - newG_y_ * (2.0 / 3.0);
        g2_y_ = 2.0 * newG_y_;

        e1_z_ = K_z_ + newG_z_ * (4.0 / 3.0);
        e2_z_ = K_z_ - newG_z_ * (2.0 / 3.0);
        g2_z_ = 2.0 * newG_z_;

        return { newG_x_, newG_y_, newG_z_ }; // Return all three reduced shear moduli.
    }

} // namespace models
//EOF
