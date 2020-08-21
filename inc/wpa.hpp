#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>

extern "C" {
#include "wpa_ctrl.h"
}   

using namespace std;

#define MAX_SIZE 40
#define INTERFACE  "wlan0"
#define CTRL_IFACE "/var/run/wpa_supplicant/wlan0"

static int parse_mess(char *buff, int &flag);
static void parse_to_wifi_name(char *buff);
static int wpa_ctrl_cmd(struct wpa_ctrl *ctrl, char *cmd, char *buf);
static void wpa_recv_pending(struct wpa_ctrl *ctrl, int &flag);

class Wpa
{
    private:
        int flag;
        char *ssid;
        char *psk;
        struct wpa_ctrl *ctrl_conn;
    public:
        Wpa();
        void usage(void);
        int scan_results(void);
        int config_wifi(int, int, char **);
};