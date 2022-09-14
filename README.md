# MMI2022
Man Machine Interface 2022

### Conda Management

To use the provided conda enviroment, run

```bash
conda env create -f environment.yml
```

followed by 

```bash
conda activate MMI
```

The current conda enviroment can be exported with

```bash
conda env export | grep -v "^prefix: " > environment.yml
```

If you need to add any extra packages during development, try and use Conda first, then pip. If you add dependencies to the enviroment, re-export and update the conda enviroment.yml file.  

### DAD Submodule

Previous team code is stored in the DAD_Software submodule. To use it, a conda enviroment can be created from the dad-enviroment.yml file. Then run

```bash
conda activate MMI-DAD
python DAD.py
```