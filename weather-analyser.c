/**
 * Weather Analyzer.
 */
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MONTHS 12        // Months in a year
#define MAX_ENTRIES 512  // Maximum years in a file
#define MAX_LIGNE 200

typedef struct
{
    double temperature;
    double precipitations;
} WMonth;

typedef struct
{
    int year;
    WMonth months[MONTHS];
    double temperature;
    double precipitations;
} WYear;

typedef struct
{
    WYear years[MAX_ENTRIES];
    int start;
} WData;

typedef struct
{
    char *in_filename;
    char *out_filename;
    bool binary_output;
} Options;

void version(FILE *fp)
{
    fprintf(fp,
            "Version 0.0.1 "
            "Copyright(c) HEIG-VD\n");
}

void help(FILE *fp)
{
    fprintf(fp,
            "USAGE: ./weather-analyser [options] [filename]\n\n"
            "This program processes weather data issues from the internet. \n"
            "It reads from [filename] or if missing, \n"
            "directly from STDIN.\n\n"
            "The output format is CSV compliant.\n\n"
            "OPTIONS:\n\n"
            "    --version      Shows the version\n"
            "    --help         Shows this help dialog\n\n"
            "    --binary, -b   Output in binary mode, not CSV\n\n"
            "    -o<filename>   Write output on <filename>\n\n");
}

int collect_data(WData *data, FILE *fp) {
    char buffer[MAX_LIGNE] = {0};
    int firstYear = 1;
    for(int i = 0; i < MAX_ENTRIES; i++){
        data->years[i].year = 0;
    }
    while(fgets(buffer, 200, fp) != NULL){
        int dyear;
        int dmonth;
        WMonth month;
        if(sscanf(buffer, "%d\t%d\t%lf\t%lf", &dyear, &dmonth, &month.temperature, &month.precipitations) != 4) continue;
        if(firstYear){
            data->start = dyear;
            firstYear = 0;
        }
        data->years[dyear-data->start].months[dmonth-1] = month;
        data->years[dyear-data->start].year = dyear;
        data->years[dyear-data->start].precipitations += month.precipitations;
        if(dmonth == 12){
            double tempSomme = 0;
            for(int i = 0; i < 12; i++){
                tempSomme += data->years[dyear-data->start].months[i].temperature;
            }
            data->years[dyear-data->start].temperature = tempSomme/12;
        }
    }
    return 0;
}

void process_data(WData *data) {
    for(int i = 0; i < MAX_ENTRIES; i++){
        printf("%d\n", data->years->year);
    }
}

void fprint_csv(FILE *fp, WData *data) {
    fprintf(fp, "year;temperature;precipitations\n");
    for(int i = 0; i < MAX_ENTRIES; i++){
        if(data->years[i].year == 0) return;
        fprintf(fp,"%d;%lf;%lf\n", data->years[i].year, data->years[i].temperature, data->years[i].precipitations);
    }
    return;
}

void fprint_binary(FILE *fp, WData *data) {
    fwrite("WEATHER", 8, 1, fp);
    for(int i = 0; i < MAX_ENTRIES; i++){
        if(data->years[i].year == 0) return;
        fwrite(&data->years[i].year, 4, 1, fp);
        fwrite(&data->years[i].precipitations, 4, 1, fp);
        fwrite(&data->years[i].temperature, 4, 1, fp);
    }
}

void process_arg(int argc, char *argv[])
{
    bool binary = false;
    char* filename = "";
    char outputFilename[256] = {0};

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--version") == 0)
        {
            version(stdout);
            return;
        }
        if (strcmp(argv[i], "--help") == 0)
        {
            help(stdout);
            return;
        }
        if (strcmp(argv[i], "--binary") == 0 || strcmp(argv[i], "-b") == 0)
        {
            binary = true;
            continue;
        }
        if (strcmp(argv[i], "-o") == 0)
        {
            if(i+1 >= argc) exit(2);
            sscanf(argv[i+1], "%s", outputFilename);
            i++;
            continue;
        }

        if(argv[i][0] != '-'){
            filename = argv[i];
            continue;
        }
        
    }
    FILE* fp = stdin;
    if(strcmp(filename, "") != 0){
        fp = fopen(filename, "r");
    }
    if (fp == NULL) exit(1);
    WData data;
    collect_data(&data, fp);
    fclose(fp);
    fp = stdout;
    if(outputFilename[0] != '\0'){
        fp = fopen(outputFilename, "w");
    }
    if (fp == NULL) exit(1);
    if(binary){
        fprint_binary(fp, &data);
        fclose(fp);
        return;
    }
    fprint_csv(fp, &data);
    fclose(fp);
}

int main(int argc, char *argv[])
{
    process_arg(argc, argv);
}
