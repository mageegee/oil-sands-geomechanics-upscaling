
#include "modelanisossoft.h"
//#include "base/src/version.h"
#include <cmath>       // for std::tan, std::sqrt
#include <stdexcept> // for std::runtime_error
#include "version.txt"

// excerpt-export-start
int __stdcall DllMain(void*, unsigned, void*) {
    return 1;
}

extern "C" EXPORT_TAG const char* getName() {
#ifdef MODELDEBUG
    return "cmodelanisotropic-strain-softening";
#else
    return "cmodelanisotropicstrainsoftening";
#endif
}

extern "C" EXPORT_TAG unsigned getMajorVersion() {
    return MAJOR_VERSION;
}

extern "C" EXPORT_TAG unsigned getMinorVersion() {
    return UPDATE_VERSION;
}

extern "C" EXPORT_TAG void* createInstance() {
    models::ModelStrainSofteningAnisotropic* m = new models::ModelStrainSofteningAnisotropic();
    return (void*)m;
}
// excerpt-export-end


namespace models {

    ModelStrainSofteningAnisotropic::ModelStrainSofteningAnisotropic()
        : ModelMohr() {

        // Additional initialization if necessary.
    }

    String ModelStrainSofteningAnisotropic::getProperties(void) const {
        return ModelMohr::getProperties() +
            L",table-cohesion-x,table-cohesion-y,table-cohesion-z,"
            L"table-friction-x,table-friction-y,table-friction-z,"
            L"table-dilation-x,table-dilation-y,table-dilation-z,"
            L"table-tension-x,table-tension-y,table-tension-z,"
            L"strain-shear-plastic-x,strain-shear-plastic-y,strain-shear-plastic-z,"
            L"strain-tension-plastic-x,strain-tension-plastic-y,strain-tension-plastic-z";
    }


    UInt ModelStrainSofteningAnisotropic::getMinorVersion() const {
        return UPDATE_VERSION;
    }

    Variant ModelStrainSofteningAnisotropic::getProperty(UInt index) const {
        if (index <= 27) {
            return ModelMohr::getProperty(index);
        }
        switch (index) {
            // Table-cohesion (X, Y, Z)
        case 28: return cTableX_;
        case 29: return cTableY_;
        case 30: return cTableZ_;
            // Table-friction (X, Y, Z)
        case 31: return fTableX_;
        case 32: return fTableY_;
        case 33: return fTableZ_;
            // Table-dilation (X, Y, Z)
        case 34: return dTableX_;
        case 35: return dTableY_;
        case 36: return dTableZ_;
            // Table-tension (X, Y, Z)
        case 37: return tTableX_;
        case 38: return tTableY_;
        case 39: return tTableZ_;
            // Strain-shear-plastic (X, Y, Z)
        case 40: return sHPX_;
        case 41: return sHPY_;
        case 42: return sHPZ_;
            // Strain-tension-plastic (X, Y, Z)
        case 43: return tHPX_;
        case 44: return tHPY_;
        case 45: return tHPZ_;
        default:
            throw std::runtime_error("Invalid property index in ModelStrainSofteningAnisotropic.");
        }
        return 0.0;
    }
    void ModelStrainSofteningAnisotropic::setProperty(UInt index, const Variant& p, UInt restoreVersion) {
        if (index <= 27) {
            ModelMohr::setProperty(index, p, restoreVersion);
            return;
        }
        switch (index) {
            // Table-cohesion (X, Y, Z)
        case 28: cTableX_ = p.toString(); break;
        case 29: cTableY_ = p.toString(); break;
        case 30: cTableZ_ = p.toString(); break;
            // Table-friction (X, Y, Z)
        case 31: fTableX_ = p.toString(); break;
        case 32: fTableY_ = p.toString(); break;
        case 33: fTableZ_ = p.toString(); break;
            // Table-dilation (X, Y, Z)
        case 34: dTableX_ = p.toString(); break;
        case 35: dTableY_ = p.toString(); break;
        case 36: dTableZ_ = p.toString(); break;
            // Table-tension (X, Y, Z)
        case 37: tTableX_ = p.toString(); break;
        case 38: tTableY_ = p.toString(); break;
        case 39: tTableZ_ = p.toString(); break;
            // Strain-shear-plastic (X, Y, Z)
        case 40: sHPX_ = p.toDouble(); break;
        case 41: sHPY_ = p.toDouble(); break;
        case 42: sHPZ_ = p.toDouble(); break;
            // Strain-tension-plastic (X, Y, Z)
        case 43: tHPX_ = p.toDouble(); break;
        case 44: tHPY_ = p.toDouble(); break;
        case 45: tHPZ_ = p.toDouble(); break;
        default:
            throw std::runtime_error("Invalid property index for input parameters in ModelStrainSofteningAnisotropic.");
        }
    }

    bool ModelStrainSofteningAnisotropic::isPropertyReadOnly(UInt i) const {
        if (i <= 27)
            return ModelMohr::isPropertyReadOnly(i);
        else if (i >= 40 && i <= 45)
            return true;  // Strain plastic properties are read-only.
        return false;
    }

    bool ModelStrainSofteningAnisotropic::isPropertyAdvanced(UInt i) const {
        if (i <= 27)
            return ModelMohr::isPropertyAdvanced(i);
        else if (i >= 28 && i <= 39)
            return true;  // Table identifiers (cohesion, friction, dilation, tension) are advanced.
        return false;
    }


    void ModelStrainSofteningAnisotropic::copy(const ConstitutiveModel* m) {
        const ModelStrainSofteningAnisotropic* mm = dynamic_cast<const ModelStrainSofteningAnisotropic*>(m);
        if (!mm)
            throw std::runtime_error("Internal error: constitutive model dynamic cast failed.");
        ModelMohr::copy(m);

        // Copy cohesion table command strings for each direction.
        cTableX_ = mm->cTableX_;
        cTableY_ = mm->cTableY_;
        cTableZ_ = mm->cTableZ_;

        // Copy friction table command strings for each direction.
        fTableX_ = mm->fTableX_;
        fTableY_ = mm->fTableY_;
        fTableZ_ = mm->fTableZ_;

        // Copy dilation table command strings for each direction.
        dTableX_ = mm->dTableX_;
        dTableY_ = mm->dTableY_;
        dTableZ_ = mm->dTableZ_;

        // Copy tension table command strings for each direction.
        tTableX_ = mm->tTableX_;
        tTableY_ = mm->tTableY_;
        tTableZ_ = mm->tTableZ_;

        // Copy strain-shear-plastic properties for each direction.
        sHPX_ = mm->sHPX_;
        sHPY_ = mm->sHPY_;
        sHPZ_ = mm->sHPZ_;

        // Copy strain-tension-plastic properties for each direction.
        tHPX_ = mm->tHPX_;
        tHPY_ = mm->tHPY_;
        tHPZ_ = mm->tHPZ_;
    }


    void ModelStrainSofteningAnisotropic::initialize(UByte d, State* s) {
        // Standard initialization.
        ConstitutiveModel::initialize(d, s);

        // Initialize table index pointers for cohesion, friction, dilation, and tension.
        iCohesionX_ = iCohesionY_ = iCohesionZ_ = nullptr;
        iFrictionX_ = iFrictionY_ = iFrictionZ_ = nullptr;
        iDilationX_ = iDilationY_ = iDilationZ_ = nullptr;
        iTensionX_ = iTensionY_ = iTensionZ_ = nullptr;

        // Retrieve table indices for cohesion.
        if (cTableX_.length()) iCohesionX_ = s->getTableIndexFromID(cTableX_);
        if (cTableY_.length()) iCohesionY_ = s->getTableIndexFromID(cTableY_);
        if (cTableZ_.length()) iCohesionZ_ = s->getTableIndexFromID(cTableZ_);

        // Retrieve table indices for friction.
        if (fTableX_.length()) iFrictionX_ = s->getTableIndexFromID(fTableX_);
        if (fTableY_.length()) iFrictionY_ = s->getTableIndexFromID(fTableY_);
        if (fTableZ_.length()) iFrictionZ_ = s->getTableIndexFromID(fTableZ_);

        // Retrieve table indices for dilation.
        if (dTableX_.length()) iDilationX_ = s->getTableIndexFromID(dTableX_);
        if (dTableY_.length()) iDilationY_ = s->getTableIndexFromID(dTableY_);
        if (dTableZ_.length()) iDilationZ_ = s->getTableIndexFromID(dTableZ_);

        // Retrieve table indices for tension.
        if (tTableX_.length()) iTensionX_ = s->getTableIndexFromID(tTableX_);
        if (tTableY_.length()) iTensionY_ = s->getTableIndexFromID(tTableY_);
        if (tTableZ_.length()) iTensionZ_ = s->getTableIndexFromID(tTableZ_);

        // Check for brittle flag on any tension property.
        if ((iTensionX_ && brittleX_) || (iTensionY_ && brittleY_) || (iTensionZ_ && brittleZ_))
            throw std::runtime_error("Internal error: flag-brittle not allowed when assigning table-tension.");

        // Initialize cohesion values from the respective tables.
        if (iCohesionX_) cohesionX_ = s->getYFromX(iCohesionX_, sHPX_);
        if (iCohesionY_) cohesionY_ = s->getYFromX(iCohesionY_, sHPY_);
        if (iCohesionZ_) cohesionZ_ = s->getYFromX(iCohesionZ_, sHPZ_);

        // Initialize friction values from the respective tables.
        if (iFrictionX_) frictionX_ = s->getYFromX(iFrictionX_, sHPX_);
        if (iFrictionY_) frictionY_ = s->getYFromX(iFrictionY_, sHPY_);
        if (iFrictionZ_) frictionZ_ = s->getYFromX(iFrictionZ_, sHPZ_);

        // Initialize dilation values from the respective tables.
        if (iDilationX_) dilationX_ = s->getYFromX(iDilationX_, sHPX_);
        if (iDilationY_) dilationY_ = s->getYFromX(iDilationY_, sHPY_);
        if (iDilationZ_) dilationZ_ = s->getYFromX(iDilationZ_, sHPZ_);

        // Initialize tension values from the respective tables.
        if (iTensionX_) tensionX_ = s->getYFromX(iTensionX_, tHPX_);
        if (iTensionY_) tensionY_ = s->getYFromX(iTensionY_, tHPY_);
        if (iTensionZ_) tensionZ_ = s->getYFromX(iTensionZ_, tHPZ_);

        // Adjust tension values based on friction (calculate apex).
        // For each direction: if friction > 0, compute apex = cohesion / tan(friction*degrad)
        // and set tension to the minimum of its current value and the apex.
        if (frictionX_ > 0.0) {
            double apexX = cohesionX_ / std::tan(frictionX_ * degrad);
            tensionX_ = tensionX_ < apexX ? tensionX_ : apexX;
        }
        else if (cohesionX_ == 0.0) {
            tensionX_ = 0.0;
        }
        if (frictionY_ > 0.0) {
            double apexY = cohesionY_ / std::tan(frictionY_ * degrad);
            tensionY_ = tensionY_ < apexY ? tensionY_ : apexY;
        }
        else if (cohesionY_ == 0.0) {
            tensionY_ = 0.0;
        }
        if (frictionZ_ > 0.0) {
            double apexZ = cohesionZ_ / std::tan(frictionZ_ * degrad);
            tensionZ_ = tensionZ_ < apexZ ? tensionZ_ : apexZ;
        }
        else if (cohesionZ_ == 0.0) {
            tensionZ_ = 0.0;
        }
    }


    static const UInt Dqs = 0;
    static const UInt Dqt = 1;

    void ModelStrainSofteningAnisotropic::run(UByte d, State* s) {
        // 1) Standard setup & parameter updates
        ConstitutiveModel::run(d, s);
        bool noUpdateE = false;
        if (s->modulus_reduction_factor_ > 0.0) {
            ModelAnisotropic::moduliReduction(s->modulus_reduction_factor_);
            noUpdateE = true;
        }

        ModelMohr::updateParameters(noUpdateE, &sf1_x_, &sf3_x_, &sf1_y_, &sf3_y_, &sf1_z_, &sf3_z_);

        // 2) Shift shear/tension flags
        if (s->state_ & shear_now)   s->state_ |= shear_past;
        s->state_ &= ~shear_now;
        if (s->state_ & tension_now) s->state_ |= tension_past;
        s->state_ &= ~tension_now;

        UInt iPlas = 0;
        if (!s->sub_zone_) {
            s->working_[Dqs] = 0.0;
            s->working_[Dqt] = 0.0;
        }
        Double dSubZoneVolume = s->getSubZoneVolume();

        // 3) Elastic trial
        ModelAnisotropic::elasticTrial(s);
        s->viscous_ = true;
        if (!canFail()) return;
        Double s11 = s->stnS_.s11();
        Double s22 = s->stnS_.s22();
        Double s33 = s->stnS_.s33();
        int globalOrder[3];

        if (s11 <= s22 && s11 <= s33) {
            // X is min (most compressed)
            globalOrder[0] = 0; // X
            if (s22 <= s33) {
                globalOrder[1] = 1; // Y
                globalOrder[2] = 2; // Z
            }
            else {
                globalOrder[1] = 2; // Z
                globalOrder[2] = 1; // Y
            }
        }
        else if (s22 <= s11 && s22 <= s33) {
            // Y is min
            globalOrder[0] = 1; // Y
            if (s11 <= s33) {
                globalOrder[1] = 0; // X
                globalOrder[2] = 2; // Z
            }
            else {
                globalOrder[1] = 2; // Z
                globalOrder[2] = 0; // X
            }
        }
        else {
            // Z is min
            globalOrder[0] = 2; // Z
            if (s11 <= s22) {
                globalOrder[1] = 0; // X
                globalOrder[2] = 1; // Y
            }
            else {
                globalOrder[1] = 1; // Y
                globalOrder[2] = 0; // X
            }
        }
        // 4) Get principal stresses
        SymTensorInfo info;
        DVect3 prin = s->stnS_.getEigenInfo(&info);
        Double p[3] = { prin.x(), prin.y(), prin.z() };

        // 5) Find index of the max principal
        UInt dirMax = 0;
        if (p[1] > p[dirMax]) dirMax = 1;
        if (p[2] > p[dirMax]) dirMax = 2;
        UInt i1 = (dirMax + 1) % 3;
        UInt i2 = (dirMax + 2) % 3;
       
        Double friction[3] = { frictionX_, frictionY_, frictionZ_ };
        Double cohesion[3] = { cohesionX_, cohesionY_, cohesionZ_ };
        Double nph[3] = { nph_x_, nph_y_, nph_z_ };
        Double csn[3] = { csn_x_, csn_y_, csn_z_ };
        Double rc[3] = { rc_x_, rc_y_, rc_z_ };
        Double tensionArr[3] = { tensionX_, tensionY_, tensionZ_ };
        bool brittleArr[3] = { brittleX_, brittleY_, brittleZ_ };
        Double e1[3] = { e1_x_, e1_y_, e1_z_ };
        Double e2[3] = { e2_x_, e2_y_, e2_z_ };
        Double sc1[3] = { sc1_x_, sc1_y_, sc1_z_ };
        Double sc2[3] = { sc2_x_, sc2_y_, sc2_z_ };
        Double sc3[3] = { sc3_x_, sc3_y_, sc3_z_ };

        int matIdx = globalOrder[0];  // the material axis for the max principal
        Double fs = nph[matIdx] * p[dirMax] - p[i1] - csn[matIdx];
        Double fsd = fs / rc[matIdx];

        Double ft = p[dirMax] - tensionArr[matIdx];
        Double f1 = p[i1] - tensionArr[matIdx];
        Double f2 = p[i2] - tensionArr[matIdx];

        if (fsd > 0.0 && fsd >= ft) 
            ModelMohr::shearCorrection(s, &prin, &iPlas, fs, matIdx, sc1, sc2, sc3);
        
        else if (ft > 0.0) 
            ModelMohr::tensionCorrection(s, &prin, &iPlas, ft, dirMax, globalOrder, tensionArr, e1, e2, brittleArr);
            ModelMohr::apexCorrection(s, &prin, &iPlas,  globalOrder, friction, cohesion, tensionArr, brittleArr);

        // 9) If plasticity happened, resolve & compute work
        if (iPlas) {
            s->viscous_ = false;
            s->stnS_ = info.resolve(prin);
            Double sf1[3] = { sf1_x_ ,sf1_y_ ,sf1_z_ };
            Double sf3[3] = { sf3_x_ ,sf3_y_ ,sf3_z_ };
            
            if (iPlas == 1) {
                // shear‐dominated
                Double dDe1p = fs * sf1[matIdx];
                Double dDe3p = fs * sf3[matIdx];
                Double dDepa = (dDe1p + dDe3p) / 3.0;
                dDe1p -= dDepa;  dDe3p -= dDepa;
                s->working_[Dqs] +=
                    std::sqrt(0.5 * (dDe1p * dDe1p + dDepa * dDepa + dDe3p * dDe3p))
                    * dSubZoneVolume;
            }
            else if (iPlas == 2) {
                // pure tensile in max-dir
                Double dAux = ft / e1[matIdx];
                s->working_[Dqt] += dAux * dSubZoneVolume;
            }
            else if (iPlas == 3) {
                // mixed tension on one secondary dir
                Double dAux = (ft + f1) / (e1[matIdx] + e2[matIdx]);
                s->working_[Dqt] += dAux * dSubZoneVolume;
            }
            else if (iPlas == 4) {
                // mixed tension on both secondaries
                Double dAux = (ft + f1 + f2) / (e1[matIdx] + 2.0 * e2[matIdx]);
                s->working_[Dqt] += dAux * dSubZoneVolume;
            }
        }

        if (s->sub_zone_ == s->total_sub_zones_ - 1) {
            // Compute the auxiliary factor from the zone volume.
            Double dAux = 1.0 / s->getZoneVolume();
            if (s->overlay_ == 2)
                dAux *= 0.5;
            //sHPZ_ += s->working_[Dqs] * dAux;
            //sHPX_ += s->working_[Dqs] * dAux;
            //sHPY_ += s->working_[Dqs] * dAux;
            // 2) Shear‐hardening only in yielded direction
            if (s->working_[Dqs] > 0.0) {
                switch (matIdx) {
                case 0:  sHPX_ += s->working_[Dqs] * dAux; break;
                case 1:  sHPY_ += s->working_[Dqs] * dAux; break;
                case 2:  sHPZ_ += s->working_[Dqs] * dAux; break;
                }
                // 3) Shear softening table look‑ups for that same direction
                if (matIdx == 0) {
                    if (iCohesionX_) cohesionX_ = s->getYFromX(iCohesionX_, sHPX_);
                    if (iFrictionX_) frictionX_ = s->getYFromX(iFrictionX_, sHPX_);
                    if (iDilationX_) dilationX_ = s->getYFromX(iDilationX_, sHPX_);
                }
                else if (matIdx == 1) {
                    if (iCohesionY_) cohesionY_ = s->getYFromX(iCohesionY_, sHPY_);
                    if (iFrictionY_) frictionY_ = s->getYFromX(iFrictionY_, sHPY_);
                    if (iDilationY_) dilationY_ = s->getYFromX(iDilationY_, sHPY_);
                }
                else if (matIdx == 2) {
                    if (iCohesionZ_) cohesionZ_ = s->getYFromX(iCohesionZ_, sHPZ_);
                    if (iFrictionZ_) frictionZ_ = s->getYFromX(iFrictionZ_, sHPZ_);
                    if (iDilationZ_) dilationZ_ = s->getYFromX(iDilationZ_, sHPZ_);
                }
            }
            //switch (matIdx) {
            //case 0:  sHPX_ += s->working_[Dqs] * dAux;  break;
            //case 1:  sHPY_ += s->working_[Dqs] * dAux;  break;
            //case 2:  sHPZ_ += s->working_[Dqs] * dAux;  break;
            //}
            // 
            //sHPX_ += s->working_[Dqs] * dAux;
            //sHPY_ += s->working_[Dqs] * dAux;
            

            //// Update tension-hardening for each direction.
            tHPX_ += s->working_[Dqt] * dAux;
            tHPY_ += s->working_[Dqt] * dAux;
            tHPZ_ += s->working_[Dqt] * dAux;

            ////// If shear work has been done, update cohesion, friction, and dilation from their tables.
            //if (s->working_[Dqs] > 0.0) {
            //        if (iCohesionX_) cohesionX_ = s->getYFromX(iCohesionX_, sHPX_);
            //        if (iFrictionX_) frictionX_ = s->getYFromX(iFrictionX_, sHPX_);
            //        if (iDilationX_) dilationX_ = s->getYFromX(iDilationX_, sHPX_);
            //        if (iCohesionY_) cohesionY_ = s->getYFromX(iCohesionY_, sHPY_);
            //        if (iFrictionY_) frictionY_ = s->getYFromX(iFrictionY_, sHPY_);
            //        if (iDilationY_) dilationY_ = s->getYFromX(iDilationY_, sHPY_);
            //        if (iCohesionZ_) cohesionZ_ = s->getYFromX(iCohesionZ_, sHPZ_);
            //        if (iFrictionZ_) frictionZ_ = s->getYFromX(iFrictionZ_, sHPZ_);
      /*              if (iDilationZ_) dilationZ_ = s->getYFromX(iDilationZ_, sHPZ_);
            }*/

            // 4) Tension‐hardening only in yielded direction
            if (s->working_[Dqt] > 0.0) {
                if (iTensionX_) tensionX_ = s->getYFromX(iTensionX_, tHPX_);
                if (iTensionY_) tensionY_ = s->getYFromX(iTensionY_, tHPY_);
                if ( iTensionZ_) tensionZ_ = s->getYFromX(iTensionZ_, tHPZ_);
            }
        
        }
    }

}
