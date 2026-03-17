#pragma once

#include "modelmohr.h"
#include <array>

namespace models {

    class ModelStrainSofteningAnisotropic : public ModelMohr {
    public:
        ModelStrainSofteningAnisotropic();

        // If you need to change the model's identifier:
        virtual String  getName() const { return L"anisotropic-strain-softening"; }
        virtual String  getFullName() const { return L"Anisotropic Strain Softening"; }
        virtual String  getPluginName() const { return getName(); }
        virtual UInt    getMinorVersion() const;
        virtual String  getProperties() const;
        virtual Variant getProperty(UInt index) const;
        virtual void    setProperty(UInt index, const Variant& p, UInt restoreVersion = 0);
        virtual ModelStrainSofteningAnisotropic* clone() const { return (NEW("ModelStrainSofteningAnisotropic") ModelStrainSofteningAnisotropic()); }
        virtual void    copy(const ConstitutiveModel* mod);
        virtual void    initialize(UByte dim, State* s);
        virtual void    run(UByte dim, State* s);

        // Optional property interface methods:
        virtual bool    isPropertyReadOnly(UInt i) const;
        virtual bool    isPropertyAdvanced(UInt i) const;
        virtual bool    supportsHystereticDamping() const { return true; }
        virtual bool    supportsStressStrengthRatio() const { return true; }
        virtual bool    supportsPropertyScaling() const { return false; }
        virtual bool    isModelAdvanced() const { return true; }

    protected:
        //std::array<double, 3> sf1_ = { 0.0, 0.0, 0.0 };
        //std::array<double, 3> sf3_ = { 0.0, 0.0, 0.0 };
        Double sf1_x_ = 0;
        Double sf1_y_ = 0;
        Double sf1_z_ = 0;

        Double sf3_x_ = 0;
        Double sf3_y_ = 0;
        Double sf3_z_ = 0;
        
        double sHPX_ = 0.0;
        double sHPY_ = 0.0;
        double sHPZ_ = 0.0;
        double tHPX_ = 0.0;
        double tHPY_ = 0.0;
        double tHPZ_ = 0.0;
        
        // Table identifiers for anisotropic strain softening (as strings)
        String cTableX_, cTableY_, cTableZ_;
        String fTableX_, fTableY_, fTableZ_;
        String dTableX_, dTableY_, dTableZ_;
        String tTableX_, tTableY_, tTableZ_;
     

        // Pointers (or indices) to the tables (retrieved during initialization)
        void* iCohesionX_ = nullptr;
        void* iCohesionY_ = nullptr;
        void* iCohesionZ_ = nullptr;
        void* iFrictionX_ = nullptr;
        void* iFrictionY_ = nullptr;
        void* iFrictionZ_ = nullptr;
        void* iDilationX_ = nullptr;
        void* iDilationY_ = nullptr;
        void* iDilationZ_ = nullptr;
        void* iTensionX_ = nullptr;
        void* iTensionY_ = nullptr;
        void* iTensionZ_ = nullptr;
    };

} // namespace models
