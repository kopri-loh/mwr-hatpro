import struct as st


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
