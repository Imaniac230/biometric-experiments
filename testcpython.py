#!/usr/bin/env python3

#sudo apt-get install python3-pigpio

def main():
    from ctypes import CDLL, POINTER, c_int16

    R503 = CDLL("./R503_fingerprint.so")
    R503.GetFingerprintData.restype = POINTER(c_int16 * R503.MaxPacketDataLen())
    finger_data = R503.GetFingerprintData().contents
    finger_data = finger_data[0:finger_data[:].index(-1)]
    finger_data = int("".join(map(hex, finger_data)).replace("0x", ""), base=16)

    print(f"\n\ttemplate (in python):\n{format(finger_data, 'x')}\n")

#    serhandle = R503.serOpen(bytes("/dev/ttyAMA0", 'utf-8'), 115200, 0)

    with open("finger_template.fpt", "r") as f:
        e = int(f.read(), base=16)
    print(f"\n\tfile (python):\n{format(e, 'x')}\n")


if __name__ == "__main__":
    main()
