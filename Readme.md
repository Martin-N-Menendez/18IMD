## 18 IMD

# Contenido:
1) Código fuente del device driver que hayan desarrollado.

  myeeprom.c
  
2) Código fuente de la aplicación de usuario que lo usa (no es obligatorio que esté escrita en C).

  test.c
  
3) Device tree "custom".

   am335x-customboneblack.dtb
  
4) Makefiles correspondientes para compilar el driver y la aplicación, si es necesario.

  Makefile
  
5) Material utilizado para la presentación en clase.

  Presentacion.ppt
  
6) README explicando cómo se usa el driver.

# Instrucciones:

Compilación del kernel:

1) Hacer las configuraciones necesarias
2) En linux-kernel-labs/src/linux ejecutar:
  export ARCH=arm
  export CROSS_COMPILE=arm-linux-gnueabi-
3) Ejecutar:
  make
4) Se obtiene el zimagen

Compilación del device tree:

1) En ../linux-kernel-labs/src/linux se modifico una copia am335x-boneblack.dtb para habilitar el bus I2C de la placa.
2) Ejecutar:
  make dtbs
3) Se obtiene am335x-customboneblack.dtb (renombrado del anterior)

Carga del device tree y kernel a la BB:

1) En /var/lib/tftpboot/ copiar los archivos am335x-customboneblack.dtb y zimage.
2) Conectar el conversor usb-serie a la BeagleBone
3) En la terminal ejecutar:
  picocom -b 115200 /dev/ttyUSB0 para acceder a la BB
4) Pausar la carga del sistema tocando cualquier tecla, reiniciar la placa si no se llegó
5) Ejecutar en la BB:
  setenv ipadrr 192.168.1.100
  setenv serverip 192.168.1.1
  setenv usbnet_devaddr f8:dc:7a:00:00:02
  
  setenv bootargs root=/dev/nfs rw ip=192.168.1.100 console=ttyO0,115200n8 nfsroot=192.168.1.1:/home/menendez91/linux-kernel-labs/modules/nsfroot,nfsvers=3
 6) Ejecutar en la BB: 
    tftp 0x81000000 zImage
    tftp 0x92000000 am335x-customboneblack.dtb
    bootz 0x81000000 - 0x82000000
    
 7) La BB se reiniciará, el usuario a utilizar es root
 
 Instanciación del módulo:
 
 1) Agregar myeeprom.c,test.c y makefile en ../linux-kernel-labs/modules/nfsroot/root/myeeprom

2)

b
c
