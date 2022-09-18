from cmath import cos, sin                                                      #import sin and cos for distance
from turtle import color                                                        #import color for styling !
import serial                                                                   #import serial for UART communication
import matplotlib.pyplot as plt                                                 #import matplotlib for 3d visualization
from mpl_toolkits.mplot3d import Axes3D                                         #import Axes3D for axes manipulation

count = 0                                                                       #general counter that will be used in many occasions
                
if(input("Erase previous data?(Y for yes/N for no)") == 'Y'): open('plotpoints.txt', 'w').close()                                     
                                                                        #if "Y" (Yes) is the request open in write mode to change entire file
s = serial.Serial('COM4', baudrate = 115200, timeout = None)                    #begin to receive data from uC
                                                                                #timeout 0 to wait until bytes come
s.reset_output_buffer()                                                         #need each time program starts
s.reset_input_buffer()                                                          #need eachtime program starts

graphsize = plt.figure(figsize=(12,12))                                         #initialize graph
cartesian = graphsize.add_subplot(111, projection='3d')                         #make it 3D
cartesian.set_xlabel('x');cartesian.set_ylabel('y');cartesian.set_zlabel('z')   #names of axes

s.write('s'.encode())                                                           #flag to start getting data
  
f = open('plotpoints.txt','a')                                                   #append so previous data not deleted
while (count != 64): print(count+1);f.write(str(float(s.readline().decode())));f.write('\n');count+=1#readline
                                                                                #put in text file
                                                                                #new line each iteration
                                                                                #increment
f.close(); s.close()                                                            #close text file and UART communication

data = open("plotpoints.txt","r")                                                #open text file in read only mode
dataStr = data.read().split("\n")                                               #seperated by spaces
data.close()                                                                    #close file

dataStr = [str(i) for i in dataStr]                                             #to string
dataStr.pop(len(dataStr)-1)                                                     #last "\n" removed
Data = [float(i) for i in dataStr]                                              #turned into type float

count = 0;x = 0                                                                 #variables 
while(count!=len(Data)):                                                        #while theres data left to graph
    if count >=64 and count %64==0:x+=1                                         #increment for every 64 data

    cartesian.scatter(x,(Data[count])*sin(count*0.09817477),(Data[count])*cos(count*0.09817477),c="blue",s=1)
                                                                                #plot points in cartesian

    cartesian.plot([0,0],[(Data[count-1])*sin((count-1)*0.09817477),(Data[count])*sin(count*0.09817477)],[(Data[count-1])*cos((count-1)*0.09817477),(Data[count])*cos(count*0.09817477)] , color = 'blue')#blue line segments 
                                                                                #connect points
    count+=1                                                                    #increment
    
count=0;x = 0
while(count < len(Data)-64):                                                    #while (number of rings-1)
    if count >=64 and count %64==0:x+=0.1                                       #for every 64 measurements
    cartesian.plot([x,x+0.1],[(Data[count])*sin((count)*0.09817477),(Data[count+64])*sin(count*0.09817477)],[(Data[count])*cos((count)*0.09817477),(Data[count+64])*cos(count*0.09817477)],color='blue')#blue line segments
                                                                                #plot the lines that connect the different poitns of each ring
    count+=1                                                                    #increment
plt.show()                                          #show graph