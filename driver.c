#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

MODULE_AUTHOR("me");
MODULE_LICENSE("GPL");
/*
 TODO:
 define device buffer size, device name, etc. and other global data structures
 you will need here.
 */
#define BUFFER_SIZE 900
void *memory=NULL;//this is where we will write to
const char * name = "My Driver";
int timesOpened = 0;
int timesClosed = 0;
int bufferMax = 9;
int currOffset = 0;//track the offset at the end of every read write

ssize_t char_driver_read(struct file *pfile, char __user *buffer, size_t length,
                         loff_t *offset) {
  /*
   TODO:
   buffer is the userspace buffer to where you are writing the data you
   want to read from the device file

   length is the length of the userspace buffer

   offset will be set to current position of the opened file after reading
   */
    
   printk(KERN_INFO "Entering read\n");
    int offsetInt = *offset;
    int lengthInt = (int) length; 
    printk(KERN_INFO "offsetInt: %i \n", offsetInt);
    printk(KERN_INFO "length Int: %i \n", lengthInt);
    //make sure length isnt more than our kernel buffer
    if((offsetInt + lengthInt) > bufferMax)
    {
        //print warning
        printk(KERN_INFO "Size is to big. reading as much as is possible\n");
        //set length to something that we can fit
        lengthInt = bufferMax - offsetInt;
        printk(KERN_INFO "New length Int: %i \n", lengthInt);
    }
    //copy to user
    int failCheck = copy_to_user(buffer, memory + offsetInt, lengthInt);
    if(failCheck != 0)
    {
        printk(KERN_INFO "Read failed\n");
        return -1;
    }
    else
    {
        printk(KERN_INFO "Read success\n");
    }
    //update offset by adding bytes written to offset
    offsetInt = offsetInt + lengthInt;
    *offset = offsetInt;
    //update current offset
    currOffset = *offset;
    return lengthInt;
}

ssize_t char_driver_write(struct file *pfile, const char __user *buffer,
                          size_t length, loff_t *offset) {
  /*
   TODO:
   buffer is the userspace buffer where you are writing the data you want to be
   written into the device file
    
   length is the length of the userspace buffer

   offset will be set to current position of the opened file
   */
  printk(KERN_INFO "Entering write\n");
   //dereference offset
    int offsetInt = *offset; 
    int lengthInt = (int) length;
    printk(KERN_INFO "offsetInt: %i \n", offsetInt);
    printk(KERN_INFO "length Int: %i \n", lengthInt);
   //make sure that write wont be larger than buffer
    if((offsetInt + lengthInt) > bufferMax)
    {
        //print warning
        printk(KERN_INFO "Size is to big. Wrtining as much as is possible\n");
        //set length to something that we can fit
        lengthInt = bufferMax - offsetInt;
    }
    //use copy to user to write to buffer. Write from the offsetInt
    int bytesWritten = copy_from_user(memory + offsetInt, buffer, lengthInt);
    //make sure that write was succesful
    if(bytesWritten != 0)//copy_from_user returns 0 on a success
    {
        printk(KERN_INFO "Write failed\n");
        return -1;
    }
    else
    {
        printk(KERN_INFO "Write success\n");
    }
    //update offset by adding bytes written to offset
    offsetInt = offsetInt + lengthInt;
    *offset = offsetInt;
    //update current offset
    currOffset = *offset;
    printk(KERN_INFO "Length copied %i \n", lengthInt);
    return length;
}

int char_driver_open(struct inode *pinode, struct file *pfile) {
  /*
   TODO:
   print to the kernel log that the device has been opened, and also print the
   number of times the device has been opened until now.
   */
    printk(KERN_INFO "Entering open\n");
    //increment times opened
    timesOpened ++;
    printk(KERN_INFO "Device opened. Device has been opened %i times\n", timesOpened);
    return 0;
}

int char_driver_close(struct inode *pinode, struct file *pfile) {
  /*
   TODO:
   print to the kernel log that the device has been closed, and also print the
   number of times the device has been closed until now.
   */
    printk(KERN_INFO "Entering close\n");
    timesClosed++;
    printk(KERN_INFO "Device closed Device has been closed %i times\n", timesClosed);
    return 0;
}



loff_t char_driver_seek(struct file *pfile, loff_t offset, int whence) {
  /*
   TODO:
   update the opened file's position according to the values of offset and
   whence.
   */
    printk(KERN_ALERT "seeking");
    if(whence == 0) //0 means set offset to begining of file
    {
        currOffset = 0;
        return 0;
    }
    else if(whence == 1)
    {
        currOffset = (int)offset;
        return currOffset;
    }
    else
    {
        //make sure offset isnt off buffer
        if((currOffset + (int)offset) <= bufferMax && (currOffset + (int)offset) >= 0 )//if offset is less than max and greater than zero
        {
            currOffset = currOffset + (int)offset;
            return currOffset;
        }
    }
    return 0;
}

struct file_operations char_driver_file_operations = {
    //.owner = THIS_MODULE,
    /*
     TODO:
     add the function pointers to point to corresponding file operations. have a
     look at the linux/include/fs.h file for examples
     */
    .owner = THIS_MODULE,
    .open = char_driver_open,
    .release = char_driver_close,
    .read = char_driver_read,
    .write = char_driver_write,
    .llseek = char_driver_seek,
};


static int char_driver_init(void) {
  /*
   TODO:
   register the device and print to the kernel log that the init function has
   been called.
   */
    
    if(register_chrdev(511, name, &char_driver_file_operations) >= 0)//last argument expects a pointer so we pass memory address. 0 returned on success
    {
        printk(KERN_INFO "register success\n");
    }
    else
    {
        printk(KERN_INFO "register fail\n"); 
        //unregister_chrdev(511, "My Driver");
        return 1;
    }
    //allocate memory
    if((memory=kmalloc(BUFFER_SIZE, GFP_KERNEL)) == NULL)
    {
        printk(KERN_INFO "kmalloc failed\n");
        return 1;
    }
    printk(KERN_INFO "init function called\n");
    //zero times opened and closed
    //timesOpened = 0;
    //timesClosed = 0;
    printk(KERN_INFO "About to return\n");
    return 0;
}

static void char_driver_exit(void) {
  /*
   TODO:
   unregister the device using the register_chrdev() function, and print to the
   kernel log that the exit function has been called.
   */
    unregister_chrdev(511, "My Driver");
    kfree(memory);
    printk(KERN_INFO "exit function called\n");
    //zero times opened and closed
    timesOpened = 0;
    timesClosed = 0;
}

/*
 TODO:
 add module_init() and module_exit() functions that point to the corresponding
 init and exit functions of the driver.
*/




module_init(char_driver_init);
module_exit(char_driver_exit);