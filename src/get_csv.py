from pathlib import Path
from tqdm import tqdm

import pandas as pd
import numpy as np
import datetime

import click

# Set-up local (and temporary) sys.path for import
# All scripts for calculations and plots need this
from context import add_path


add_path(Path(".").resolve())

try:
    import plib.io
except Exception:
    raise Exception("Issue with dynamic import")


def get_dt(time):
    d_0 = datetime.datetime(2001, 1, 1)
    return d_0 + datetime.timedelta(seconds=time)


def get_ang(ang):
    if ang > 0:
        sign = +1
    else:
        sign = -1
    az = np.abs(ang / 1e3)

    if az > 360:
        az = (az - 1e3) * 10
    elev = np.abs(az - int(az)) * 1e2 + 1e2

    return az, sign * elev


def get_df(item) -> pd.DataFrame:
    unpack = plib.io.unpack

    with open(item, "rb") as f:
        bin = f.read()

        b_type = unpack(bin, 0)[0]
        n_rec = unpack(bin, 4)[0]

        if b_type == 594811068:  # IWV
            cols = [
                "date",
                "time",
                "rain_flag",
                "IWV [kg/m^2]",
                "angle [deg]",
                "elevation [deg]",
            ]

            pos_0 = 24

            # Dump data into Pandas Dataframe
            rec = pd.DataFrame(columns=cols)

            for i in range(n_rec):
                t = get_dt(unpack(bin, pos_0, type="i")[0])
                pos_0 += 4
                rf = unpack(bin, pos_0, "b")[0]
                pos_0 += 1

                iwv = unpack(bin, pos_0, "f")[0]
                pos_0 += 4
                ang, elev = get_ang(unpack(bin, pos_0, "f")[0])
                pos_0 += 4

                rec.loc[i] = [t.date(), t.time(), rf, iwv, ang, elev]
        elif b_type == 780798065:  # TPC
            cols = [
                "date",
                "time",
                "rain_flag",
            ]

            n_alt = unpack(bin, 24)[0]
            pos_0 = 28

            for _ in range(n_alt):
                cols.append(str(unpack(bin, pos_0)[0]))
                pos_0 += 4

            # Dump data into Pandas Dataframe
            rec = pd.DataFrame(columns=cols)

            for i in range(n_rec):
                t = get_dt(unpack(bin, pos_0, type="i")[0])
                pos_0 += 4
                rf = unpack(bin, pos_0, "b")[0]
                pos_0 += 1

                l_temp = []
                for _ in range(n_alt):
                    l_temp.append(unpack(bin, pos_0, "f")[0])
                    pos_0 += 4

                _row = [t.date(), t.time(), rf]
                _row.extend(l_temp)
                rec.loc[i] = _row
        elif b_type == 117343673:  # HPC
            cols = ["date", "time", "type"]

            n_alt = unpack(bin, 24)[0]
            pos_0 = 28

            for _ in range(n_alt):
                cols.append(str(unpack(bin, pos_0)[0]))
                pos_0 += 4

            # Dump data into Pandas Dataframe
            rec = pd.DataFrame(columns=cols)

            for i in range(n_rec):
                t = get_dt(unpack(bin, pos_0, type="i")[0])
                pos_0 += 4
                rf = unpack(bin, pos_0, "b")[0]
                pos_0 += 1

                ah = []
                for _ in range(n_alt):
                    ah.append(unpack(bin, pos_0, "f")[0])
                    pos_0 += 4

                _row = [t.date(), t.time(), "AHP"]
                _row.extend(ah)
                rec.loc[i] = _row

            # Skip RHMin and RHmax
            pos_0 += 8
            for i in range(n_rec):
                t = get_dt(unpack(bin, pos_0, type="i")[0])
                pos_0 += 4
                rf = unpack(bin, pos_0, "b")[0]
                pos_0 += 1

                rh = []
                for _ in range(n_alt):
                    rh.append(unpack(bin, pos_0, "f")[0])
                    pos_0 += 4

                _row = [t.date(), t.time(), "RHP"]
                _row.extend(rh)
                rec.loc[n_rec + i] = _row
        else:
            raise TypeError("Unrecognized Input Type")

        return rec


@click.command()
@click.argument("p_src", nargs=1)
@click.option("-t", "--to", help="Path to (alternative) output directory.")
def wrapper(p_src, to):
    if not (p_src := Path(p_src)).is_dir():
        raise ValueError("Not a valid input directory path")

    # Output options
    if to is None:
        p_dst = "."
    else:
        if not (to := Path(to)).is_dir():
            raise ValueError("Not a valid directory path")
        p_dst = to

    ret = plib.io.get_ret()

    for key in ret.keys():
        if key == 'HPC':
            l_key = sorted(p_src.rglob(f"*[0-9].{key}"))

            if len(l_key) == 0:
                print("No record found")
            else:
                df = pd.DataFrame()

                for item in (pbar := tqdm(l_key)):
                    pbar.set_description(f"Processing {item}")

                    df = pd.concat([df, get_df(item)])

                df = df.sort_values(by="time").reset_index(drop=True)

                for subkey in ['AHP', 'RHP']:
                    _df = df[df.type == subkey]
                    _df = _df.drop(columns='type')

                    f_path = f"{p_dst}/{df.date[0]}-{subkey}.csv"
                    plib.io.to_csv(_df, f_path, ret[key][subkey])
        else:
            l_key = sorted(p_src.rglob(f"*[0-9].{key}"))

            if len(l_key) == 0:
                print("No record found")
            else:
                df = pd.DataFrame()

                for item in (pbar := tqdm(l_key)):
                    pbar.set_description(f"Processing {item}")

                    df = pd.concat([df, get_df(item)])

                df = df.sort_values(by="time").reset_index(drop=True)

                f_path = f"{p_dst}/{df.date[0]}-{key}.csv"
                plib.io.to_csv(df, f_path, ret[key])


if __name__ == "__main__":
    wrapper()
