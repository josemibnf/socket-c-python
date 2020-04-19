    import socket
import multiprocessing
from random import randint, uniform,random


#Datos de Configuracion
file=open("server.cfg","r")
nom=file.readline()
mac=file.readline()
udp_port=file.readline()
tcp_port=file.readline()
file.close()

nom = nom[4:]
mac = mac[4:]
udp_port = udp_port[9:]
tcp_port = tcp_port[9:]
print ("SERVER:" 'nom', 'mac', 'udp_port', 'tcp_port',"\n\n")


#Datos de esquipos
equips_dat=open("equips.dat","r")
line=equips_dat.readline
equips= {}
while line!="":
     mac=line[6:]
     nom=line[:5]
     equips.update(mac, {"nom": nom, "estat": "DISCONNECTED", "num": 0})
     line=equips_dat.readline
for x in equips:
    print (x)
    for y in equips[x]:
        print (y,':',equips[x][y])
print("\n")


#Peticiones de Registro
Descriptor = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
Descriptor.bind(('local_host',udp_port))
Descriptor.listen(5)  #clientes en cola

def autorizat(addr):
    for x in equips:
        if x==addr:
            return True
    return False

def comprobacion(addr, data):
    datos= data.split(" ")
    return equips[addr]["nom"]==datos[1] and equips[addr]==datos[2] and equips[addr]["num"]==datos[3]

def udp_conexion(data, addr):
    print ("Nueva conexion establecida\n")
    if autorizat(addr)==False:  #no autorizado
        Descriptor.sendto("0x03" + " " + "Peticion de registro rechazada" + "\n", ( addr, udp_port))

    elif comprobacion(addr, data)==False:  #autorizado, datos incorrectos
        Descriptor.sendto("0x02" + " " + "No se ha podido registrar con el servidor" + "\n", ( addr, udp_port))

    elif equips[addr]["estat"]=="REGISTERED":  #autorizado, datos correctos, registrado
        Descriptor.sendto("0x01" + " " + equips[addr]["nom"] + " " + addr + " " + equips[addr]["num"] + " "  + tcp_port + "\n", ( addr, udp_port))

    else: #autorizado, datos correctos, desconectado
        equips[addr]["num"]=randint(0,999)
        Descriptor.sendto("0x01" + " " + "Peticion de registro rechazada" + "\n", ( addr, udp_port))
        equips[addr]["estat"]="REGISTERED"
    
while True:
    data, addr = Descriptor.recvfrom(20)
    multiprocessing.Process(target=udp_conexion, args=( data, addr)).start()

