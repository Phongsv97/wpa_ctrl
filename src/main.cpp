#include "wpa.hpp"

int main(int argc, char *argv[])
{
    int opt;
    struct Wpa myWpa;

    for(;;) {
        opt = getopt(argc, argv, "hsdcu");
        if (opt < 0)
            break;
        switch (opt) {
            case 'c':
                myWpa.config_wifi(optind, argc, argv);
                break;
            case 'd':
                break;
            case 'h':
                myWpa.usage();
                return 0;
            case 'u':
                break;
            case 's':
                myWpa.scan_results();
                break;
            case '?':
                cout << "unknow option: " << (char)optopt << endl;
                break;
            default:
                myWpa.usage();
                return 1;
        }
    }
}