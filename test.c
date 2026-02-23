#include <stdio.h>
int main()
{
    int BUFFER_SIZE = 900;
    int offsetInt = 800 ; 
    int lengthInt = 50;
   //make sure that write wont be larger than buffer
   if((offsetInt + lengthInt) > BUFFER_SIZE)
   {
        //print warning
        printf("write goes out of bounds\n");
        //set length to write to end of file
        lengthInt = BUFFER_SIZE - offsetInt;
        printf("New length %i \n", lengthInt);
        return 0;
   }
   printf("write in bounds\n");
   return 0;
}