#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "config.h"

int getopt(int argc, char *const argv[], const char *optstring);

int str_or_num(char *lat_or_lon) {
    int a, is_number = 1;
    for (a=0;a<=strlen(lat_or_lon);a++) {
        if (isdigit(lat_or_lon[a]) || lat_or_lon[a] == '.') {
        } else
            is_number = 0;
    }
    return is_number;
}


int main(int argc, char *argv[]){
    int e = 0, round = 0, printing = 1, item_order = 1;
    int w_option = 0, i_option = 0, n_option = 0, e_option = 0, t_option = 0, p_option = 0, W_option = 0;
    double latitude_num = 0, longitude_num = 0, nearest = 1000000, nearest_e = -1, dist = 0;

    int opt;
    char *warehouse_name_optarg, *warehouse_item_optarg, *item_name_optarg, *price_optarg, *latitude, *longitude;
    char* optstring = "-:w:i:n:e:t:p:W";

    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
            case 'w':
                w_option = 1;
                warehouse_name_optarg = optarg;
                break;
            case 'i':
                i_option = 1;
                warehouse_item_optarg = optarg;
                break;
            case 'n':
                n_option = 1;
                latitude = optarg;
                break;
            case 'e':
                e_option = 1;
                longitude = optarg;
                break;
            case 't':
                t_option = 1;
                item_name_optarg = optarg;
                break;
            case 'p':
                p_option = 1;
                price_optarg = optarg;
                break;
            case 'W':
                W_option = 1;
                break;
            case '?':  // false situation
                return 1;
            case ':':
                return 2;
        }
    }
    if (n_option == 1 && e_option == 1) {  // false situation
        latitude_num = atof(latitude);
        longitude_num = atof(longitude);
        if (latitude_num == 0 || longitude_num == 0) {
            if (str_or_num(latitude) == 0 || str_or_num(longitude) == 0)
                return 4;
        }
        if (latitude_num < LAT_MIN || latitude_num > LAT_MAX)
            return 4;
        if (longitude_num < LON_MIN || longitude_num > LON_MAX)
            return 4;
    } else if (n_option != e_option)
        return 3;


    // warehouses
    while (e<DB_NUM) {
        int items_n = db[e].n, enable_W = 1, enable_items = 0, item_in_warehouse = 0, max_price = 0;
        int g = 0, f = 0;
        round++;

        if (w_option == 1) {  // warehouse name filter
            if (strcmp(db[e].name, warehouse_name_optarg) == 0)
                enable_items = 1;

        } else if (i_option == 1) {  // items in warehouse filter
            for (g=0;g<=db[e].n;g++) {
                if (strcmp(db[e].items[g].name, warehouse_item_optarg) == 0)
                    enable_items = 1;
            }
        } else if (n_option == 1 && e_option == 1) {  // nearest warehouse filter
            GPS optarg_gps;
            optarg_gps.lat = fabs(latitude_num);
            optarg_gps.lon = fabs(longitude_num);
            dist = distance(optarg_gps, db[e].gps);
            if (dist < nearest) {
                nearest = dist;
                nearest_e = e;
            }
            if (e == DB_NUM - 2) {
                enable_items = 1;
                e = nearest_e;
                items_n = db[e].n;
            }
        } else
            enable_items = 1;  // no warehouse filters


        // items
        if (enable_items == 1) {
            for (f=0;f<items_n;f++) {
                printing = 0;

                if (t_option == 0 && p_option == 0)  // no item filters
                    printing = 1;
                else if (t_option == 1) { // item name filter
                    if (strcmp(db[e].items[f].name, item_name_optarg) == 0)
                        printing = 1;
                } else if (p_option == 1) { // item max. price filter
                    max_price = atoi(price_optarg);
                    if (max_price >= db[e].items[f].price)
                        printing = 1;
                }

                // printing
                if (W_option == 1) {
                    if (enable_W == 1) {  // a) printing warehouses
                        item_order = 1;
                        printf("%s %.3lf %.3lf %d :\n", db[e].name, db[e].gps.lat, db[e].gps.lon, db[e].n);
                        enable_W = 0;
                    }
                }
                if (printing == 1) {
                    if (W_option == 0) {  // b) printing through items
                        printf("%d. %s %d : %s %.3lf %.3lf %d\n", item_order, db[e].items[f].name, db[e].items[f].price, db[e].name, db[e].gps.lat, db[e].gps.lon, db[e].n);
                    } else if (W_option == 1) {  // a) printing items through warehouses
                        printf("%d. %s %d\n", item_order, db[e].items[f].name, db[e].items[f].price);
                    }
                    item_order++;
                }
            }
        }
        if (round == 51 && nearest_e != -1) {  // because of GPS filter
            e = DB_NUM + 1;
        }
        e++;
    }
    return 0;
}