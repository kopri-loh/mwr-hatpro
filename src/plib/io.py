import struct as st
import json


# Define getter
def unpack(bd, pos, type="i"):
    if type in ["i", "f"]:
        return st.unpack(type, bd[pos : pos + 4])
    elif type == "d":
        return st.unpack(type, bd[pos : pos + 8])
    elif type == "b":
        return st.unpack(type, bd[pos : pos + 1])
    else:
        raise TypeError("Invalid Type Reference")


def get_ret():
    with open("../ret.json") as f_json:
        ret = json.load(f_json)

    return ret


def to_csv(df, f_path, desc):
    # Add comment and write CSV
    with open(f_path, "w") as f:
        f.write(f"{desc} \n")

    df.to_csv(f_path, mode="a", index=False)
