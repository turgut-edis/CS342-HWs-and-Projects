#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "fat.h"

#define SECTOR_SIZE 512
#define CLUSTER_SIZE 1024

//convert all chars to upper case
void to_upper(char* string)
{
    const char OFFSET = 'a' - 'A';
    while (*string)
    {
        *string = (*string >= 'a' && *string <= 'z') ? *string -= OFFSET : *string;
        string++;
    }
}
// Append the char into given string
void append(char *s, char c)
{
    int len = strlen(s);
    s[len] = c;
    s[len + 1] = '\0';
}

// Conversion of hexadecimal to binary for only date value
char *hex_binary(char *hex)
{

    char *bin = malloc(32 * sizeof(char));
    int i = 0;

    /* Extract first digit and find binary of each hex digit */
    for (i = 0; hex[i] != '\0'; i++)
    {
        switch (hex[i])
        {
        case '0':
            strcat(bin, "0000");
            break;
        case '1':
            strcat(bin, "0001");
            break;
        case '2':
            strcat(bin, "0010");
            break;
        case '3':
            strcat(bin, "0011");
            break;
        case '4':
            strcat(bin, "0100");
            break;
        case '5':
            strcat(bin, "0101");
            break;
        case '6':
            strcat(bin, "0110");
            break;
        case '7':
            strcat(bin, "0111");
            break;
        case '8':
            strcat(bin, "1000");
            break;
        case '9':
            strcat(bin, "1001");
            break;
        case 'a':
        case 'A':
            strcat(bin, "1010");
            break;
        case 'b':
        case 'B':
            strcat(bin, "1011");
            break;
        case 'c':
        case 'C':
            strcat(bin, "1100");
            break;
        case 'd':
        case 'D':
            strcat(bin, "1101");
            break;
        case 'e':
        case 'E':
            strcat(bin, "1110");
            break;
        case 'f':
        case 'F':
            strcat(bin, "1111");
            break;
        default:
            printf("Invalid hexadecimal input.");
        }
    }

    return bin;
}

// Get the date specific values : year, month, day, hour and minute
unsigned int get_year(char *data)
{
    char *bin_year = malloc(sizeof(char) * 7);
    strncpy(bin_year, data, 7);
    unsigned int year = 0;
    for (int i = 0; i < 32; i++)
    {
        if (bin_year[i] == '1')
        {
            year = year + pow(2, 6 - i);
        }
    }
    year = year + 1980;
    free(bin_year);
    return year;
}

unsigned int get_month(char *data)
{
    char *bin_month = malloc(sizeof(char) * 4);
    strncpy(bin_month, &(data[7]), 4);
    unsigned int month = 0;
    for (int i = 0; i < 32; i++)
    {
        if (bin_month[i] == '1')
        {
            month = month + pow(2, 3 - i);
        }
    }
    free(bin_month);
    return month;
}

unsigned int get_day(char *data)
{
    char *bin_day = malloc(sizeof(char) * 5);
    strncpy(bin_day, &(data[11]), 5);
    unsigned int day;
    for (int i = 0; i < 32; i++)
    {
        if (bin_day[i] == '1')
        {
            day = day + pow(2, 4 - i);
        }
    }
    free(bin_day);
    return day;
}

unsigned int get_hour(char *data)
{
    char *bin_hour = malloc(sizeof(char) * 5);
    strncpy(bin_hour, &(data[16]), 5);
    unsigned int hour = 0;
    for (int i = 0; i < 32; i++)
    {
        if (bin_hour[i] == '1')
        {
            hour = hour + pow(2, 4 - i);
        }
    }
    free(bin_hour);
    return hour;
}

unsigned int get_minute(char *data)
{
    char *bin_minute = malloc(sizeof(char) * 6);
    strncpy(bin_minute, &(data[21]), 6);
    unsigned int minute = 0;
    for (int i = 0; i < 32; i++)
    {
        if (bin_minute[i] == '1')
        {
            minute = minute + pow(2, 5 - i);
        }
    }
    free(bin_minute);
    return minute;
}

// This prints the formatted date and time for -d
void get_date_formatted(unsigned char *buf, int idx)
{
    unsigned int a;
    char *j;

    // print date hex values in 4 bytes
    // 1- convert added hex to big endian
    // 2- convert to decimal
    // 3- convert to date by parsing

    memcpy(&a, &(buf[idx]), sizeof(int));
    char aa[20];

    sprintf(aa, "%x", a);
    j = hex_binary(aa);

    unsigned int year = get_year(j);
    unsigned int month = get_month(j);
    unsigned int day = get_day(j);
    unsigned int hour = get_hour(j);
    unsigned int minute = get_minute(j);

    printf("date\t     = %d-%d-%d\ntime\t     = %d:%d \n", day, month, year, hour, minute);
    free(j);
}

// This prints the formatted date and time for -l
void print_dates(unsigned char *buf, int idx)
{
    unsigned int a;
    char *j;

    // print date hex values in 4 bytes
    // 1- convert added hex to big endian
    // 2- convert to decimal
    // 3- convert to date by parsing

    memcpy(&a, &(buf[idx]), sizeof(int));
    char aa[20];

    sprintf(aa, "%x", a);
    j = hex_binary(aa);

    unsigned int year = get_year(j);
    unsigned int month = get_month(j);
    unsigned int day = get_day(j);
    unsigned int hour = get_hour(j);
    unsigned int minute = get_minute(j);

    printf("date=%d-%d-%d:%d:%d \n", day, month, year, hour, minute);
    free(j);
}

// reserved for the basic information
char fs_type[5];
char volume_label[10];
unsigned long number_of_sectors;
unsigned long sector_size_in_bytes;
unsigned long number_of_reserved_sectors;
unsigned long number_of_sectors_per_fat_table;
unsigned long number_of_fat_tables;
unsigned long number_of_sectors_per_cluster;
unsigned long number_of_clusters;
unsigned long data_region_starts_at_sector;
unsigned long root_directory_starts_at_sector;
unsigned long root_directory_starts_at_cluster;
unsigned long disk_size_in_bytes;
unsigned long disk_size_in_megabytes;
int number_of_used_clusters;
int number_of_free_clusters;

void get_fat_table_content(int fd, int count)
{
    unsigned char buff[SECTOR_SIZE];
    int is_ended = 0;
    int table_start = 32;
    int linecount = 0;
    //printf("count: %d", count);

        while (is_ended < 2)
        {

            // Read the fat table
            printf("\n");
            readsector(fd, buff, table_start);
            char printable[16] = {'\0'};
            // read the whole content of the boot sector for debugging purposes
            int counter = 0;
            int j = 0;
            printf("000000%x: ", j);
            for (int i = 0; i < SECTOR_SIZE; i++)
            {

                int ctrl = isprint(buff[i]);

                if (ctrl)
                {
                    // append(printable, buff[i]);
                    counter++;
                }
                else
                {
                    // append(printable, '.');
                    counter++;
                }
                if(i % 4 == 0)
                {
                    //printf("i:%d %02x ", i, buff[i] & 0xff);
                    //printf("%02x ", buff[i] & 0xff);
                    if (buff[i] == 0xf8 || buff[i] == 0xff)
                    {
                        printf("EOF");
                    }
                    else
                    {
                        printf("%02d ", buff[i]);
                    }
                    
                }
                if (counter % 4 == 0)
                {
                    printf("\n");
                    j++;
                    if (counter != SECTOR_SIZE)
                    {
                        if (j < 10)
                        {
                            printf("000000%d: ", j);
                            linecount++;
                        }
                        else
                        {
                            printf("00000%d: ", j);
                            linecount++;
                        }
                    }
                }
                if (linecount >= count)
                {
                    is_ended = 1;
                    break;
                }
                
            }
            is_ended++;
            table_start += 1;
            //printf("linecount: %d\n", linecount);
        }
    
}

int calculate_used_clusters(int fd, int table_start)
{
    unsigned char buff[SECTOR_SIZE];
    int is_ended = 0;
    int is_counting = 0;
    int starting_idx = 0;
    int wrong_free = 0;
    int used_clusters = 0;
    int eof_cnt = 0;
    int free_cnt = 0;
    // Read the fat table
    while (is_ended < number_of_sectors_per_fat_table)
    {
        readsector(fd, buff, table_start);

        for (int x = 12; x < SECTOR_SIZE; x += 4)
        {

            if ((buff[x] & 0xff) == 0xff)
            {
                eof_cnt += 1;
            }
            else if (buff[x] != 0x00)
            {
                used_clusters = buff[x];
            }
            else
            {
                if (!is_counting)
                {

                    starting_idx = x / 4;
                    // printf("start: %d\n", starting_idx);
                    is_counting = 1;
                }
                if ((x + 4 < SECTOR_SIZE) & (buff[x + 4] != 0x00) & (is_counting))
                {
                    // printf("hit : %d\n", ((x+4)/4));
                    wrong_free += ((x + 4) / 4 - starting_idx);
                    starting_idx = 0;
                    is_counting = 0;
                }
                free_cnt += 1;
            }
        }
        is_ended++;
        table_start += 1;
        strcpy(buff, "");
    }
    //printf("%d, %d", used_clusters, wrong_free);
    used_clusters -= wrong_free;
    // Adding cluster 2
    used_clusters++;
    return used_clusters;
}

// Prints the file in the format in -l recursively
int print_files_path(int fd, unsigned char *buffer, unsigned int c_num, char *start, char *path)
{
    // Path initially "/"
    // Read the given cluster (initially cluster 2)
    int success;
    success = readcluster(fd, buffer, c_num);
    if (success == -1)
    {
        printf(" Reading error in cluster\n");
        return -1;
    }
    // If the path is "/"
    if (strcmp(path, "/") == 0)
    {
        // Traversing the files and dictionaries
        for (int x = 11; x < CLUSTER_SIZE; x += 32)
        {
            unsigned char name[8];
            unsigned char ext[3];
            char file_type[4] = {'\0'};
            char full_name[12] = {'\0'};
            // Check if the space is empty or not
            if ((buffer[x - 11] != 0xffffffe5) & (buffer[x - 11] != 0xe5) & (buffer[x - 11] != 0x00) & (buffer[x] != 0x08))
            {
                // Initializing the name and extension values
                memcpy(name, &(buffer[x - 11]), 8);
                ext[0] = '\0';
                ext[1] = '\0';
                ext[2] = '\0';

                // Calculating the cluster number of the file or directory
                unsigned int c1 = (buffer[x + 15] + (buffer[x + 16] << 16) + (buffer[x + 9] << 256) + (buffer[x + 10] << 4096)) & 0xff;
                // Calculating the size
                unsigned int s1;
                memcpy(&s1, &(buffer[x + 17]), sizeof(int));

                if (buffer[x] == 0x10)
                {
                    strcpy(file_type, "(d)");
                }
                else if (buffer[x] == 0x20)
                {
                    strcpy(file_type, "(f)");
                    memcpy(ext, &(buffer[x - 3]), 3);
                }

                if (ext[0] != '\0')
                {
                    for (int i = 0; i < 8; i++)
                    {
                        if (name[i] != ' ')
                            append(full_name, name[i]);
                    }
                    append(full_name, '.');
                    for (int i = 0; i < 3; i++)
                    {
                        append(full_name, ext[i]);
                    }
                }
                else
                {
                    for (int i = 0; i < 8; i++)
                    {
                        if (name[i] != ' ')
                            append(full_name, name[i]);
                    }
                }
                printf("%s name=%s\tfcn=%d\tsize(bytes)=%d\t", file_type, full_name, c1, s1);
                print_dates(buffer, x + 11);
            }
        }
        return 0;
    }
    // Else
    else
    {
        for (int x = 11; x < CLUSTER_SIZE; x += 32)
        {
            char parent_path[256] = {'\0'};
            unsigned char name[8];
            strcpy(parent_path, start);
            if (buffer[x - 11] != 0xffffffe5)
            {
                if (buffer[x] == 0x10)
                {
                    // copy the name of the file
                    memcpy(name, &(buffer[x - 11]), 8);

                    if (strcmp(parent_path, "/") == 0)
                    {
                        for (int i = 0; i < 8; i++)
                        {
                            if (name[i] != ' ')
                            {
                                append(parent_path, name[i]);
                            }
                        }
                    }
                    else
                    {
                        append(parent_path, '/');
                        for (int i = 0; i < 8; i++)
                        {
                            if (name[i] != ' ')
                            {
                                append(parent_path, name[i]);
                            }
                        }
                    }

                    if (strcmp(parent_path, path) == 0)
                    {
                        // calculate the cluster number of directory
                        unsigned int c = buffer[x + 15] + (buffer[x + 16] << 16) + (buffer[x + 9] << 256) + (buffer[x + 10] << 4096);
                        unsigned char buf1[CLUSTER_SIZE];

                        // read the cluster
                        readcluster(fd, buf1, c);
                        // print the filenames in the cluster
                        for (int x = 11; x < CLUSTER_SIZE; x += 32)
                        {
                            unsigned char name[8];
                            unsigned char ext[3];
                            char file_type[3] = {'\0'};
                            char full_name[12] = {'\0'};
                            // Check if the space is empty or not
                            if ((buf1[x - 11] != 0xffffffe5) & (buf1[x - 11] != 0xe5) & (buf1[x - 11] != 0x00) & (buf1[x] != 0x08))
                            {
                                // Initializing the name and extension values
                                memcpy(name, &(buf1[x - 11]), 8);
                                ext[0] = '\0';
                                ext[1] = '\0';
                                ext[2] = '\0';

                                // Calculating the cluster number of the file or directory
                                unsigned int c1 = (buf1[x + 15] + (buf1[x + 16] << 16) + (buf1[x + 9] << 256) + (buf1[x + 10] << 4096)) & 0xff;
                                // Calculating the size
                                unsigned int s1;
                                memcpy(&s1, &(buf1[x + 17]), sizeof(int));

                                if (buf1[x] == 0x10)
                                {
                                    strcpy(file_type, "(d)");
                                }
                                else if (buf1[x] == 0x20)
                                {
                                    strcpy(file_type, "(f)");
                                    memcpy(ext, &(buf1[x - 3]), 3);
                                }

                                if (ext[0] != '\0')
                                {
                                    for (int i = 0; i < 8; i++)
                                    {
                                        if (name[i] != ' ')
                                            append(full_name, name[i]);
                                    }
                                    append(full_name, '.');
                                    for (int i = 0; i < 3; i++)
                                    {
                                        append(full_name, ext[i]);
                                    }
                                }
                                else
                                {
                                    for (int i = 0; i < 8; i++)
                                    {
                                        if (name[i] != ' ')
                                            append(full_name, name[i]);
                                    }
                                }
                                printf("%s name=%s\tfcn=%d\tsize(bytes)=%d\t", file_type, full_name, c1, s1);
                                print_dates(buf1, x + 11);
                            }
                            strcpy(file_type, "");
                        }
                        return 0;
                    }
                    else
                    {
                        if (((name[0] == '.') & (name[1] == '.')) | (name[0] == '.'))
                        {
                        }
                        else
                        {
                            // calculate the cluster number of directory
                            unsigned int c = buffer[x + 15] + (buffer[x + 16] << 16) + (buffer[x + 9] << 256) + (buffer[x + 10] << 4096);
                            unsigned char buf1[CLUSTER_SIZE];
                            int success = print_files_path(fd, buf1, c, parent_path, path);
                            if (success == 0)
                            {
                                return 0;
                            }
                        }
                    }
                }
            }
        }
        return -1;
    }
}

// Printing all directories and files
void print_files(int fd, unsigned char *buffer, unsigned int c_num, char *path)
{
    // Path initially "/"
    int success = readcluster(fd, buffer, c_num);
    if (success == -1)
    {
        printf(" Read cluster failed\n");
        return;
    }
    for (int x = 11; x < CLUSTER_SIZE; x += 32)
    {
        char parent_path[256];
        strcpy(parent_path, path);
        if ((buffer[x - 11] != 0xffffffe5) & (buffer[x - 11] != 0xe5))
        {
            unsigned char name[8];
            unsigned char ext[3];
            // copy the name of the file
            memcpy(name, &(buffer[x - 11]), 8);
            // copy the extension of the file
            memcpy(ext, &(buffer[x - 3]), 3);

            if (buffer[x] == 0x20)
            {
                printf("(f) ");
                if (strcmp(parent_path, "/") != 0)
                {
                    printf("%s", parent_path);
                    printf("/");
                    for (int i = 0; i < 8; i++)
                    {
                        if (name[i] != ' ')
                        {
                            printf("%c", name[i]);
                        }
                    }
                }
                else
                {
                    printf("%s", parent_path);
                    for (int i = 0; i < 8; i++)
                    {
                        if (name[i] != ' ')
                        {
                            printf("%c", name[i]);
                        }
                    }
                }
                printf(".");
                for (int i = 0; i < 3; i++)
                {
                    printf("%c", ext[i]);
                }
                printf("\n");
            }
            else if (buffer[x] == 0x10)
            {

                if (((name[0] == '.') & (name[1] == '.')) | (name[0] == '.'))
                {
                    printf("(d) ");
                    if (strcmp(parent_path, "/") != 0)
                    {
                        printf("%s", parent_path);
                        printf("/");
                        for (int i = 0; i < 8; i++)
                        {
                            if (name[i] != ' ')
                            {
                                printf("%c", name[i]);
                            }
                        }
                    }
                    else
                    {
                        printf("%s", parent_path);
                        for (int i = 0; i < 8; i++)
                        {
                            if (name[i] != ' ')
                            {
                                printf("%c", name[i]);
                            }
                        }
                    }
                    printf("\n");
                }
                else
                {
                    unsigned int c = buffer[x + 15] + (buffer[x + 16] << 16) + (buffer[x + 9] << 256) + (buffer[x + 10] << 4096);
                    unsigned char b[CLUSTER_SIZE];
                    printf("(d) ");
                    if (strcmp(parent_path, "/") != 0)
                    {
                        printf("%s", parent_path);
                        printf("/");
                        append(parent_path, '/');
                        for (int i = 0; i < 8; i++)
                        {
                            if (name[i] != ' ')
                            {
                                printf("%c", name[i]);
                                append(parent_path, name[i]);
                            }
                        }
                    }
                    else
                    {
                        printf("%s", parent_path);
                        for (int i = 0; i < 8; i++)
                        {
                            if (name[i] != ' ')
                            {
                                printf("%c", name[i]);
                                append(parent_path, name[i]);
                            }
                        }
                    }

                    printf("\n");
                    print_files(fd, b, c, parent_path);
                }
            }
        }
        // printf("%X ", buffer[x]);
    }
    printf("\n");
}

int readsector(int fd, unsigned char *buf,
               unsigned int snum)
{
    off_t offset;
    int n;
    offset = snum * SECTOR_SIZE;
    lseek(fd, offset, SEEK_SET);
    n = read(fd, buf, SECTOR_SIZE);
    if (n == SECTOR_SIZE)
        return (0);
    else
        return (-1);
}
int readcluster(int fd, unsigned char *buf, unsigned int cnum)
{
    off_t offset;
    int n;
    unsigned int snum; // sector number
    snum = data_region_starts_at_sector + (cnum - 2) * number_of_sectors_per_cluster;
    offset = snum * SECTOR_SIZE;
    lseek(fd, offset, SEEK_SET);
    n = read(fd, buf, CLUSTER_SIZE);
    if (n == CLUSTER_SIZE)
        return (0); // success
    else
        return (-1);
}

// Print number of clusters
int get_cluster_data(int fd, unsigned char *ch, char *begin, char *path, unsigned int c_num)
{
    // Path initially "/"

    unsigned char buffer[CLUSTER_SIZE];
    int success = readcluster(fd, buffer, c_num);
    if (success == -1)
    {
        printf("Cluster Data is not found");
        return 0;
    }
    for (int x = 11; x < CLUSTER_SIZE; x += 32)
    {
        char parent_path[256];
        strcpy(parent_path, begin);
        if (buffer[x - 11] != 0xffffffe5)
        {
            unsigned char name[8];
            unsigned char ext[3];

            // copy the name of the file
            memcpy(name, &(buffer[x - 11]), 8);
            // copy the extension of the file
            memcpy(ext, &(buffer[x - 3]), 3);

            if (buffer[x] == 0x20)
            {
                if (strcmp(parent_path, "/") != 0)
                {
                    append(parent_path, '/');
                    for (int i = 0; i < 8; i++)
                    {
                        if (name[i] != ' ')
                        {
                            append(parent_path, name[i]);
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < 8; i++)
                    {
                        if (name[i] != ' ')
                        {
                            append(parent_path, name[i]);
                        }
                    }
                }
                append(parent_path, '.');
                for (int i = 0; i < 3; i++)
                {
                    append(parent_path, ext[i]);
                }

                if (strcmp(path, parent_path) == 0)
                {

                    unsigned int s1;
                    memcpy(&s1, &(buffer[x + 17]), sizeof(int));
                    unsigned int c = buffer[x + 15] + (buffer[x + 16] << 16) + (buffer[x + 9] << 256) + (buffer[x + 10] << 4096);
                    unsigned int cluster_cnt = s1 / CLUSTER_SIZE;
                    for (int i = 0; i < cluster_cnt; i++)
                    {
                        printf("cindex=%d\t clusternum=%d\n", i, c + i);
                    }
                    return c;
                }
            }
            else if (buffer[x] == 0x10)
            {

                if (((name[0] == '.') & (name[1] == '.')) | (name[0] == '.'))
                {
                }
                else
                {
                    unsigned int c = buffer[x + 15] + (buffer[x + 16] << 16) + (buffer[x + 9] << 256) + (buffer[x + 10] << 4096);
                    unsigned char b[CLUSTER_SIZE];
                    if (strcmp(parent_path, "/") != 0)
                    {
                        append(parent_path, '/');
                        for (int i = 0; i < 8; i++)
                        {
                            if (name[i] != ' ')
                            {
                                append(parent_path, name[i]);
                            }
                        }
                    }
                    else
                    {
                        for (int i = 0; i < 8; i++)
                        {
                            if (name[i] != ' ')
                            {
                                append(parent_path, name[i]);
                            }
                        }
                    }
                    if (strcmp(path, parent_path) == 0)
                    {

                        unsigned int s1;
                        memcpy(&s1, &(buffer[x + 17]), sizeof(int));
                        unsigned int cluster_cnt = s1 / CLUSTER_SIZE;
                        for (int i = 0; i < cluster_cnt; i++)
                        {
                            printf("cindex=%d\t clusternum=%d\n", i, c + i);
                        }
                        return c;
                    }
                    else
                    {

                        unsigned int success = get_cluster_data(fd, b, parent_path, path, c);
                        if (success)
                        {
                            return success;
                        }
                    }
                }
            }
        }
    }
    return 0;
}
// Prints the info about the file or directory with -d
unsigned int get_cluster_metadata(int fd, unsigned char *ch, char *begin, char *path, unsigned int c_num)
{
    // Path initially "/"
    unsigned char buffer[CLUSTER_SIZE];
    int success = readcluster(fd, buffer, c_num);
    if (success == -1)
    {
        printf("Get cluster metadata failed\n");
        return 0;
    }
    for (int x = 11; x < CLUSTER_SIZE; x += 32)
    {
        char parent_path[256];
        strcpy(parent_path, begin);
        if (buffer[x - 11] != 0xffffffe5)
        {
            unsigned char name[8];
            unsigned char ext[3];
            char full_name[11] = {'\0'};
            // copy the name of the file
            memcpy(name, &(buffer[x - 11]), 8);
            // copy the extension of the file
            memcpy(ext, &(buffer[x - 3]), 3);

            if (buffer[x] == 0x20)
            {
                if (strcmp(parent_path, "/") != 0)
                {
                    append(parent_path, '/');
                    for (int i = 0; i < 8; i++)
                    {
                        if (name[i] != ' ')
                        {
                            append(parent_path, name[i]);
                            append(full_name, name[i]);
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < 8; i++)
                    {
                        if (name[i] != ' ')
                        {
                            append(parent_path, name[i]);
                            append(full_name, name[i]);
                        }
                    }
                }
                append(parent_path, '.');
                append(full_name, '.');
                for (int i = 0; i < 3; i++)
                {
                    append(parent_path, ext[i]);
                    append(full_name, ext[i]);
                }
                // printf("%s", parent_path);
                // printf("path: %s parent_path: %s", path, parent_path);
                if (strcmp(path, parent_path) == 0)
                {

                    unsigned int s1;
                    memcpy(&s1, &(buffer[x + 17]), sizeof(int));
                    unsigned int c = buffer[x + 15] + (buffer[x + 16] << 16) + (buffer[x + 9] << 256) + (buffer[x + 10] << 4096);
                    unsigned int cluster_cnt = s1 / CLUSTER_SIZE;

                    printf("name\t     = %s\n", full_name);
                    printf("type\t     = %s\n", "FILE");
                    printf("firstcluster = %d\n", c);
                    printf("clustercount = %d\n", cluster_cnt);
                    printf("size(bytes)  = %d\n", s1);
                    get_date_formatted(buffer, x + 11);

                    return c;
                }
            }
            else if (buffer[x] == 0x10)
            {

                if (((name[0] == '.') & (name[1] == '.')) | (name[0] == '.'))
                {
                }
                else
                {
                    unsigned int c = buffer[x + 15] + (buffer[x + 16] << 16) + (buffer[x + 9] << 256) + (buffer[x + 10] << 4096);
                    unsigned char b[CLUSTER_SIZE];
                    if (strcmp(parent_path, "/") != 0)
                    {
                        append(parent_path, '/');
                        for (int i = 0; i < 8; i++)
                        {
                            if (name[i] != ' ')
                            {
                                append(parent_path, name[i]);
                                append(full_name, name[i]);
                            }
                        }
                    }
                    else
                    {
                        for (int i = 0; i < 8; i++)
                        {
                            if (name[i] != ' ')
                            {
                                append(parent_path, name[i]);
                                append(full_name, name[i]);
                            }
                        }
                    }
                    if (strcmp(path, parent_path) == 0)
                    {

                        unsigned int s1;
                        memcpy(&s1, &(buffer[x + 17]), sizeof(int));
                        unsigned int cluster_cnt = s1 / CLUSTER_SIZE;

                        printf("name\t     = %s\n", full_name);
                        printf("type\t     = %s\n", "FILE");
                        printf("firstcluster = %d\n", c);
                        printf("clustercount = %d\n", cluster_cnt);
                        printf("size(bytes)  = %d\n", s1);
                        get_date_formatted(buffer, x + 11);

                        return c;
                    }
                    else
                    {

                        unsigned int success = get_cluster_metadata(fd, b, parent_path, path, c);
                        if (success)
                        {
                            return success;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

// Returns the cluster number
unsigned int get_cluster_num(int fd, unsigned char *ch, char *begin, char *path, unsigned int c_num)
{
    // Path initially "/"

    unsigned char buffer[CLUSTER_SIZE];
    readcluster(fd, buffer, c_num);
    for (int x = 11; x < CLUSTER_SIZE; x += 32)
    {
        char parent_path[256];
        strcpy(parent_path, begin);
        if (buffer[x - 11] != 0xffffffe5)
        {
            unsigned char name[8];
            unsigned char ext[3];
            // copy the name of the file
            memcpy(name, &(buffer[x - 11]), 8);
            // copy the extension of the file
            memcpy(ext, &(buffer[x - 3]), 3);

            if (buffer[x] == 0x20)
            {
                if (strcmp(parent_path, "/") != 0)
                {
                    append(parent_path, '/');
                    for (int i = 0; i < 8; i++)
                    {
                        if (name[i] != ' ')
                        {
                            append(parent_path, name[i]);
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < 8; i++)
                    {
                        if (name[i] != ' ')
                        {
                            append(parent_path, name[i]);
                        }
                    }
                }
                append(parent_path, '.');
                for (int i = 0; i < 3; i++)
                {
                    append(parent_path, ext[i]);
                }
                // printf("%s", parent_path);
                // printf("path: %s parent_path: %s", path, parent_path);
                if (strcmp(path, parent_path) == 0)
                {

                    // printf("%d", c_num);
                    unsigned int c = buffer[x + 15] + (buffer[x + 16] << 16) + (buffer[x + 9] << 256) + (buffer[x + 10] << 4096);
                    return c;
                }
            }
            else if (buffer[x] == 0x10)
            {

                if (((name[0] == '.') & (name[1] == '.')) | (name[0] == '.'))
                {
                }
                else
                {
                    unsigned int c = buffer[x + 15] + (buffer[x + 16] << 16) + (buffer[x + 9] << 256) + (buffer[x + 10] << 4096);
                    unsigned char b[CLUSTER_SIZE];
                    if (strcmp(parent_path, "/") != 0)
                    {
                        append(parent_path, '/');
                        for (int i = 0; i < 8; i++)
                        {
                            if (name[i] != ' ')
                            {
                                append(parent_path, name[i]);
                            }
                        }
                    }
                    else
                    {
                        for (int i = 0; i < 8; i++)
                        {
                            if (name[i] != ' ')
                            {
                                append(parent_path, name[i]);
                            }
                        }
                    }
                    if (strcmp(path, parent_path) == 0)
                    {

                        return c;
                    }
                    else
                    {

                        unsigned int success = get_cluster_num(fd, b, parent_path, path, c);
                        if (success)
                        {
                            return success;
                        }
                    }
                }
            }
        }
        // printf("%X ", buffer[x]);
    }
    return 0;
}

void calculate_summary(int fd, unsigned char *buffer)
{
    // read the boot sector
    readsector(fd, buffer, 0);

    // copy the filesystem type to the buffer
    for (int x = 82; x < 91; x++)
    {

        append(fs_type, buffer[x]);
    }

    // copy the volume label to the buffer
    for (int x = 71; x < 82; x++)
    {
        // printf("%d : %c \n", x ,buffer[x]);
        append(volume_label, buffer[x]);
    }

    // calculate the number of sectors
    number_of_sectors = buffer[32] + (buffer[33] << 8) + (buffer[34] << 16) + (buffer[35] << 24);

    // calculate the bytes per sector
    sector_size_in_bytes = buffer[12] * 256 + buffer[11];

    // calculate the number of reserved sectors
    number_of_reserved_sectors = buffer[14] + (buffer[15] << 8);

    // calculate the number of sectors per FAT table
    number_of_sectors_per_fat_table = (buffer[36] & 0xff) + (buffer[37] * 256) + (buffer[38] * 4096) + (buffer[39] * 65536);

    // calculate the number of FAT tables
    number_of_fat_tables = buffer[16];

    // calculate the number of sectors per cluster
    number_of_sectors_per_cluster = buffer[13];

    // calculate the number of clusters
    number_of_clusters = number_of_sectors_per_fat_table * SECTOR_SIZE / 4;

    // calculate the data region starts at sector
    data_region_starts_at_sector = number_of_reserved_sectors + (number_of_fat_tables * number_of_sectors_per_fat_table);

    // calculate the root directory starts at sector
    root_directory_starts_at_sector = data_region_starts_at_sector;

    // calculate the root directory starts at cluster
    root_directory_starts_at_cluster = buffer[44] + (buffer[45] << 8) + (buffer[46] << 16) + (buffer[47] << 24);

    // calculate the disk size in bytes
    disk_size_in_bytes = number_of_sectors * sector_size_in_bytes;

    // calculate the disk size in megabytes
    disk_size_in_megabytes = disk_size_in_bytes / 1024 / 1024;

    // calculate the number of used clusters
    number_of_used_clusters = calculate_used_clusters(fd, 32);

    // calculate the number of free clusters
    number_of_free_clusters = number_of_clusters - number_of_used_clusters;
}

int main(int argc, char *argv[])
{
    int fd;
    unsigned char sector_buffer[SECTOR_SIZE];
    unsigned char cluster_buffer[CLUSTER_SIZE];
    char *file_name = argv[1];
    int res, res2;
    char *code = argv[2];

    // open the file
    fd = open(file_name, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Unable to open %s\n", file_name);
        return (1);
    }

    if (code == NULL)
    {
        res2 = 0;
    }
    else
    {
        code = strtok(code, "-");
        // printf("%s\n", code);
        res = strcmp(code, "h");
        res2 = 1;
    }
    if (res == 0 || res2 == 0)
    {
        printf("./fat DISKIMAGE -v:         \t print some summary information about the diskiamge\n ");
        printf("./fat DISKIMAGE -s SECTORNUM: \t print the content of the specified sector to screen in hex form\n");
        printf("./fat DISKIMAGE -c CLUSTERNUM: \t print the content of the specified cluster to the screen in hex form \n");
        printf("./fat DISKIMAGE -t:         \t print all directories and their files and subdirectories starting from the root directory \n");
        printf("./fat DISKIMAGE -r PATH OFFSET COUNT: \t read COUNT bytes from the file indicated with PATH starting at OFFSET and print the bytes read to the screen \n");
        printf("./fat DISKIMAGE -b PATH: \t print the content of the file indicated with PATH to the screen in hex form in the following format.  \n");
        printf("./fat DISKIMAGE -a PATH: \t print the content of the ascii text file indicated with PATH to the screen as it is \n");
        printf("./fat DISKIMAGE -n PATH: \t print the numbers of the clusters storing the content of the file or directory indicated with PATH. \n");
        printf("./fat DISKIMAGE -m COUNT: \t print a map of the volume. \n");
        printf("./fat DISKIMAGE -f COUNT: \t print the content of the FAT table. The first COUNT entries will be printed out. \n");
        printf("./fat DISKIMAGE -d PATH: \t print the content of the directory entry of the file or directory indicated with PATH. \n");
        printf("./fat DISKIMAGE -l PATH: \t print the names of the files and subdirectories in the directory indicated with PATH.  \n");
        printf("./fat DISKIMAGE -h:          \t print a help page showing all 12 options. \n");
    }

    else if (strcmp(code, "v") == 0)
    {

        // Calculate the summary information
        calculate_summary(fd, sector_buffer);

        printf("--- SUMMARY INFORMATION ---\n");
        // print the basic information
        printf("File system type: %s\n", fs_type);
        printf("Volume label: %s\n", volume_label);
        printf("Number of sectors: %ld\n", number_of_sectors);
        printf("Sector size in bytes: %ld\n", sector_size_in_bytes);
        printf("Number of reserved sectors: %ld\n", number_of_reserved_sectors);
        printf("Number of sectors per FAT table: %ld\n", number_of_sectors_per_fat_table);
        printf("Number of FAT tables: %ld\n", number_of_fat_tables);
        printf("Number of sectors per cluster: %ld\n", number_of_sectors_per_cluster);
        printf("Number of clusters: %ld\n", number_of_clusters);
        printf("Data region starts at sector: %ld\n", data_region_starts_at_sector);
        printf("Root directory starts at sector: %ld\n", root_directory_starts_at_sector);
        printf("Root directory starts at cluster: %ld\n", root_directory_starts_at_cluster);
        printf("Disk size in bytes: %ld bytes\n", disk_size_in_bytes);
        printf("Disk size in megabytes: %ld MB\n", disk_size_in_megabytes);

        // REWORK THIS
        printf("Number of used clusters: %ld\n", number_of_used_clusters);
        printf("Number of free clusters: %ld\n", number_of_free_clusters);

        printf("\n");
    }
    else if (strcmp(code, "s") == 0)
    {
        // calculate_summary(fd, sector_buffer);
        int sec_num = atoi(argv[3]);
        readsector(fd, sector_buffer, sec_num);
        printf("\n");
        char printable[16] = {'\0'};
        // read the whole content of the boot sector for debugging purposes
        int counter = 0;
        int j = 0;
        printf("00000%x0: ", j);
        for (int i = 0; i < SECTOR_SIZE; i++)
        {
            int ctrl = isprint(sector_buffer[i]);

            if (ctrl)
            {
                append(printable, sector_buffer[i]);
                counter++;
            }
            else
            {
                append(printable, '.');
                counter++;
            }

            printf("%02x ", sector_buffer[i] & 0xff);
            if (counter % 16 == 0)
            {
                printf("%s", printable);
                strcpy(printable, "");
                printf("\n");
                j++;
                if (counter != SECTOR_SIZE)
                {
                    if (j < 16)
                    {
                        printf("00000%x0: ", j);
                    }
                    else
                    {
                        printf("0000%x0: ", j);
                    }
                }
            }
        }

        close(fd);
    }
    else if (strcmp(code, "c") == 0)
    {

        int clus_num = atoi(argv[3]);
        printf("\n");
        // read the whole content of the boot sector for debugging purposes
        /* for (int x = 0; x < SECTOR_SIZE; x++)
        {
            if (counterr == 16)
            {
                printf("\n");
            }
            printf("%d: %02x ", x, buffer[x] & 0xff);
            counterr++;

        } */

        // Calculate the summary information
        calculate_summary(fd, sector_buffer);

        readcluster(fd, cluster_buffer, clus_num);

        printf("\n");
        char printable[16] = {'\0'};

        // read the whole content of the boot sector for debugging purposes
        int counter = 0;
        int j = 0;
        printf("00000%x0: ", j);
        for (int i = 0; i < CLUSTER_SIZE; i++)
        {
            int ctrl = isprint(cluster_buffer[i]);
            if (ctrl)
            {
                append(printable, cluster_buffer[i]);
                counter++;
            }
            else
            {
                append(printable, '.');
                counter++;
            }

            printf("%02x ", cluster_buffer[i] & 0xff);
            if (counter % 16 == 0)
            {
                printf("%s", printable);
                strcpy(printable, "");
                printf("\n");
                j++;
                if (counter != CLUSTER_SIZE)
                {
                    if (j < 16)
                    {
                        printf("00000%x0: ", j);
                    }
                    else
                    {
                        printf("0000%x0: ", j);
                    }
                }
            }
        }
        close(fd);
    }
    else if (strcmp(code, "t") == 0)
    {

        calculate_summary(fd, sector_buffer);
        char parent[256] = {'/'};
        print_files(fd, cluster_buffer, 2, parent);
    }
    else if (strcmp(code, "a") == 0)
    {
        calculate_summary(fd, sector_buffer);
        unsigned char buff3[CLUSTER_SIZE];

        // take file path from the user
        char *path = argv[3];
        to_upper(path);
        // calculate the which cluster the file is in
        unsigned int clus_num = get_cluster_num(fd, cluster_buffer, "/", path, 2);
        printf("clus_num: %d\n", clus_num);
        // read the cluster
        readcluster(fd, buff3, clus_num);
        // print the cluster
        printf("\n");
        for (int i = 0; i < CLUSTER_SIZE; i++)
        {
            printf("%c", buff3[i]);
        }
    }
    else if (strcmp(code, "b") == 0)
    {
        calculate_summary(fd, sector_buffer);
        unsigned char buff4[CLUSTER_SIZE];

        // take file path from the user
        char *path = argv[3];
        //printf("before path: %s\n", path);
        to_upper(path);


        //printf("after path: %s\n", path);
        // calculate the which cluster the file is in
        unsigned int clus_num = get_cluster_num(fd, cluster_buffer, "/", path, 2);
        //printf("clus_num: %d\n", clus_num);
        // read the cluster
        readcluster(fd, buff4, clus_num);
        // print the cluster

        printf("\n");
        char printable[16] = {'\0'};

        // read the whole content of the boot sector for debugging purposes
        int counter = 0;
        int j = 0;
        printf("00000%x0: ", j);
        for (int i = 0; i < CLUSTER_SIZE; i++)
        {
            int ctrl = isprint(buff4[i]);
            if (ctrl)
            {
                append(printable, buff4[i]);
                counter++;
            }
            else
            {
                // append(printable, '.');
                counter++;
            }

            printf("%02x ", buff4[i] & 0xff);
            if (counter % 16 == 0)
            {
                printf("%s", printable);
                strcpy(printable, "");
                printf("\n");
                j++;
                if (counter != CLUSTER_SIZE)
                {
                    if (j < 16)
                    {
                        printf("00000%x0: ", j);
                    }
                    else
                    {
                        printf("0000%x0: ", j);
                    }
                }
            }
        }
        close(fd);
    }
    else if (strcmp(code, "l") == 0)
    {
        calculate_summary(fd, sector_buffer);
        unsigned char buff5[CLUSTER_SIZE];
        unsigned char buff6[CLUSTER_SIZE];

        // take file path from the user
        char *path = argv[3];
        to_upper(path);
        // calculate the which cluster the file is in
        unsigned int clus_num = get_cluster_num(fd, cluster_buffer, "/", path, 2);
        // copy the cluster to the buffer
        // print the cluster
        printf("clus_num: %d\n", clus_num);
        print_files_path(fd, buff5, 2, "/", path);
        readcluster(fd, buff6, clus_num);

        /*
            The DOS date/time format is a bitmask:

                        24                16                 8                 0
            +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+
            |Y|Y|Y|Y|Y|Y|Y|M| |M|M|M|D|D|D|D|D| |h|h|h|h|h|m|m|m| |m|m|m|s|s|s|s|s|
            +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+
            \___________/\________/\_________/ \________/\____________/\_________/
                year        month       day      hour       minute        second

            The year is stored as an offset from 1980.
            Seconds are stored in two-second increments.
            (So if the "second" value is 15, it actually represents 30 seconds.)

        */
    }
    else if (strcmp(code, "n") == 0)
    {
        char *path = argv[3];
        to_upper(path);

        calculate_summary(fd, sector_buffer);
        get_cluster_data(fd, cluster_buffer, "/", path, 2);
    }
    else if (strcmp(code, "d") == 0)
    {
        char *path = argv[3];
        to_upper(path);

        calculate_summary(fd, sector_buffer);
        get_cluster_metadata(fd, cluster_buffer, "/", path, 2);
    }
    else if (strcmp(code, "f") == 0)
    {
        int count = atoi(argv[3]);
        calculate_summary(fd, sector_buffer);
        get_fat_table_content(fd, count);
    }
    else if (strcmp(code, "r") == 0)
    {
        calculate_summary(fd, sector_buffer);
        unsigned char buff4[CLUSTER_SIZE];

        // take file path from the user
        char *path = argv[3];
        //printf("before path: %s\n", path);
        to_upper(path);

        int off_cnt = atoi(argv[4]);
        int cnt = atoi(argv[5]);

        //printf("after path: %s\n", path);
        // calculate the which cluster the file is in
        unsigned int clus_num = get_cluster_num(fd, cluster_buffer, "/", path, 2);
        //printf("clus_num: %d\n", clus_num);
        // read the cluster
        readcluster(fd, buff4, clus_num);
        // print the cluster

        printf("\n");
        char printable[16] = {'\0'};

        // read the whole content of the boot sector for debugging purposes
        int counter = 0;
        int j = 0;
        printf("00000%x%x: ", j, off_cnt);
        for (int i = off_cnt; i < (off_cnt + cnt); i++)
        {
            int ctrl = isprint(buff4[i]);
            if (ctrl)
            {
                append(printable, buff4[i]);
                counter++;
            }
            else
            {
                // append(printable, '.');
                counter++;
            }

            printf("%02x ", buff4[i] & 0xff);
            if ((counter % 16 == 0) | (i + 1 == off_cnt + cnt))
            {
                printf("%s", printable);
                strcpy(printable, "");
                printf("\n");
                j++;
                if (counter != CLUSTER_SIZE)
                {
                    if (j < 16)
                    {
                        printf("00000%x%x: ", j, off_cnt);
                    }
                    else
                    {
                        printf("0000%x%x: ", j, off_cnt);
                    }
                }
            }
        }
        close(fd);
    }
    else if (strcmp(code, "m") == 0)
    {
        
    }
}
