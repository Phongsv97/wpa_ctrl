#include "wpa.hpp"

Wpa::Wpa()
{
    this->ctrl_conn = wpa_ctrl_open(CTRL_IFACE);
    if (!this->ctrl_conn)
        printf("Could not get ctrl interface!\n");
}

void Wpa::usage(void)
{
    cout << "usage: sudo wpa_ctrl [-h ] [-s ] [-d  ] [-u ] [-c <ssid> <psk> ]" << endl;
    cout << "list option:" << endl;
    cout << "  -h = help (show this usage text)"  << endl;
    cout << "  -s = scan (scan all network)"      << endl;
    cout << "  -c = connect (connect to wifi)"    << endl;
    cout << "       <ssid>: wifi name " << endl;
    cout << "       <psk> : password"   << endl;
    cout << "  -d = disconnect (wifi down)" << endl;
    cout << "  -u = up (wifi up)"  << endl;
    cout << "ex: sudo wpa_ctrl -c \"ALL LUMI\" lumivn274" << endl;
    cout << "default etc path: /etc/wpa_supplicant/wpa_supplicant.conf" << endl;
}

static int wpa_ctrl_cmd(struct wpa_ctrl *ctrl, char *cmd, char *buf)
{
    int ret;
    size_t len = 4096;

    ret = wpa_ctrl_request(ctrl, cmd, strlen(cmd), buf, &len, NULL);
    if (ret == -2) {
        printf("'%s' command timed out.\n", cmd);
            return -2;
    } else if (ret < 0) {
        printf("'%s' command failed.\n", cmd);
        return -1;
    }
    buf[len -1] = '\0';

    return 0;
}

static void parse_to_wifi_name(char *buff)
{
    int r_size = 0;
    string myStr(buff), val, line;
    stringstream ss(myStr);
    vector<vector<string>> array;

    while (getline(ss, line, '\n')) {
        vector<string> row;
        stringstream s(line);
        while (getline(s, val, '\t')) {
            row.push_back (val);      
        }
        array.push_back (row);
        r_size++;   
    }

    for (int i = 1; i < r_size -1;i++) {
        if (array[i].size() != 5)
            continue;
        cout << '\t' << array[i][4] << endl;
    }         
}

static int parse_mess(char *buff)
{
    string myStr(buff), val;
    stringstream ss(myStr);

    while (getline(ss, val, ' ')) {
        if (!val.compare("reason=CONN_FAILED")) {
           cout << "Connect failed" << endl;
           cout << "Please check your wifi password" << endl;
           return -1;
        }

       if (!val.compare("<3>CTRL-EVENT-NETWORK-NOT-FOUND")) {
           cout << "Connect failed" << endl;
           cout << "Please check your wifi name" << endl;
           return -1;
       }

       if (!val.compare("<3>CTRL-EVENT-CONNECTED")) {
           cout << "Connect successfully" << endl;
           return -1;
       }
    }

    return 0;
}

int Wpa::scan_results(void)
{   
    int ret;
    char *buff = new char[4096];
    char *OK = new char[2];

    this->ctrl_conn = wpa_ctrl_open(CTRL_IFACE);
    if (!this->ctrl_conn){
        printf("Could not get ctrl interface!\n");
        return -1;
    }

    if ((ret = wpa_ctrl_cmd(this->ctrl_conn, "SCAN", OK)) < 0)
        return -1;
    else
        cout << "scan wifi status: " << OK << endl;

    if ((ret = wpa_ctrl_cmd(this->ctrl_conn, "SCAN_RESULTS", buff)) < 0) 
        return -1;
    else
        cout << "List SSID: " << endl;
    
    parse_to_wifi_name(buff);

    delete[] buff;
    delete[] OK;

    return 0;
}

static void wpa_recv_pending(struct wpa_ctrl *ctrl, int &flag)
{
    int ret;

    while (wpa_ctrl_pending(ctrl) > 0) {
        char mess[2048];
        size_t len = 2047;
        if (wpa_ctrl_recv(ctrl, mess, &len) == 0) {
            mess[len] = '\0';
            ret = parse_mess(mess);
            if (ret < 0)
                flag = 0;
        } else {
            cout << "Could not read pending message." << endl;
            break;
        }
    }

    if (wpa_ctrl_pending(ctrl) < 0) {
        cout << "Connection to wpa_supplicant lost - trying to reconnect" << endl;
    }
}

int Wpa::config_wifi(int sPoint, int ePoint, char *argv[])
{
    int val;
    fstream fp;
    char *pos;
    char *err = new char[2048];
    char *buff = new char[64];
    char *OK = new char[2];

    val = ePoint - sPoint;
    if (val < 1) {
        cout << "ex: sudo wpa_ctrl -c \"ALL LUMI\" lumivn274" << endl;
        return -1;
    }
    
    this->ssid = argv[sPoint];

    if (val > 1) {
        this->psk = argv[sPoint + 1];
    } else {
        cout << "Reading psk from stdin" << endl;
        if (fgets(buff, sizeof(buff), stdin) == NULL) {
            cout << "Failed to read psk from stdin" << endl;
            return -1;
        }
        buff[sizeof(buff) -1] = '\0';
        pos = buff;
        while (*pos != '\0') {
            if (*pos == '\r' || *pos == '\n') {
                *pos = '\0';
                break;
            }
            pos++;
        }
        this->psk = buff;
    }
    
    if (strlen(this->psk) < 8 || strlen(this->psk)> 63) {
        cout << "Size: " << strlen(this->psk) << endl << "Psk must be 8...63 characters\n" <<endl;
        return -1;
    }
	
    fp.open("/etc/wpa_supplicant/wpa_supplicant.conf", ios::out);
    if(!fp.is_open()) {
        cout << "error while opening file /etc/wpa_supplicant/wpa_supplicant.conf" << endl;
        return -1;
    }

    fp << "ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev" << endl;
    fp << "update_config=1" << endl << "country=VN" << endl << endl;
    fp << "network={" << endl;
    fp << "\tssid=\"" << this->ssid << "\"" << endl;
    fp << "\tpsk=\"" << this->psk << "\"" << endl;
    fp << "}\n" << endl;
    fp.close();

    if ((wpa_ctrl_cmd(this->ctrl_conn, "RECONFIGURE", OK)) < 0) 
        return -1;
    else { 
        cout << "Config wifi " << this->ssid << " status: " << OK << endl;
        cout << "Wait a few seconds ... " << endl;
    }

    if (wpa_ctrl_attach(ctrl_conn) < 0) {
            cout << "Warning: Failed to attach to wpa_supplicant." << endl;
            return -1;
    }

    this->flag = 1;

    while(this->flag) {
        wpa_recv_pending(this->ctrl_conn, this->flag);
    };
    
    delete[] buff;
    delete[] OK;
    delete[] err;

    return 0;
}
