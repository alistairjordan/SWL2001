#include <stdio.h>
#include "smtc_hal_eeprom.h"

void main() {
    char test1[] = "HELLO WORLD 1";
    char test2[] = "GOODBYE WORLD 1";
    char test3[] = "HELLO WORLD 2";
    char test4[] = "GOODBYE WORLD 2";
    char ret_data[100];

    hal_eeprom_write_buffer(0, test1, sizeof(test1));
    hal_eeprom_write_buffer(100, test2, sizeof(test2));
    hal_eeprom_write_buffer(300, test3, sizeof(test3));
    hal_eeprom_write_buffer(400, test4, sizeof(test4));

    hal_eeprom_read_buffer( 0, ret_data, 25 );
    printf("TEST1: %s\n",ret_data);

    hal_eeprom_read_buffer( 100, ret_data, 25 );
    printf("TEST2: %s\n",ret_data);

    hal_eeprom_read_buffer( 300, ret_data, 25 );
    printf("TEST3: %s\n",ret_data);

    hal_eeprom_read_buffer( 400, ret_data, 25 );
    printf("TEST4: %s\n",ret_data);

}