# MWR-Hatpro

This repository contains Python scripts and Jupyter notebooks used to covert the measurements from Hatpro microwave radiometer into simple CSV files.

## Usage

First cd into the source folder:

```cd ~loh/workspace/mwr_hatpro/src```

and run

```python get_csv.py {INPUT_DIR}```

where {INPUT_DIR} is the path to the MWR measurement data (ensure that the files from the FTP server have been uncompressed). The script will recursively go through sub-folders and locate the daily measurements.

To designate output location, run

```python get_csv.py {INPUT_DIR} --to {OUTPUT_DIR}```

where {OUTPUT_DIR} is the path to the resulting output files. If {OUTPUT_DIR} is not given, the output files will be written to the current working directory.

The list of variables translated by the script is defined in ret.json file.

As of this writing, four output files will be written per day, named as

```{YYYY}-{MM}-{DD}-{TYPE}.csv```

where {TYPE} corresponds to the type of variable -- IWV (integrated waver vapour [kg/m^2]), TPC (temperature profile [K]), AHP (absolute humidity profile [g/m^3]) and RHP (relative humidity profile [%]).