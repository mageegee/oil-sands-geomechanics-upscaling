#include "modelmohr.h"
//#include "base/src/version.h"
#include <cmath> 
#include <stdexcept>
#include "version.txt"


namespace models {

    ModelMohr::ModelMohr(unsigned short option)
        : ModelAnisotropic(option) {
    }
    UInt ModelMohr::getMinorVersion() const {
        return UPDATE_VERSION;
    }

    std::wstring ModelMohr::getProperties() const {
        return ModelAnisotropic::getProperties() + std::wstring(
            L",cohesion-x,cohesion-y,cohesion-z,"
            L"friction-x,friction-y,friction-z,"
            L"dilation-x,dilation-y,dilation-z,"
            L"tension-x,tension-y,tension-z,"
            L"flag-brittle-x,flag-brittle-y,flag-brittle-z");
    }

    Variant ModelMohr::getProperty(UInt index) const {
        if (index <= 12) {
            return ModelAnisotropic::getProperty(index);
        }
        else {
            switch (index) {
            case 13: return cohesionX_;
            case 14: return cohesionY_;
            case 15: return cohesionZ_;
            case 16: return frictionX_;
            case 17: return frictionY_;
            case 18: return frictionZ_;
            case 19: return dilationX_;
            case 20: return dilationY_;
            case 21: return dilationZ_;
            case 22: return tensionX_;
            case 23: return tensionY_;
            case 24: return tensionZ_;
            case 25: return brittleX_;
            case 26: return brittleY_;
            case 27: return brittleZ_;
            default:
                throw std::runtime_error("Invalid property index Mohr.");
            }
        }
        return 0.0;
    }

    void ModelMohr::setProperty(UInt index, const Variant& p, UInt restoreVersion) {
        if (index <= 12) {
            ModelAnisotropic::setProperty(index, p, restoreVersion);
        }
        else {
            switch (index) {
            case 13: cohesionX_ = p.toDouble(); break;
            case 14: cohesionY_ = p.toDouble(); break;
            case 15: cohesionZ_ = p.toDouble(); break;
            case 16: frictionX_ = p.toDouble(); break;
            case 17: frictionY_ = p.toDouble(); break;
            case 18: frictionZ_ = p.toDouble(); break;
            case 19: dilationX_ = p.toDouble(); break;
            case 20: dilationY_ = p.toDouble(); break;
            case 21: dilationZ_ = p.toDouble(); break;
            case 22: tensionX_ = p.toDouble(); break;
            case 23: tensionY_ = p.toDouble(); break;
            case 24: tensionZ_ = p.toDouble(); break;
            case 25: brittleX_ = p.toBool(); break;
            case 26: brittleY_ = p.toBool(); break;
            case 27: brittleZ_ = p.toBool(); break;
            default:
                throw std::runtime_error("Invalid property index for input parameters Mohr.");
            }
        }
    }
    bool ModelMohr::isPropertyAdvanced(UInt i) const {
        if (i <= 12)
            return ModelAnisotropic::isPropertyAdvanced(i);
        else if (i >= 25 && i <= 27)
            return true;  // Mark brittle flags as advanced.
        return false;
    }

    void ModelMohr::copy(const ConstitutiveModel* m) {
        const ModelMohr* mm = dynamic_cast<const ModelMohr*>(m);
        if (!mm)
            throw std::runtime_error("Internal error: constitutive model dynamic cast failed.");
        //
        ModelAnisotropic::copy(m);
        //
        cohesionX_ = mm->cohesionX_;
        cohesionY_ = mm->cohesionY_;
        cohesionZ_ = mm->cohesionZ_;
        frictionX_ = mm->frictionX_;
        frictionY_ = mm->frictionY_;
        frictionZ_ = mm->frictionZ_;
        dilationX_ = mm->dilationX_;
        dilationY_ = mm->dilationY_;
        dilationZ_ = mm->dilationZ_;
        tensionX_ = mm->tensionX_;
        tensionY_ = mm->tensionY_;
        tensionZ_ = mm->tensionZ_;
        brittleX_ = mm->brittleX_;
        brittleY_ = mm->brittleY_;
        brittleZ_ = mm->brittleZ_;
    }
    void ModelMohr::initialize(UByte d, State* s) {
        ConstitutiveModel::initialize(d, s);
        updateParameters();
    }

    void ModelMohr::run(UByte d, State* s) {
        ConstitutiveModel::run(d, s);

        if (s->modulus_reduction_factor_ > 0.0)
            moduliReduction(s->modulus_reduction_factor_);

        // update shear/tension state flags
        if (s->state_ & shear_now)   s->state_ |= shear_past;
        s->state_ &= ~shear_now;
        if (s->state_ & tension_now) s->state_ |= tension_past;
        s->state_ &= ~tension_now;

        UInt iPlas = 0;
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

        // get principal stresses
        SymTensorInfo info;
        DVect3 prin = s->stnS_.getEigenInfo(&info);

        // 1) pack them into an array
        Double p[3] = { prin.x(), prin.y(), prin.z() };

        // 2) find index of the largest principal stress
        int dirMax = 0;
        if (p[1] > p[dirMax]) dirMax = 1;
        if (p[2] > p[dirMax]) dirMax = 2;
        UInt i1 = (dirMax + 1) % 3;

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
        //Double sf1[3] = { sf1_x_, sf1_y_, sf1_z_ };
        //Double sf3[3] = { sf3_x_, sf3_y_, sf3_z_ };

        int matIdx = globalOrder[0]; // material axis for principal direction
        Double fs = nph[matIdx] * p[dirMax]
            - p[i1]
            - csn[matIdx];
        Double fsd = fs / rc[matIdx];
        Double ft = p[dirMax] - tensionArr[matIdx];

        if (fsd > 0.0 && fsd >= ft) 
            shearCorrection(s, &prin, &iPlas, fs, matIdx, sc1, sc2, sc3); // If needed, pass matIdx as well
        
        else if (ft > 0.0) 
            tensionCorrection(s, &prin, &iPlas, ft, dirMax, globalOrder, tensionArr, e1, e2, brittleArr);
            apexCorrection(s, &prin, &iPlas,  globalOrder, friction, cohesion, tensionArr, brittleArr);
        
        if (iPlas) {
            s->stnS_ = info.resolve(prin);
            s->viscous_ = false;
        }
    }
    bool ModelMohr::updateParameters() {
        if (cohesionX_ < 0.0 || cohesionY_ < 0.0 || cohesionZ_ < 0.0)
            throw std::runtime_error("Mohr-Coulomb type model: cohesion is not allowed to be less than 0.");

        if (frictionX_ >= 90.0 || frictionX_ < 0.0 ||
            frictionY_ >= 90.0 || frictionY_ < 0.0 ||
            frictionZ_ >= 90.0 || frictionZ_ < 0.0)
            throw std::runtime_error("Mohr-Coulomb type model: friction angle is not in the valid range of 0 to 90.");

        if (dilationX_ > frictionX_ || dilationY_ > frictionY_ || dilationZ_ > frictionZ_)
            throw std::runtime_error("Mohr-Coulomb type model: dilation angle is not allowed to be greater than the friction angle.");
        // --- For X-direction:
        {
            double rsinX = std::sin(frictionX_ * degrad);
            nph_x_ = (1.0 + rsinX) / (1.0 - rsinX);
            csn_x_ = 2.0 * cohesionX_ * std::sqrt(nph_x_);
            rsinX = std::sin(dilationX_ * degrad);
            nps_x_ = (1.0 + rsinX) / (1.0 - rsinX);
            rc_x_ = std::sqrt(1.0 + nph_x_ * nph_x_);

            // Tension update for X-direction:
            if (frictionX_ > 0.0) {
                double apexX = cohesionX_ / std::tan(frictionX_ * degrad);
                tensionX_ = std::min(tensionX_, apexX);
            }
            else if (cohesionX_ == 0.0) {
                tensionX_ = 0.0;
            }

            double ra_x = e1_x_ - nps_x_ * e2_x_;
            double rb_x = e2_x_ - nps_x_ * e1_x_;
            double rd_x = ra_x - rb_x * nph_x_;
            sc1_x_ = ra_x / rd_x;
            sc3_x_ = rb_x / rd_x;
            sc2_x_ = e2_x_ * (1.0 - nps_x_) / rd_x;
        }

        // --- For Y-direction:
        {
            double rsinY = std::sin(frictionY_ * degrad);
            nph_y_ = (1.0 + rsinY) / (1.0 - rsinY);
            csn_y_ = 2.0 * cohesionY_ * std::sqrt(nph_y_);
            rsinY = std::sin(dilationY_ * degrad);
            nps_y_ = (1.0 + rsinY) / (1.0 - rsinY);
            rc_y_ = std::sqrt(1.0 + nph_y_ * nph_y_);

            // Tension update for Y-direction:
            if (frictionY_ > 0.0) {
                double apexY = cohesionY_ / std::tan(frictionY_ * degrad);
                tensionY_ = std::min(tensionY_, apexY);
            }
            else if (cohesionY_ == 0.0) {
                tensionY_ = 0.0;
            }

            double ra_y = e1_y_ - nps_y_ * e2_y_;
            double rb_y = e2_y_ - nps_y_ * e1_y_;
            double rd_y = ra_y - rb_y * nph_y_;
            sc1_y_ = ra_y / rd_y;
            sc3_y_ = rb_y / rd_y;
            sc2_y_ = e2_y_ * (1.0 - nps_y_) / rd_y;
        }

        // --- For Z-direction:
        {
            double rsinZ = std::sin(frictionZ_ * degrad);
            nph_z_ = (1.0 + rsinZ) / (1.0 - rsinZ);
            csn_z_ = 2.0 * cohesionZ_ * std::sqrt(nph_z_);
            rsinZ = std::sin(dilationZ_ * degrad);
            nps_z_ = (1.0 + rsinZ) / (1.0 - rsinZ);
            rc_z_ = std::sqrt(1.0 + nph_z_ * nph_z_);

            // Tension update for Z-direction:
            if (frictionZ_ > 0.0) {
                double apexZ = cohesionZ_ / std::tan(frictionZ_ * degrad);
                tensionZ_ = std::min(tensionZ_, apexZ);
            }
            else if (cohesionZ_ == 0.0) {
                tensionZ_ = 0.0;
            }

            double ra_z = e1_z_ - nps_z_ * e2_z_;
            double rb_z = e2_z_ - nps_z_ * e1_z_;
            double rd_z = ra_z - rb_z * nph_z_;
            sc1_z_ = ra_z / rd_z;
            sc3_z_ = rb_z / rd_z;
            sc2_z_ = e2_z_ * (1.0 - nps_z_) / rd_z;
        }

        return true;
    }

    bool ModelMohr::updateParameters(bool bEUpdated, Double* sf1_x_, Double* sf3_x_, Double* sf1_y_, Double* sf3_y_, Double* sf1_z_, Double* sf3_z_) {
        if (cohesionX_ < 0.0 || cohesionY_ < 0.0 || cohesionZ_ < 0.0)
            throw std::runtime_error("Mohr-Coulomb type model: cohesion is not allowed less than 0.");
        if (frictionX_ >= 90.0 || frictionX_ < 0.0 ||
            frictionY_ >= 90.0 || frictionY_ < 0.0 ||
            frictionZ_ >= 90.0 || frictionZ_ < 0.0)
            throw std::runtime_error("Mohr-Coulomb type model: friction angle is not in the valid range of 0 to 90.");
        if (dilationX_ > frictionX_ || dilationY_ > frictionY_ || dilationZ_ > frictionZ_)
            throw std::runtime_error("Mohr-Coulomb type model: dilation angle is not allowed greater than friction angle.");

        // --- For X-direction:
        {
            Double rsinX = std::sin(frictionX_ * degrad);
            nph_x_ = (1.0 + rsinX) / (1.0 - rsinX);
            csn_x_ = 2.0 * cohesionX_ * std::sqrt(nph_x_);
            rsinX = std::sin(dilationX_ * degrad);
            nps_x_ = (1.0 + rsinX) / (1.0 - rsinX);
            rc_x_ = std::sqrt(1.0 + nph_x_ * nph_x_);

            // Tension update for X-direction:
            if (frictionX_ > 0.0) {
                Double apexX = cohesionX_ / std::tan(frictionX_ * degrad);
                tensionX_ = std::min(tensionX_, apexX);
            }
            else if (cohesionX_ == 0.0) {
                tensionX_ = 0.0;
            }

            Double ra_x = e1_x_ - nps_x_ * e2_x_;
            Double rb_x = e2_x_ - nps_x_ * e1_x_;
            Double rd_x = ra_x - rb_x * nph_x_;
            sc1_x_ = ra_x / rd_x;
            sc3_x_ = rb_x / rd_x;
            sc2_x_ = e2_x_ * (1.0 - nps_x_) / rd_x;
            if (sf1_x_)* sf1_x_ = 1.0 / rd_x;
            if (sf3_x_)* sf3_x_ = -nps_x_ / rd_x;
        }

        // --- For Y-direction:
        {
            Double rsinY = std::sin(frictionY_ * degrad);
            nph_y_ = (1.0 + rsinY) / (1.0 - rsinY);
            csn_y_ = 2.0 * cohesionY_ * std::sqrt(nph_y_);
            rsinY = std::sin(dilationY_ * degrad);
            nps_y_ = (1.0 + rsinY) / (1.0 - rsinY);
            rc_y_ = std::sqrt(1.0 + nph_y_ * nph_y_);

            // Tension update for Y-direction:
            if (frictionY_ > 0.0) {
                Double apexY = cohesionY_ / std::tan(frictionY_ * degrad);
                tensionY_ = std::min(tensionY_, apexY);
            }
            else if (cohesionY_ == 0.0) {
                tensionY_ = 0.0;
            }

            Double ra_y = e1_y_ - nps_y_ * e2_y_;
            Double rb_y = e2_y_ - nps_y_ * e1_y_;
            Double rd_y = ra_y - rb_y * nph_y_;
            sc1_y_ = ra_y / rd_y;
            sc3_y_ = rb_y / rd_y;
            sc2_y_ = e2_y_ * (1.0 - nps_y_) / rd_y;
            if (sf1_y_)* sf1_y_ = 1.0 / rd_y;
            if (sf3_y_)* sf3_y_ = -nps_y_ / rd_y;
        }

        // --- For Z-direction:
        {
            Double rsinZ = std::sin(frictionZ_ * degrad);
            nph_z_ = (1.0 + rsinZ) / (1.0 - rsinZ);
            csn_z_ = 2.0 * cohesionZ_ * std::sqrt(nph_z_);
            rsinZ = std::sin(dilationZ_ * degrad);
            nps_z_ = (1.0 + rsinZ) / (1.0 - rsinZ);
            rc_z_ = std::sqrt(1.0 + nph_z_ * nph_z_);

            // Tension update for Z-direction:
            if (frictionZ_ > 0.0) {
                Double apexZ = cohesionZ_ / std::tan(frictionZ_ * degrad);
                tensionZ_ = std::min(tensionZ_, apexZ);
            }
            else if (cohesionZ_ == 0.0) {
                tensionZ_ = 0.0;
            }

            Double ra_z = e1_z_ - nps_z_ * e2_z_;
            Double rb_z = e2_z_ - nps_z_ * e1_z_;
            Double rd_z = ra_z - rb_z * nph_z_;
            sc1_z_ = ra_z / rd_z;
            sc3_z_ = rb_z / rd_z;
            sc2_z_ = e2_z_ * (1.0 - nps_z_) / rd_z;
            if (sf1_z_)* sf1_z_ = 1.0 / rd_z;
            if (sf3_z_)* sf3_z_ = -nps_z_ / rd_z;
        }

        if (!bEUpdated)
            ModelAnisotropic::updateParameters();

        return !bEUpdated;
    }
    std::array<Double, 3> ModelMohr::moduliReduction(const Double& factor) {
        std::array<Double, 3> newG = ModelAnisotropic::moduliReduction(factor);

        { // X
            Double ra = e1_x_ - nps_x_ * e2_x_;
            Double rb = e2_x_ - nps_x_ * e1_x_;
            Double rd = ra - rb * nph_x_;
            sc1_x_ = ra / rd;
            sc3_x_ = rb / rd;
            sc2_x_ = e2_x_ * (1.0 - nps_x_) / rd;
        }
        { // Y
            Double ra = e1_y_ - nps_y_ * e2_y_;
            Double rb = e2_y_ - nps_y_ * e1_y_;
            Double rd = ra - rb * nph_y_;
            sc1_y_ = ra / rd;
            sc3_y_ = rb / rd;
            sc2_y_ = e2_y_ * (1.0 - nps_y_) / rd;
        }
        { // Z
            Double ra = e1_z_ - nps_z_ * e2_z_;
            Double rb = e2_z_ - nps_z_ * e1_z_;
            Double rd = ra - rb * nph_z_;
            sc1_z_ = ra / rd;
            sc3_z_ = rb / rd;
            sc2_z_ = e2_z_ * (1.0 - nps_z_) / rd;
        }
        return { newG_x_, newG_y_, newG_z_ };
    }

    //void ModelMohr::apexCorrection(
    //    State* s, DVect3* prin, UInt* iPlasticity, int dirMax, int globalOrder[3],
    //    Double friction[3], Double cohesion[3], Double tensionArr[3], bool brittleArr[3])
    //{
    //    int matIdx = globalOrder[dirMax];
    //    Double* pArr[3] = { &(prin->rx()), &(prin->ry()), &(prin->rz()) };

    //    if (friction[matIdx] > 0.0) {
    //        Double apex = cohesion[matIdx] / std::tan(friction[matIdx] * degrad);
    //        if (*(pArr[dirMax]) >= apex) {
    //            if (iPlasticity) *iPlasticity = 4;
    //            s->state_ |= tension_now;
    //            if (brittleArr[matIdx]) tensionArr[matIdx] = 0.0;
    //            *(pArr[dirMax]) = apex;
    //        }
    //    }
    //}
    void ModelMohr::apexCorrection(State* s, DVect3* prin, UInt* iPlasticity, int globalOrder[3],
            Double friction[3], Double cohesion[3], Double tensionArr[3], bool brittleArr[3]) {
       
        Double* pArr[3] = { &(prin->rx()), &(prin->ry()), &(prin->rz()) };

        for (int i = 0; i < 3; ++i) {
            int globalIdx = globalOrder[i]; // Which global axis is this principal?
            if (friction[globalIdx] > 0.0) {
                double apex = cohesion[globalIdx] / std::tan(friction[globalIdx] * degrad);
                if (*(pArr[i]) >= apex) {
                    if (iPlasticity) *iPlasticity = 4;
                    s->state_ |= tension_now;
                    if (brittleArr[globalIdx]) tensionArr[globalIdx] = 0.0;
                    *(pArr[i]) = apex;
                }
            }
        }
    }
    void ModelMohr::tensionCorrection( State* s, DVect3* prin, UInt* iPlasticity, const Double& ft, UInt dirMax,
        int globalOrder[3], Double tensionArr[3], Double e1[3], Double e2[3], bool brittleArr[3])
    {
        s->state_ |= tension_now;

        int globalDir = globalOrder[dirMax];
        Double tensionDir = tensionArr[globalDir];
        if (brittleArr[globalDir])
            tensionArr[globalDir] = 0.0;

        Double p[3] = { prin->x(), prin->y(), prin->z() };

        UInt i1 = (dirMax + 1) % 3;
        UInt i2 = (dirMax + 2) % 3;

        // (No need to get global_i1 and global_i2 unless you want direction-dependent partitioning, which you don't here.)
        Double e1_dir = e1[globalDir];
        Double e2_dir = e2[globalDir];

        Double f1 = p[i1] - tensionDir;
        Double f2 = p[i2] - tensionDir;

        if (f1 > 0.0) {
            if (iPlasticity) *iPlasticity = 4;
            p[0] = p[1] = p[2] = tensionDir;
        }
        else if (f2 > 0.0) {
            if (iPlasticity) *iPlasticity = 3;
            p[i1] -= (f2 + ft) * e2_dir / (e1_dir + e2_dir);
            p[i2] = tensionDir;
            p[dirMax] = tensionDir;
        }
        else {
            if (iPlasticity) *iPlasticity = 2;
            Double tco = ft * e2_dir / e1_dir;
            p[i1] -= tco;
            p[i2] -= tco;
            p[dirMax] = tensionDir;
        }

        prin->rx() = p[0];
        prin->ry() = p[1];
        prin->rz() = p[2];
    }


    void ModelMohr::shearCorrection(
        State* s, DVect3* prin, UInt* iPlasticity, const Double& fs, int matIdx,
        Double sc1[3], Double sc2[3], Double sc3[3])
    {
        if (iPlasticity) *iPlasticity = 1;
        s->state_ |= shear_now;
        prin->rx() += fs * sc1[matIdx];
        prin->ry() += fs * sc2[matIdx];
        prin->rz() += fs * sc3[matIdx];
    }

    Double ModelMohr::getStressStrengthRatio(const SymTensor& st) const {
        // 1. Get principal stresses
        DVect3 prin = st.getEigenInfo();
        double vals[3] = { prin.x(), prin.y(), prin.z() };

        // 2. Find which principal is the max (most critical for failure)
        int dirMax = 0;
        if (vals[1] > vals[dirMax]) dirMax = 1;
        if (vals[2] > vals[dirMax]) dirMax = 2;

        // 3. Build mapping from principal axes to global axes
        // Use st.s11(), st.s22(), st.s33() as your global components
        Double s11 = st.s11();
        Double s22 = st.s22();
        Double s33 = st.s33();
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

        // 4. Use anisotropic parameters for the principal-to-material mapping
        int matIdx = globalOrder[dirMax]; // Which material axis is this principal direction?

        Double friction[3] = { frictionX_, frictionY_, frictionZ_ };
        Double cohesion[3] = { cohesionX_, cohesionY_, cohesionZ_ };
        Double tension[3] = { tensionX_,  tensionY_,  tensionZ_ };

        Double tanf = std::tan(friction[matIdx] * degrad);
        Double tcut = friction[matIdx] ? std::min(tension[matIdx], (cohesion[matIdx] / tanf)) : tension[matIdx];

        Double sig1 = vals[dirMax];
        // Pick the *smallest* of the other two as the minor, for safety
        Double sig3 = vals[(dirMax == 0) ? 1 : 0];
        if (dirMax == 2) sig3 = std::min(vals[0], vals[1]);
        else if (dirMax == 1) sig3 = std::min(vals[0], vals[2]);
        else if (dirMax == 0) sig3 = std::min(vals[1], vals[2]);

        Double rat = 10.0;

        if (tcut - sig1 <= 0.0)
            rat = 0.0;
        else {
            Double sinf = std::sin(friction[matIdx] * degrad);
            Double denom = 1.0 - sinf;
            Double nph = std::numeric_limits<Double>::max();
            if (denom)
                nph = (1.0 + sinf) / denom;
            Double sig1f = nph * sig1 - 2.0 * cohesion[matIdx] * std::sqrt(nph);
            denom = sig1 - sig3;
            if (denom)
                rat = (sig1 - sig1f) / denom;
        }
        rat = std::min(rat, 10.0);
        return rat;
    }

    void ModelMohr::scaleProperties(const Double &scale, const std::vector<UInt> &props) {
        if (std::abs(scale - 1.0) < 1e-12) return;

        auto clampValue = [](double value, double minVal, double maxVal) {
            return (value < minVal) ? minVal : (value > maxVal) ? maxVal : value;
        };

        auto scaleAngle = [&](double angleDeg, double scl) {
            if (angleDeg <= 0.0) return 0.0;
            double rad = angleDeg * degrad;
            double t = std::tan(rad) * scl;
            if (t < 0.0) t = 0.0;
            double out = std::atan(t) / degrad;
            return clampValue(out, 0.0, 89.0);
        };

        for (UInt p : props) {
            switch (p) {
            case 13: cohesionX_ *= scale; break;
            case 14: cohesionY_ *= scale; break;
            case 15: cohesionZ_ *= scale; break;
            case 16: frictionX_ = scaleAngle(frictionX_, scale); break;
            case 17: frictionY_ = scaleAngle(frictionY_, scale); break;
            case 18: frictionZ_ = scaleAngle(frictionZ_, scale); break;
            case 19: dilationX_ = scaleAngle(dilationX_, scale); break;
            case 20: dilationY_ = scaleAngle(dilationY_, scale); break;
            case 21: dilationZ_ = scaleAngle(dilationZ_, scale); break;
            case 22: tensionX_ *= scale; break;
            case 23: tensionY_ *= scale; break;
            case 24: tensionZ_ *= scale; break;
            default:
                throw std::invalid_argument("Invalid property index in scaleProperties: " + std::to_string(p));
            }
        }
        updateParameters();
    }


} // namespace models
// EOF
