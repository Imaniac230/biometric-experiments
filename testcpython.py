#!/usr/bin/env python3

#sudo apt-get install python3-pigpio

if __name__ == "__main__":

    import ctypes as ct

    r503 = ct.CDLL("./r503_fingerprint.so")
    r503.GetFingerprintData.restype = ct.POINTER(ct.c_int16 * 200)
    finger_data = r503.GetFingerprintData().contents
    finger_data = finger_data[0:finger_data[:].index(-1)]
    finger_data = int("".join(map(str, finger_data)))

    print(f"\n\ttemplate (in python):\n{finger_data}\n")

#    serhandle = r503.serOpen(bytes("/dev/ttyAMA0", 'utf-8'), 115200, 0)

    with open("finger_template.fpt", "r") as f:
        e = int(f.read(), base=10)
    print(f"\n\tfile (python):\n{e}\n")

