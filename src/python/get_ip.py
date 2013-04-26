import Tkinter as tk
from tkSimpleDialog import askstring
from tkMessageBox import showerror

def get_ip_address():
    root = tk.Tk()
    root.withdraw()

    ip = askstring( 'Server Address', 'Enter IP:' )

    if ip is None:
        return None
    
    ip = ip.strip()

    if ip is '':
        showerror( 'Error', 'Please enter a valid IP address' )
        return GetInput()

    if len(ip.split(".")) is not 4:
        showerror( 'Error', 'Please enter a valid IP address' )
        return GetInput()

    for octlet in ip.split("."):
        x = 0

        if octlet.isdigit():
            x = int(octlet)
        else:
            showerror( 'Error', 'Please enter a valid IP address' )
            return GetInput()
        
        if not ( x < 256 and x >= 0 ):
            showerror( 'Error', 'Please enter a valid IP address' )
            return GetInput()

    return ip
