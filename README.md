# 🌊 CyanoSense 2.0 (CS2.0) – Low-Cost Hyperspectral Sensor for CyanoHAB Monitoring

**CyanoSense 2.0 (CS2.0)** is a compact, low-cost, open-source spectroradiometer designed to support real-time and offline monitoring of **Cyanobacterial Harmful Algal Blooms (CyanoHABs)** in inland and coastal waters. This repository includes all components needed to **reproduce, assemble, and operate** the CS2.0 device.

> 🛰️ CS2.0 was developed as a complementary in situ validation tool for satellite-based CyanoHAB monitoring.  
> 📝 A scientific publication describing CS2.0 is currently under review. Please cite the work appropriately (see [Citation coming soon](#-citation)) and refer to the paper for broader scientific context and applications.

---

## 📦 What's in This Repository?

This repository includes:

1. 🧩 **3D Printed Parts** – STL files and models for the CS2.0 housing and optical mount components.  
2. 🛠️ **Mechanical Assembly Instructions** – Step-by-step guide with diagrams and instructions for assembling CS2.0.  
3. ⚡ **Electronic Schematics** – Circuit diagrams and wiring layout.  
4. 📷 **Prototype Pictures** – Reference images of a fully built and field-deployed CS2.0 unit.  
5. 💾 **Offline Data Download Instructions** – Guidelines for retrieving stored measurements from the device when internet is not available.  
6. 📂 **Documents Folder** – Contains the **Bill of Materials** (BOM) and **Hamamatsu documentation** including the wavelength space coefficients and sensor specifications.  
   - 📌 An **exhaustive list of all components used** in this project can be found in the Bill of Materials (`Documents/Bill of Materials` or similar).  

🎥 A **tutorial video demonstrating the operation** of CS2.0 is also provided: https://youtu.be/9sZjeJtRidM

---

## 📡 Sensor Details

CS2.0 uses the **Hamamatsu C12880MA** mini-spectrometer to capture hyperspectral light spectra in the range of 380nm - 880nm.

- Records data in **6 channels**, each corresponding to a segment of the full sensor array.  
- Includes calibration code to convert raw intensity values into continuous **wavelength space** using a polynomial mapping.  
- Wavelength mapping is implemented using conversion coefficients provided in the Hamamatsu documentation (see `Documents/`) and handled in the included data extraction code (`/code/`).  
- Real-time or offline data download is managed through the [CoolTerm](https://freeware.the-meiers.org/) serial terminal tool, described in `/offline_data/`.

---

## 📚 Usage

To build a CS2.0 prototype and use it for yourself:
- Follow the mechanical and electrical instructions in the `/mechanical_assembly/` and `/electronics/` folders.
- Use `/offline_data/` for accessing data stored during field deployments without real-time upload.

### 🌐 Satellite Data Validation

CS2.0 can be used to **validate satellite-based CyanoHAB products**:
- Overlay CS2.0-derived in situ spectra with coincident satellite-derived reflectance (e.g., Sentinel-2, Landsat, PACE).
- Helps assess model accuracy and algorithm performance under real-world conditions.
- Supports efforts in operational satellite monitoring by offering ground-based reference measurements.

---

## 📄 Citation

> **This sensor and methodology are described in a scientific article currently under peer review.**  
> If you use or build on this work, **please cite the associated article** (citation will be added once published).  
> Also, please **refer to the paper** for a deeper understanding of how CS2.0 fits into the broader **CyanoHAB monitoring paradigm**, including its role in supporting satellite remote sensing efforts.

---

## 🪪 License

- **Code**: [MIT License](LICENSE)
- **Documentation, Schematics, and Hardware Designs**: [Creative Commons Attribution 4.0 International (CC BY 4.0)](https://creativecommons.org/licenses/by/4.0/)

---

## 📬 Contact

For questions, collaborations, or issues, please reach out to:  
**Chintan B. Maniyar** at
📧 [chintanmaniyar@uga.edu] OR [cyanotracker@gmail.com]
