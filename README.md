# Smart Coupled Flow–Geomechanical Upscaling for Oil Sands

This repository contains the source code developed for the MSc thesis:

Ou, X. (2026). *Smart Coupled Flow–Geomechanical Upscaling Technique for Oil Sands*.  
University of Alberta.

---

## Overview

This project presents a machine learning–assisted framework for upscaling coupled flow–geomechanical behavior in heterogeneous oil sands.

A 3D convolutional neural network (CNN) is trained to predict mechanical and hydro-mechanical responses from fine-scale geological realizations, including:

- Deviator stress (qa)  
- Volumetric strain  
- Pore pressure  
- Plastic strain  

These predictions are used to derive upscaled anisotropic constitutive parameters, which are implemented in FLAC3D for large-scale simulations.

---

## Workflow

1. Generate 3D geological realizations  
2. Perform fine-scale coupled flow–geomechanical simulations (FLAC3D)  
3. Train 3D CNN model  
4. Predict stress–strain and pore pressure responses  
5. Derive upscaled parameters (E50, Poisson’s ratio, friction angle, dilation, cohesion)  
6. Implement anisotropic strain-softening model in FLAC3D  
7. Apply to reservoir-scale case studies  

---

## Repository Structure

- `flac3d_fine_simulation/`  
  Fine-scale FLAC3D simulation models and scripts  

- `cnn_model/`  
  3D CNN architecture, training, and prediction scripts  

- `constitutive_model/`  
  Development of anisotropic strain-softening constitutive model  

- `case_study/`  
  Application of the upscaled model in larger-scale simulations  

---

## Requirements

- Python 3.10  
- PyTorch  
- NumPy  
- FLAC3D 7.0  

---

## Reproducibility

Due to the large size of the dataset and high computational cost of fine-scale simulations, the full dataset is not included.

This repository provides:
- Model architecture and training scripts  
- Upscaling procedures  
- Example input/output data  

The workflow can be reproduced following the methodology described in the thesis.



## Citation

If you use this code, please cite:

Ou, X. (2026). *Smart Coupled Flow–Geomechanical Upscaling Technique for Oil Sands*. University of Alberta.
