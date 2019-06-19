## 19-Implementacion de manejadores de dispositivos (RTC DS3231)

https://datasheets.maximintegrated.com/en/ds/DS3231.pdf

# Logros:

- [x] Escritura de hora/fecha/dia al instanciar.
- [x] Lectura de hora/fecha/dia/temperatura al instanciar/remover o por medio del usuario.
- [x] Correcta conversión de los registros en valores numéricos coherentes.
- [x] Programa de usuario que permite elegir que valor leer.
- [x] Modularización de las funciones para separar el parseo de la presentación de los datos.
- [ ] \(Opcional) Seteo de la hora por parte del usuario.
- [ ] \(Opcional) Seteo de alarmas.
- [ ] \(Opcional) Seteo de AM/PM.

# Contenido del repositorio:
1) Código fuente del device driver que hayan desarrollado.

 > myeeprom.c
  
2) Código fuente de la aplicación de usuario que lo usa (no es obligatorio que esté escrita en C).

 > test.c
  
3) Device tree "custom".

 >  am335x-customboneblack.dtb
  
4) Makefiles correspondientes para compilar el driver y la aplicación, si es necesario.

>  Makefile
  
5) Material utilizado para la presentación en clase.

>  https://docs.google.com/presentation/d/1MA5sRsCMgNUqkUWAGHTY9cbsq7G2MxMdyRVlW63p07M/edit#slide=id.g35ed75ccf_022
  
6) README explicando cómo se usa el driver.

# Instrucciones:

Compilación del kernel:

1) Hacer las configuraciones necesarias
2) En linux-kernel-labs/src/linux ejecutar:
```
  export ARCH=arm
  
  export CROSS_COMPILE=arm-linux-gnueabi-
```  
3) Ejecutar:
```
  make
```  
4) Se obtiene el zimagen

Compilación del device tree:

1) En ../linux-kernel-labs/src/linux/arch/arm/boot/dts/ se modifico una copia am335x-boneblack.dtb para habilitar el bus I2C de la placa.
2) Ejecutar:
```
  make dtbs
```  
3) Se obtiene am335x-customboneblack.dtb (renombrado del anterior)

Carga del device tree y kernel a la BB:

1) En /var/lib/tftpboot/ copiar los archivos am335x-customboneblack.dtb y zimage.
2) Conectar el conversor usb-serie a la BeagleBone
3) En la terminal ejecutar:
```
  picocom -b 115200 /dev/ttyUSB0 para acceder a la BB
```  
4) Pausar la carga del sistema tocando cualquier tecla, reiniciar la placa si no se llegó
5) Ejecutar en la BB:
```
  setenv ipadrr 192.168.1.100
  
  setenv serverip 192.168.1.1
  
  setenv usbnet_devaddr f8:dc:7a:00:00:02
  
  setenv bootargs root=/dev/nfs rw ip=192.168.1.100 console=ttyO0,115200n8 nfsroot=192.168.1.1:/home/menendez91/linux-kernel-labs/modules/nsfroot,nfsvers=3
```  
 6) Ejecutar en la BB: 
 ```
    tftp 0x81000000 zImage
    
    tftp 0x92000000 am335x-customboneblack.dtb
    
    bootz 0x81000000 - 0x82000000
 ```   
 7) La BB se reiniciará, el usuario a utilizar es root
 
 Instanciación del módulo:
 
 1) Agregar myeeprom.c,test.c y makefile en ../linux-kernel-labs/modules/nfsroot/root/myeeprom
 2) Ejecutar:
 ```
    make
 ```
 3) En la BB ejecutar:
 ```
    cd myeeprom/
    insmod myeeprom.ko
 ```  
 4) Ir a /dev y ejecutar ls para comprobar que el dispositivo se encuentra correctamente instanciado
 5) volver a myeeprom/
 
 Compilación y ejecución del programa
 
 1) Para compilar se deberá estar en la terminal en la carpeta ../linux-kernel-labs/modules/nfsroot/root/myeeprom y ejecutar:
 ```
 arm-linux-gnueabi-gcc -static test.c -o test.e
 ```
 2) Se obtiene test.e como ejecutable
 3) En la BB dentro de la carpeta myeeprom ejecutar:
 ```
    ./test.e
 ```   
 4) Se tienen 5 modos de operacion en pantalla:
 ```
    1 > Ver todo -> Equivalente a ejecutar todos los otros modos de operación a la vez
    2 > Ver hora -> Se le pedirá al RTC la hora en formato HH:MM:SS
    3 > Ver dia de la semana -> Se el pedirá al RTC que indique que día es hoy
    4 > Ver fecha -> Se le pedirá al RTC que indique la fecha en formato DD/MM/AA
    5 > Ver temperatura -> Se le pedirá al RTC que indique la temperatura en celsius
    0 > Salida -> Fin del programa
  ```  
  Cualquier otro modo pedido será inválido y se repreguntará las veces que sean necesarias, hasta que se ingrese el modo 0 para salir.
  
  5) Al finalizar remover el modulo ejecutando:
   ``` 
    rmmod myeeprom.ko
  ```

# Ejemplo de resultados:
 ```
# insmod myeeprom.ko
[12555.695613] I2C: Inicializando el driver I2C
[12555.700236] I2C registrado exitosamente con numero mayor  246
[12555.706094] I2C: clase registrada correctamente
[12555.712979] I2C: clase de dispositivo creada correctamente
[12555.718805] Iniciando myeeprom!
# ./test.e
Abriendo el dispositivo.[12559.474038] I2C: Se ha abierto el dispositivo 1 vece(s)

Ingrese el modo de operacion:
1 > Ver todo
2 > Ver hora
3 > Ver dia de la semana
4 > Ver fecha
5 > Ver temperatura
0 > Salida
5
[12561.325677] ############################
[12561.329833] Temperatura: 24 
[12561.332737] ############################
Ingrese el modo de operacion:
1 > Ver todo
2 > Ver hora
3 > Ver dia de la semana
4 > Ver fecha
5 > Ver temperatura
0 > Salida
4
[12562.524515] ############################
[12562.528666] Fecha: 19 / 5 / 19
[12562.531745] ############################
Ingrese el modo de operacion:
1 > Ver todo
2 > Ver hora
3 > Ver dia de la semana
4 > Ver fecha
5 > Ver temperatura
0 > Salida
3
[12563.620142] ############################
[12563.624205] Hoy es Miercoles
[12563.627228] ############################
Ingrese el modo de operacion:
1 > Ver todo
2 > Ver hora
3 > Ver dia de la semana
4 > Ver fecha
5 > Ver temperatura
0 > Salida
2
[12564.547655] ############################
[12564.551720] 19 : 25 : 53
[12564.554273] ############################
Ingrese el modo de operacion:
1 > Ver todo
2 > Ver hora
3 > Ver dia de la semana
4 > Ver fecha
5 > Ver temperatura
0 > Salida
1
[12565.644074] ############################
[12565.648222] 19 : 25 : 54
[12565.650779] Hoy es Miercoles
[12565.653681] Fecha: 19 / 5 / 19
[12565.656756] Temperatura: 24 
[12565.659742] ############################
Ingrese el modo de operacion:
1 > Ver todo
2 > Ver hora
3 > Ver dia de la semana
4 > Ver fecha
5 > Ver temperatura
0 > Salida
0
Fin del programa
[12568.035665] I2C: Dispositivo cerrado exitosamente
# rmmod myeeprom.ko
[12572.238230] Removiendo myeeprom
[12572.242408] ############################
[12572.246460] 19 : 26 : 1
[12572.249114] Miercoles
[12572.251413] 3 | 19 / 5 / 19
[12572.254226] Temperatura: 24 
[12572.257204] ############################
[12572.263441] I2C: remocion exitosa.
 ```



