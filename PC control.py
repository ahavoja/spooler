# Hello, Joel here. With this code you can control one crane with your computer's keyboard through ethernet or USB cable. Use arrow keys, and A, Z, L, H, S, U keys.

# libraries you may need to install with pip
import pygame # pip install pygame
import serial # pip install pyserial
import serial.tools.list_ports

# comes with Python 3.8.1
import struct
import threading
import time
import socket
from tkinter import * 

spdBuffer=bytearray(7)
accelBuffer=bytearray(7)
settings=0b10100000
IP="0.0.0.0"
slewAccel,trolAccel,hookAccel=40,20,20
slewSpeed,trolSpeed,hookSpeed=20,10,10
slewSpeedPad,trolSpeedPad,hookSpeedPad=20,10,10
def readSettings():
	global IP,slewAccel,trolAccel,hookAccel,slewSpeed,trolSpeed,hookSpeed,slewSpeedPad,trolSpeedPad,hookSpeedPad
	try:
		with open("settings.txt") as f:
			pass
	except FileNotFoundError:
		try:
			f=open("settings.txt","w")
		except:
			print('Could not create settings.txt file.')
		else:
			f.write('IP_address=192.168.10.21\n\n')
			f.write('#Acceleration in units of steps/(s^2). Range 0 to 16000.\n')
			f.write('accel_slew=500\n')
			f.write('accel_trol=500\n')
			f.write('accel_hook=500\n\n')
			f.write('#Speeds in units of steps/s. Range 0 to 8000.\n')
			f.write('speed_slew=50,100,200,400,800,1600,3200,6400\n')
			f.write('speed_trol=25,50,100,200,400,800,1600,3200\n')
			f.write('speed_hook=25,50,100,200,400,800\n\n')
			f.write('#Speeds for DualShock 4.\n')
			f.write('speed_slew_pad=1000\n')
			f.write('speed_trol_pad=500\n')
			f.write('speed_hook_pad=300')
			print('settings.txt file created.')
		finally:
			f.close()
	try:
		f=open("settings.txt")
	except:
		print('Can not open settings.txt file.')
	else:
		for x in f:
			x=x.strip()
			if x[:11]=="IP_address=":
				IP=x[11:]
			if x[:11]=="accel_slew=":
				slewAccel=int(x[11:])
			if x[:11]=="accel_trol=":
				trolAccel=int(x[11:])
			if x[:11]=="accel_hook=":
				hookAccel=int(x[11:])
			if x[:11]=="speed_slew=":
				slewSpeed=[int(v) for v in x[11:].split(',')]
				slewSpeed.insert(0,0)
			if x[:11]=="speed_trol=":
				trolSpeed=[int(v) for v in x[11:].split(',')]
				trolSpeed.insert(0,0)
			if x[:11]=="speed_hook=":
				hookSpeed=[int(v) for v in x[11:].split(',')]
				hookSpeed.insert(0,0)
			if x[:15]=="speed_slew_pad=":
				slewSpeedPad=int(x[15:])
			if x[:15]=="speed_trol_pad=":
				trolSpeedPad=int(x[15:])
			if x[:15]=="speed_hook_pad=":
				hookSpeedPad=int(x[15:])
	finally:
		f.close()
readSettings()

def packAccels():
	print('Speeds:  {}  {}  {}'.format(slewSpeed,trolSpeed,hookSpeed))
	struct.pack_into('>B',accelBuffer,0,settings|0b1000000)
	struct.pack_into('>BB',accelBuffer,1,(slewAccel&0x3FFF)>>7,slewAccel&0x7F)
	struct.pack_into('>BB',accelBuffer,3,(trolAccel&0x3FFF)>>7,trolAccel&0x7F)
	struct.pack_into('>BB',accelBuffer,5,(hookAccel&0x3FFF)>>7,hookAccel&0x7F)

ser=None
cat=None
say=False
def serStop():
	global ser,cat,say
	if ser is not None:
		ser.close()
	ser=None
	cat=None
	say=False
def mode1():
	pass
def mode2():
	global slew,trol,hook
	slew,trol,hook=0,0,0
def mode3():
	pass

ikkuna=Tk()
ikkuna.title("Asetukset")
#ikkuna.geometry('250x120')
valikko=Menu(ikkuna)
#valinta=Menu(valikko,tearoff=0)
#valikko.add_cascade(label='Profiili',menu=valinta)
#valinta.add_command(label='Hidas')
#valinta.add_command(label='Nopea')
#valinta.add_command(label='Turbo')
ikkuna.config(menu=valikko)
Label(ikkuna,text='Mode:').grid(column=0,row=0)
mode=IntVar()
mode.set(1)
Radiobutton(ikkuna,text='Max speed',value=1,variable=mode,command=mode1).grid(column=1,row=0)
Radiobutton(ikkuna,text='Adjustable',value=2,variable=mode,command=mode2).grid(column=2,row=0)
Radiobutton(ikkuna,text='DualShock',value=3,variable=mode,command=mode3).grid(column=3,row=0)
Label(ikkuna,text='Output:').grid(column=0,row=1)
output=IntVar()
output.set(1)
Radiobutton(ikkuna,text='Off',value=1,variable=output).grid(column=1,row=1)
USBbutton=Radiobutton(ikkuna,text='USB',value=2,variable=output)
USBbutton.grid(column=2,row=1)
Radiobutton(ikkuna,text=IP,value=3,variable=output).grid(column=3,row=1)
IPlabel=Label(ikkuna)
IPlabel.grid(columnspan=4)

def monitorUSB(fan): # prints whatever arduino sends us
	while True:
		try:
			print('Crane: '+fan.readline().rstrip().decode())
		except:
			break

def monitorTCP():
	try:
		sockPos.connect((IP,10001))
	except:
		print("Failed to connect to IP {} port 10001.".format(IP))
		#output.set(1)
	else:
		print("Connected to IP {} port 10001.".format(IP))
		while sockConnected:
			sockPos.sendall(bytes(1))
			data=sockPos.recv(32)
			print('Crane: '+data.rstrip().decode())
			time.sleep(1)
	finally:
		sockPos.close()
		print("Disconnected from port 10001.")

sockSpd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sockSpd.settimeout(2)
sockConnected=False
sockPos = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sockPos.settimeout(2)

# Define some colors
BLACK=(0,0,0)
WHITE=(255,255,255)

# This is a simple class that will help us print to the screen
class TextPrint:
	def __init__(self):
		self.reset()
		self.font = pygame.font.SysFont('Consolas', 14, bold=False, italic=False)
	def print(self, screen, textString):
		textBitmap = self.font.render(textString, True, WHITE)
		screen.blit(textBitmap, [self.x, self.y])
		self.y += self.line_height
	def reset(self):
		self.x = 10
		self.y = 10
		self.line_height = 20
	def indent(self):
		self.x += 20
	def unindent(self):
		self.x -= 20

pygame.init()
screen = pygame.display.set_mode([300, 500]) # screen size [width,height]
pygame.display.set_caption("keyboard")
clock = pygame.time.Clock() # Used to manage how fast the screen updates
pygame.joystick.init()
textPrint = TextPrint() # Get ready to print
slew=trol=hook=slewGear=trolGear=hookGear=old=wax=0
padMode=False

def deadzone(wolf): # calculates deadzones for DualShock4
	zone=0.1
	if wolf>=zone:
		return (wolf-zone)/(1-zone)
	elif wolf<=-zone:
		return (wolf+zone)/(1-zone)
	else: # we are in deadzone
		return 0 # means don't move

padState={0:{},1:{},2:{},3:{}} # nested dictionary to hold button states of each pad
def btnPress(btn): # returns 1 when button becomes pressed
	global padState
	newBtn=pad.get_button(btn)
	try: # check if key is created yet
		oldBtn=padState[i][btn]
	except:
		return 0
	finally:
		padState[i][btn]=newBtn
	if newBtn>oldBtn:
		return 1
	return 0

# -------- Main Program Loop -----------
done = False #Loop until the user clicks the close button.
while done==False:
	myIP=socket.gethostbyname(socket.gethostname())
	IPlabel.config(text='IP of this computer is '+myIP)
	ikkuna.update()

	# DRAWING STEP
	# First, clear the screen to white. Don't put other drawing commands
	# above this, or they will be erased with this command.
	screen.fill(BLACK)
	textPrint.reset()
	joystick_count = pygame.joystick.get_count()
	textPrint.print(screen,"Number of joysticks: {}".format(joystick_count))
	send=0
	stopping=False
	if mode.get()!=2:
		slew=trol=hook=0

	# For each pad:
	for i in range(joystick_count):
		pad = pygame.joystick.Joystick(i)
		pad.init()
		textPrint.print(screen, "Joystick {} : {}".format(i,pad.get_name()) )
		textPrint.indent()
		
		# Usually axis run in pairs, up/down for one, and left/right for
		# the other.
		axes = pad.get_numaxes()
		textPrint.print(screen, "Number of axes: {}".format(axes) )
		textPrint.indent()
		
		show=False
		for j in range( axes ):
			if pad.get_axis(j) != 0:
				show=True
		if show:
			for j in range( axes ):
				textPrint.print(screen, "Axis {} : {:>6.3f}".format(j, pad.get_axis(j)) )
		textPrint.unindent()
				
		buttons = pad.get_numbuttons()
		textPrint.print(screen, "Number of buttons: {}".format(buttons) )
		textPrint.indent()

		for j in range( buttons ):
			button = pad.get_button( j )
			if button == 1:
				textPrint.print(screen, "Button {:>2}".format(j) )
		textPrint.unindent()
		textPrint.unindent()

		if btnPress(1): # silent mode
			print('Pad silent.')
			settings ^= 0b1000
		if btnPress(2): # lights
			settings ^= 0b100
			print('Pad lights {}.'.format('on' if settings&0b100 else 'off'))
		if btnPress(8): # home
			print('Pad homing.')
			settings |= 0b10000		
		if btnPress(9): # switch joystick modes
			print('Pad mode {}.'.format(padMode))
			padMode = not padMode
		if pad.get_button(13): # stop motors
			settings &= ~0b100000
			slew=trol=hook=slew0=trol0=hook0=0
			stopping=True
		elif stopping==False and mode.get()==3:
			faster=False
			if pad.get_button(4) or pad.get_button(5):
				faster=True
			trol0=int(deadzone(pad.get_axis(1))*(trolSpeed[-1] if faster else trolSpeedPad))
			if padMode:
				slew0=int(-deadzone(pad.get_axis(0))*(slewSpeed[-1] if faster else slewSpeedPad))
				hook0=int(deadzone(pad.get_axis(3))*(hookSpeed[-1] if faster else hookSpeedPad))
			else:
				slew0=int((pad.get_axis(5)-pad.get_axis(4))*((slewSpeed[-1] if faster else slewSpeedPad)/2+0.01))
				hook0=int(-deadzone(pad.get_axis(3))*(hookSpeed[-1] if faster else hookSpeedPad))
			if slew0!=0:
				slew=slew0
			if trol0!=0:
				trol=trol0
			if hook0!=0:
				hook=hook0

	# EVENT PROCESSING STEP
	for event in pygame.event.get(): # User did something
		if event.type == pygame.QUIT: # If user clicked close
			done=True # Flag that we are done so we exit this loop
		if event.type == pygame.KEYDOWN: # single key presses
			if event.key == pygame.K_s:
				settings ^= 0b1000
			if event.key == pygame.K_h:
				settings |= 0b10000 # home
			if event.key == pygame.K_l:
				settings ^= 0b100 # lights on/off
			if event.key == pygame.K_u:
				send=1
			if event.key == pygame.K_SPACE:
				settings &= ~0b100000 # stop
				slewGear=trolGear=hookGear=0
			if mode.get()==2:
				limit=len(slewSpeed)-1
				if event.key == pygame.K_LEFT and slewGear<limit:
					slewGear+=1
				if event.key == pygame.K_RIGHT and slewGear>-limit:
					slewGear-=1
				limit=len(trolSpeed)-1
				if event.key == pygame.K_UP and trolGear<limit:
					trolGear+=1
				if event.key == pygame.K_DOWN and trolGear>-limit:
					trolGear-=1
				limit=len(hookSpeed)-1
				if event.key == pygame.K_a and hookGear<limit:
					hookGear+=1
				if event.key == pygame.K_z and hookGear>-limit:
					hookGear-=1
				slew=slewSpeed[abs(slewGear)]
				trol=trolSpeed[abs(trolGear)]
				hook=hookSpeed[abs(hookGear)]
				if slewGear<0:
					slew=-slew
				if trolGear<0:
					trol=-trol
				if hookGear<0:
					hook=-hook

	if mode.get()!=2: # press and hold keyboard control
		keys=pygame.key.get_pressed()
		if keys[pygame.K_SPACE]:
			slew=trol=hook=0
		else:
			if keys[pygame.K_LEFT]:
				slew=slewSpeed[-1]
			elif keys[pygame.K_RIGHT]:
				slew=-slewSpeed[-1]
			if keys[pygame.K_UP]:
				trol=trolSpeed[-1]
			elif keys[pygame.K_DOWN]:
				trol=-trolSpeed[-1]
			if keys[pygame.K_a]:
				hook=hookSpeed[-1]
			elif keys[pygame.K_z]:
				hook=-hookSpeed[-1]
	struct.pack_into('>B',spdBuffer,0,settings)
	struct.pack_into('>BB',spdBuffer,1,(slew&0x3FFF)>>7,slew&0x7F)
	struct.pack_into('>BB',spdBuffer,3,(trol&0x3FFF)>>7,trol&0x7F)
	struct.pack_into('>BB',spdBuffer,5,(hook&0x3FFF)>>7,hook&0x7F)

	textPrint.print(screen,"{} {} {}".format(slew,trol,hook))

	if output.get()==2: # send via USB
		if ser is None: # auto select arduino COM port
			if cat is None:
				now=time.time()
				if now-old > 1 : # reduces CPU usage
					old=now
					for dog in serial.tools.list_ports.comports():
						print(dog)
						cat=dog.device
					if say is False:
						say=True
						if cat is None:
							print('Kytke Arduino USB johto.')
			else:
				try:
					ser = serial.Serial(cat,250000) # port, baud rate
				except:
					pass
				else:
					threading.Thread(target=monitorUSB, args=(ser,)).start()
		else: # send data to arduino
			try:
				ser.write(spdBuffer) # send speeds
			except:
				serStop()
			else:
				settings |= 0b100000 # stop stopping
				settings &= ~0b10000 # stop homing
				textPrint.print(screen,"USB on")
			if send:
				readSettings()
				packAccels()
				try:
					ser.write(accelBuffer) # sometimes send accelerations
				except:
					serStop()
				else:
					send=0
					print('Accelerations sent.')
	else:
		serStop()
	if output.get()==3: # send via TCP
		if not sockConnected:
			readSettings()
			try:
				sockSpd.connect((IP,10000))
			except:
				print("Failed to connect to IP {} port 10000.".format(IP))
				output.set(1)
			else:
				print("Connected to IP {} port 10000.".format(IP))
				sockConnected=True
				threading.Thread(target=monitorTCP).start()
		else:
			try:
				sockSpd.sendall(spdBuffer) # send speeds to Arduino
			except:
				print("Could not send speeds. Disconnected from port 10000.")
				sockSpd.close()
				sockConnected=False
				output.set(1)
			else:
				settings |= 0b100000 # stop stopping
				settings &= ~0b10000 # stop homing
				textPrint.print(screen,"TCP on")
			if send:
				readSettings()
				packAccels()
				try:
					sockSpd.sendall(accelBuffer) # sometimes send accelerations
				except:
					print("Could not send accelerations. Disconnected from port 10000.")
					sockSpd.close()
					sockConnected=False
					output.set(1)
				else:
					send=0
					print('Accelerations sent.')
	elif sockConnected:
		print("Disconnected from port 10000.")
		sockSpd.close()
		sockConnected=False

	if settings&0b1000:
		textPrint.print(screen,'silent mode')
	if settings&0b100:
		textPrint.print(screen,'lights on')
	pygame.display.flip() # update numbers on pygame window
	clock.tick(20) # Limit to 20 frames per second

sockSpd.close()
serStop()
pygame.quit()