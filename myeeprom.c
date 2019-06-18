#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/i2c.h>
#include <linux/utsname.h>
#include <linux/string.h>
#include <linux/types.h>

#define  DEVICE_NAME "i2c_gonza"    ///< The device will appear at /dev/i2c using this value
#define  CLASS_NAME  "i2c"        ///< The device class -- this is a character device driver

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static char   message[256] = {0};           ///< Memory for the string that is passed from userspace
static short  size_of_message;              ///< Used to remember the size of the string stored
static int    numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class*  i2cClass  = NULL; ///< The device-driver class struct pointer
static struct device* i2cDevice = NULL; ///< The device-driver device struct pointer
static struct i2c_client *modClient;

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
 
void writeByte(struct i2c_client *client, uint8_t reg, uint8_t data);
void Show_data(char* mpu9250_output_buffer,char mode);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/kernel.h>

static const struct i2c_device_id myeeprom_i2c_id[] = {
{ "myeeprom", 0 },
{ }
};
MODULE_DEVICE_TABLE(i2c, myeeprom_i2c_id);
static const struct of_device_id myeeprom_of_match[] = {
    { .compatible = "mse,myeeprom" },
    { }
    };
MODULE_DEVICE_TABLE(of, myeeprom_of_match);

#define SECOND             0x00                
#define MINUTE             0x01                                                                          
#define HOUR               0x02
#define DAY                0x03  
#define DATE               0x04
#define MONTH              0x05
#define YEAR               0x06  
#define ALARM_1_SECONDS    0x07
#define ALARM_1_MINUTES    0x08
#define ALARM_1_HOURS      0x09
#define ALARM_1_DAY_DATE   0x0A
#define ALARM_2_MINUTES    0x0B
#define ALARM_2_HOURS      0x0C
#define ALARM_2_DAY_DATE   0x0D    
#define CONTROL            0x0E
#define STATUS             0x0F
#define AGING_OFFSET       0x10
#define MSB_TEMP           0x11
#define LSB_TEMP           0x12

#define BYTES_LEER           1
#define BYTES_ESCRIBIR       1
#define REG_MAX              18
#define RTC_Addr              0x68
#define RTC_Read              ((RTC_Addr << 1) | 0x01) 
#define RTC_Write             ((RTC_Addr << 1) & 0xFE) 

#define BUFFER_SIZE 20
const char ADDRESS[] = {0x00,0x00};

//**********************************************************************************************

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int ebbchar_init(void){
   pr_info("I2C: Inicializando el driver I2C\n");
 
   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      pr_info("No se pudo asignar un numero mayor a i2c\n");
      return majorNumber;
   }
   pr_info("I2C registrado exitosamente con numero mayor  %d\n", majorNumber);

   // Register the device class
   i2cClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(i2cClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      pr_alert("No fue posible registrar la clase\n");
      return PTR_ERR(i2cClass);          // Correct way to return an error on a pointer
   }
   pr_info("I2C: clase registrada correctamente\n");
 
   // Register the device driver
   i2cDevice = device_create(i2cClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(i2cDevice)){               // Clean up if there is an error
      class_destroy(i2cClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      pr_alert("Error al crear el dispositivo\n");
      return PTR_ERR(i2cDevice);
   }
   
   pr_info("I2C: clase de dispositivo creada correctamente\n"); // Made it! device was initialized
   return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void ebbchar_exit(void){
   device_destroy(i2cClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(i2cClass);                          // unregister the device class
   class_destroy(i2cClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   pr_info("I2C: remocion exitosa.\n");
}


/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
   numberOpens++;
   pr_info("I2C: Se ha abierto el dispositivo %d vece(s)\n", numberOpens);
   return 0;
}
 
/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
   int Ret;
   
    //Mandamos el address a donde queremos leer
    Ret = i2c_master_send(modClient, ADDRESS, 2);
    pr_info("cantidad de bytes a leer = %d",len);
    Ret = i2c_master_recv(modClient, message, len);

   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, message, Ret);
 
   if (error_count==0){            // if true then have success
      pr_info("I2C: Se enviaron %d caracteres al usuario\n", Ret);
      return 0;
   }
   else {
      pr_info("EBBChar: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}
 
/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   int Ret;
   //char buf[BUFFER_SIZE];
   int i;
   char mpu9250_output_buffer[REG_MAX];
   char buf[] = {SECOND};

   copy_from_user(message,buffer, len);
   
   
    if(message[0]=='0')
    {
        pr_info("Finalizando programa ...");
        return 0;
    }

    if(message[0]=='1' || message[0]=='2' || message[0]=='3' || message[0]=='4' || message[0]=='5')
    {
        Ret = i2c_master_send(modClient, buf,1);
        //pr_alert("Valor de retorno de escritura: %d\r\n",Ret);
        i2c_master_recv(modClient,mpu9250_output_buffer,REG_MAX);      
    } 
    else
    {
        pr_alert("Modo no soportado");
        return 0;
    }
                
    Show_data(mpu9250_output_buffer,message[0]);
     
   return len;
}
 
/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
   pr_info("I2C: Dispositivo cerrado exitosamente\n");
   return 0;
}

//**********************************************************************************************

void writeByte(struct i2c_client *client, uint8_t reg, uint8_t data)
{
    char buff[2] = {reg,data};

    i2c_master_send(client, buff, 2);
}

void Show_time(uint8_t hour,uint8_t minute,uint8_t second)
{
    pr_info("%u : %u : %u\n", hour,minute,second);
}

void Show_day(uint8_t day)
{
    switch(day){
            case 1:
                pr_info("Hoy es Lunes\n");
                break;
            case 2:
                pr_info("Hoy es Martes\n");
                break;
            case 3:
                pr_info("Hoy es Miercoles\n");
                break;
            case 4:
                pr_info("Hoy es Jueves\n");
                break;
            case 5:
                pr_info("Hoy es Viernes\n");
                break;
            case 6:
                pr_info("Hoy es Sabado\n");
                break;
            case 7:
                pr_info("Hoy es Domingo\n");
                break;
            default:
                pr_info("Hoy es Osvaldo\n");
                break;
            }
}

void Show_date(uint8_t date,uint8_t month,uint8_t year)
{
    pr_info("Fecha: %u / %u / %u\n", date,month,year);
}

void Show_temperature(uint8_t temperature)
{
    pr_info("Temperatura: %u \n", temperature);
}

void Show_data(char* mpu9250_output_buffer,char mode)
{
    uint8_t year,month,day,date,hour,minute,second,lowT,highT;
    uint8_t aux;

    aux = (uint8_t)mpu9250_output_buffer[0];
    second = ((aux >> 4) * 10) + (aux & 0x0F);

    aux = (uint8_t)mpu9250_output_buffer[1];
    minute = ((aux >> 4) * 10) + (aux & 0x0F);

    aux = (uint8_t)mpu9250_output_buffer[2];
    hour = (((aux & 0x10) >> 4) * 10) + (aux & 0x0F);

    aux = (uint8_t)mpu9250_output_buffer[3];
    day = aux;

    aux = (uint8_t)mpu9250_output_buffer[4];
    date = ((aux >> 4) * 10) + (aux & 0x0F);

    aux = (uint8_t)mpu9250_output_buffer[5];
    month = ((aux & 0x10)>>4)*10+(aux & 0x0F);

    aux = (uint8_t)mpu9250_output_buffer[6];
    year = ((aux >> 4)*10)+ (aux & 0x0F);

    aux = (uint8_t)mpu9250_output_buffer[16];
    lowT = ((aux >> 6) & 0x03)/4;

    aux = (uint8_t)mpu9250_output_buffer[17];
    highT = aux;

    pr_info("############################\r\n");
 
    if(mode == '1' || mode == '2')
        Show_time(hour,minute,second);

    if(mode == '1' || mode == '3')
        Show_day(day);

    if(mode == '1' || mode == '4')
        Show_date(date,month,year);  

    if(mode == '1' || mode == '5')
        Show_temperature(lowT+highT);
        
    pr_info("############################\r\n");
}

static int myeeprom_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int rv;
    char buf[2] = {SECOND,0x00};
    char data;

    int i=0;

    ebbchar_init();
    modClient = client;

    pr_info("Iniciando myeeprom!\n");
    
    writeByte(client,SECOND,0x45);
    writeByte(client,MINUTE,0x25);
    writeByte(client,HOUR,0x19);
    writeByte(client,DATE,0x19);
    writeByte(client,DAY,0x03);
    writeByte(client,MONTH,0x05);
    writeByte(client,YEAR,0x19);
    
    char mpu9250_output_buffer[REG_MAX];

    rv = i2c_master_send(client,buf,1);
    rv = i2c_master_recv(client,mpu9250_output_buffer,REG_MAX);
    /*
    for (i = 0; i < REG_MAX; i++)
    {
        pr_info("REGISTRO=0x%02X --> Valor: 0x%02X\n",buf[0]+i,mpu9250_output_buffer[i]);
    }
    */
    return 0;
}

static int myeeprom_remove(struct i2c_client *client)
{
    uint8_t year,month,day,date,hour,minute,second,lowT,highT;
    int rv;
    uint8_t aux;
  
    pr_info("Removiendo myeeprom\n");

    char buf[]    = {SECOND};
    char mpu9250_output_buffer[REG_MAX];
    int i=0;

    rv = i2c_master_send(client,buf,1);
    rv = i2c_master_recv(client,mpu9250_output_buffer,REG_MAX);

    /*
    for (i = 0; i < REG_MAX; i++)
    {
        pr_info("REGISTRO=0x%02X --> Valor: 0x%02X\n",buf[0]+i,mpu9250_output_buffer[i]);
    }
    */

    aux = (uint8_t)mpu9250_output_buffer[0];
    second = ((aux >> 4) * 10) + (aux & 0x0F);

    aux = (uint8_t)mpu9250_output_buffer[1];
    minute = ((aux >> 4) * 10) + (aux & 0x0F);

    aux = (uint8_t)mpu9250_output_buffer[2];
    hour = (((aux & 0x10) >> 4) * 10) + (aux & 0x0F);

    aux = (uint8_t)mpu9250_output_buffer[3];
    day = aux;

    aux = (uint8_t)mpu9250_output_buffer[4];
    date = ((aux >> 4) * 10) + (aux & 0x0F);

    aux = (uint8_t)mpu9250_output_buffer[5];
    month = ((aux & 0x10)>>4)*10+(aux & 0x0F);

    aux = (uint8_t)mpu9250_output_buffer[6];
    year = ((aux >> 4)*10)+ (aux & 0x0F);

    aux = (uint8_t)mpu9250_output_buffer[16];
    lowT = ((aux >> 6) & 0x03)/4;

    aux = (uint8_t)mpu9250_output_buffer[17];
    highT = aux;

    pr_info("############################\r\n");
 
    pr_info("%u : %u : %u\n", hour,minute,second);

    switch(day){
        case 1:
            pr_info("Lunes\n");
            break;
        case 2:
            pr_info("Martes\n");
            break;
        case 3:
            pr_info("Miercoles\n");
            break;
        case 4:
            pr_info("Jueves\n");
            break;
        case 5:
            pr_info("Viernes\n");
            break;
        case 6:
            pr_info("Sabado\n");
            break;
        case 7:
            pr_info("Domingo\n");
            break;
        default:
            pr_info("Osvaldo\n");
            break;
        }
    pr_info("%u | %u / %u / %u\n", day,date,month,year);

    pr_info("Temperatura: %u \n", lowT+highT);

    pr_info("############################\r\n");

    ebbchar_exit();
    return 0;
}

static struct i2c_driver myeeprom_i2c_driver = {
    .driver = {
        .name = "myeeprom",
        .owner = THIS_MODULE,
        .of_match_table = myeeprom_of_match,
    },
        .probe = myeeprom_probe,
        .remove = myeeprom_remove,
        .id_table = myeeprom_i2c_id
};

module_i2c_driver(myeeprom_i2c_driver);
MODULE_LICENSE("GPL");
