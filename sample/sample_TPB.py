import struct as st
import pandas as pd
import numpy as np


# Define getter
def b_unpack(bd, pos, type="i"):
    if type in ["i", "f"]:
        return st.unpack(type, bd[pos : pos + 4])
    elif type == "d":
        return st.unpack(type, bd[pos : pos + 8])
    elif type == "b":
        return st.unpack(type, bd[pos : pos + 1])
    else:
        raise TypeError("Invalid Type Reference")


def get_df(bin):
    # TPB file code (459769847)
    if b_unpack(bin, 0)[0] != 459769847:
        raise TypeError("Invalid Input File")

    # Number of recorded samples
    n_sample = b_unpack(bin, 4)[0]

    # Number of altitude layers
    n_alt = b_unpack(bin, 24)[0]

    # Define output columns
    cols = ["time", "rain_flag"]

    pos_0 = 28
    for z in range(n_alt):
        cols.append(b_unpack(bin, pos_0)[0])
        pos_0 += 4

    # Dump data into Numpy array
    data = np.zeros((n_sample, n_alt+2))

    for i in range(n_sample):
        # Sample time
        data[i][0] = b_unpack(bin, pos_0)[0]
        pos_0 += 4

        # Rain flag
        data[i][1] = b_unpack(bin, pos_0, 'b')[0]
        pos_0 += 1

        # Temperatures
        for z in range(n_alt):
            data[i][z+2] = b_unpack(bin, pos_0, 'f')[0]
            pos_0 += 4

    return pd.DataFrame(columns=cols, data=data)


if __name__ == "__main__":
    f = open("ZENITH_220822_070956.TPB", "rb")
    df = get_df(f.read())

    print(df.T)

    df.to_csv("ZENITH_220822_070956.csv")
