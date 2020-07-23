### english below

# RSA- CRT auf dem XMC4700
Bei diesem Quellcode handelt es sich um eine RSA- CRT- Implementierung für den XMC4700, welcher mit der Arduino IDE kompiliert und auf den Microcontroller geladen werden kann.

## Abhöngigkeiten und Vorraussetzungen:
+ Arduino IDE (v1.18)
+ [Infinion Bibliothek](https://github.com/Infineon/XMC-for-Arduino/wiki/Implementation-in-Arduino-IDE)
+ Flint Bibliothek (im Anhang)
+ Anpassung der Stacksize: Windows-Pfad: ```C:\Users\<USER>\Documents\ArduinoData\packages\Infineon\hardware\arm\<VERSION>\variants\XMC4700\linker_skript.ld```
   Zeile 57: ```stack_size = DEFINED(stack_size) ? stack_size : 15000;```


# RSA- CRT on XMC4700 
This source code is an RSA-CRT implementation for the XMC4700, which can be compiled and loaded onto the microcontroller with the Arduino IDE.

## dependencies/ requirements:
+ Arduino IDE (v1.18)
+ Infinion library [https://github.com/Infineon/XMC-for-Arduino/wiki/Implementation-in-Arduino-IDE]
+ Flint library (is attached)
+ increase the stacksize: Windows path: ```C:\Users\<USER>\Documents\ArduinoData\packages\Infineon\hardware\arm\<VERSION>\variants\XMC4700\linker_skript.ld```
   line 57: ```stack_size = DEFINED(stack_size) ? stack_size : 15000;```
