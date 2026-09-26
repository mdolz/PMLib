# -*- coding: utf-8 -*-

#======================================================================
# DC2Device class
#======================================================================

import Device
import select
import termios
import serial
import time 
import sys

CONST_CONVERSION=0.03

## A DC2Meter device description
#
class DC2Device(Device.AttachedDevice):

	## Creates a DC2Meter device description and adds it to the
	## devices dictionary
	#
	# @param [in] name           The device name (used for identification, must be unique)
	# @param [in] computer       The computer the device is attached to
	# @param [in] url            The url of this device
	# @param [in] max_frequency  The maximum sample frequency of the device
	#
	def __init__(self, name, computer, url, max_frequency):
		self.n_lines = 1
		super(DC2Device, self).__init__(name, computer, url, max_frequency)


	## Read function
	#
	#  Reads data from DC2Device, pyserial package is needed in order to run
	#
	def read(self):
	##  Recoge los datos de consumo leyendo el puerto correpondiente
	##  Lee del dispositivo de medida DC de todos los canales    
		print "BUG: iniciamos el read"
		try:
			ser = serial.Serial(port=self.url , \
			baudrate=115200, \
			timeout= 1, \
			parity= serial.PARITY_NONE)
		except:
			print "Error, medidor de potencia mal conectado. Asegurarse que se ha definido correctamente el nombre del puerto serie."


		RELLENO=21       # 21  = 0x15 -> Relleno de 6 bits bajos de conversion
		MARCA=170        # 170 = 0xAA -> Marca de final de serie


		##Configuration variables

		ser.write("\x01")

		ser.flush()
		ser.flushInput()

		#Algunas veces en la primera peticion de configuracion salen datos incorrectos
		ser.write("\x5A")
		strdato = ser.read(1)


		ser.flush()
		ser.flushInput()
		
		ser.write("\x5A")
		
		datos = [0,0,0,0]
		
		for i in range (4):
			strdato = ser.read(1)
			dato = int(hex(ord(strdato)).replace('0x', ''), 16)
			datos[i]=dato


		if (datos[0] & 128 == 128):
			TAMDATA=2        # Numero de bits con los que se muestran los datos --> 10 bits
		else:
			TAMDATA=1        # Numero de bits con los que se muestran los datos --> 8 bits


		TAMTIME=0        # Numero de bytes con los que se muestra el time
		if (datos[0] & 64 == 64):
			TAMTIME+=1
		if (datos[0] & 32 == 32):
			TAMTIME+=1
		if (datos[0] & 16 == 16):
			TAMTIME+=1
		if (datos[0] & 8 == 8):
			TAMTIME+=1

		bits = [1,128,64,32,16,8,4,2,1,128,64,32,16,8,4,2,1,128,64,32,16,8,4,2,1]
#Comprobar que lineas estan activas
		self.n_lines= 0
		for i in range (25):
			if (datos[3-(24-i)/8] & bits[i] == bits[i]):
				self.n_lines+=1


#Calculo del numero maximo de datos producidos cada segundo
#Cada byte tiene un coste temporal de 90ms en ser enviado
 
		self.max_frequency=(1000000/(90*(self.n_lines*TAMDATA+TAMTIME+2)))

#Volver al modo conversacion
		ser.flush()
		ser.flushInput()
		ser.write("\x55")
		modo="conversacion"
		ser.read(1)
		ser.flush()
		ser.flushInput()

#Ignorar primer ciclo
		ser.read(self.n_lines*TAMDATA+TAMTIME+2)

		current     = [0] * self.n_lines
		power       = [0] * self.n_lines
		

#Empezamos a leer datos
		ant_marca = False;
		encontrado = False;

#Buscar dos marcas consecutivas
		while not (encontrado):
			strdato=ser.read(1)
			dato = int(hex(ord(strdato)).replace('0x', ''), 16)

			if dato == MARCA:
				if ant_marca:
					encontrado = True
				else:
					ant_marca = True
			else:
				ant_marca = False
        
#Asegurarse de que los datos se corresponden con la configurcion
		cont = 0
		rellenos = 0
		limite = 0
		while (cont!=self.n_lines*TAMDATA+TAMTIME+2 and rellenos == self.n_lines):
			encontrado = False
			cont = 0
			rellenos = 0
			limite += 1

	    #para evitar bucle infinito
			if limite > 100:
				print "La configuracion leida es incorrecta, volver a intentar"
				exit (0)

			while not (encontrado):
				cont +=1
				strdato=ser.read(1)
				dato = int(hex(ord(strdato)).replace('0x', ''), 16)

#comprobar si los rellenos estan correctos
				if (TAMDATA==2):
					if cont % 2 == 1:
						if not (dato & RELLENO == RELLENO):
							rellenos = 0
						else:
							rellenos += 1

				if dato == MARCA:
					if ant_marca:
						encontrado = True
					else:
						ant_marca = True
				else:
					ant_marca = False
        
		vec_datos = [0 for i in range (self.n_lines)]
		cont = 0

		while self.running:
			if (TAMDATA==2):
				strdato = ser.read(1)
				byte1 = int(hex(ord(strdato)).replace('0x', ''), 16)
				strdato = ser.read(1)
				byte2 = int(hex(ord(strdato)).replace('0x', ''), 16)
				dato = byte1 * 4 + (byte2 - RELLENO) / 64
				vec_datos[cont] = float (dato)
				cont+=1
			else:
				strdato = ser.read(1)
				dato = int(hex(ord(strdato)).replace('0x', ''), 16)
				dato = dato * 4
				vec_datos[cont] = float (dato)
				cont+=1

			if cont == (self.n_lines):
				cont = 0
#Saltamos el time
				for i in range (TAMTIME):
					strdato=ser.read(1)

#Comprobamos que los dos proximos datos son marcas
				ant_marca = False;
				encontrado = True;

				strdato=ser.read(1)
				dato = int(hex(ord(strdato)).replace('0x', ''), 16)
				if not(dato==MARCA):
					encontrado = False

				strdato=ser.read(1)
				dato = int(hex(ord(strdato)).replace('0x', ''), 16)
				if not(dato==MARCA):
					encontrado = False
				else:
					ant_marca = True
# Todo correcto, enviar iteracion
				if encontrado:
					for i in range(self.n_lines):
#transformar datos del medidor a vatios, 0.02 es el valor de conversión de los datos de este medidor.
						current[i] = vec_datos[i] * CONST_CONVERSION
						try:
							power[i] = current[i] * self.lines[i].voltage
						except:
							power[i] = current[i] * 12

					yield power

				else:
#Buscar dos marcas consecutivas
					while strdato!='' and not (encontrado):
						strdato=ser.read(1)
						dato = int(hex(ord(strdato)).replace('0x', ''), 16)
						if dato==MARCA:
							if ant_marca:
								encontrado = True
							else:
								ant_marca = True
						else:
							ant_marca = False	
		ser.close()

