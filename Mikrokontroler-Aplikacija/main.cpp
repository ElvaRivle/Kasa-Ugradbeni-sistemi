#include "mbed.h"
#include "stm32f413h_discovery_lcd.h"
#include <string>
#include <map>
#include <utility>
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include <iostream>
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"


#define MQTT_CLIENT_NAME "kasa"
#define MQTTCLIENT_QOS2 0
#define POCETNO 0
#define SKENIRANJE 1
#define PLACANJE 2
#define ZAKLJUCI 3
#define UNOS 4
#define BRISANJE 5
#define TEMAKUPOVINA "projekatkasa/kasa/kod"
#define TEMAUNOS "projekatkasa/kasa/unos"
#define TEMABRISANJE "projekatkasa/kasa/brisanje"


TS_StateTypeDef TS_State = { 0 };
char mod=POCETNO;
InterruptIn btn0(BUTTON1);
unsigned int total=0;
std::string poruka1="Ukupno: ";
std::string poruka2="Uplatiti: ";
std::string poruka3="Obrisali ste: ";
std::map<std::string,std::pair<std::string,int> > artikli;
std::map<std::string,std::pair<std::string,int> >::iterator it;

int arrivedcount = 0;
std::string str;

enum class Stanje {
    Pocetno,
    Skeniranje,
    Placanje,
    Zakljucivanje,
    Unos,
    Brisanje
};

typedef struct Artikal {
    std::string barkod, naziv;
    double cijena = 0.;
    
    Artikal() {
        barkod = "";
        naziv = "";
    }
    
    Artikal(std::string _barkod, std::string _naziv, double _cijena) {
        barkod = _barkod;
        naziv = _naziv;
        cijena = _cijena;
    }
    
    ~Artikal() {
        barkod = naziv = "";
        cijena = 0.;
    }
    
} Artikal;

class Kasa {
    Stanje aktivnoStanje = Stanje::Pocetno;
    Artikal *artikli = new Artikal[100];

    Kasa(){}
    
public:
    static Kasa& daj_instancu() {
        static Kasa singleton = Kasa();
        return singleton;
    }

    friend void mqtt_stigao_artikal(MQTT::MessageData&);

    ~Kasa() {
        delete[] artikli;
        artikli = nullptr;
    }
};

void pocetno(){
    mod=POCETNO;
    total=0;

   BSP_LCD_Clear(LCD_COLOR_WHITE);

    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 15, (uint8_t *)"Odaberite opciju", CENTER_MODE);
    
    
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillRect(0, 40, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 55, (uint8_t *)"Kupovina", CENTER_MODE);
    
    
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_FillRect(0, 80, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_RED);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 95, (uint8_t *)"Brisanje artikla", CENTER_MODE);
    
    BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA);
    BSP_LCD_FillRect(0, 120, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_MAGENTA);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 135, (uint8_t *)"Unos artikla", CENTER_MODE);
    
    
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 180, (uint8_t *)"US Projekat", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 210, (uint8_t *)"Elvir - Vedad - Tarik", CENTER_MODE);
}

void kupi(){
    if(mod!=SKENIRANJE && mod!=PLACANJE && mod!=ZAKLJUCI){
        total=0;
        mod=SKENIRANJE;
        poruka1="Ukupno: " + std::to_string(total)+ " KM";
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Kupovina", CENTER_MODE);
        BSP_LCD_DisplayStringAt(0, 60,(uint8_t *)poruka1.c_str(), LEFT_MODE);
        std::cout<<"Dostupni: "<<std::endl;
        for(std::map<std::string,std::pair<std::string,int> >::iterator it=artikli.begin();it!=artikli.end();it++){
            std::cout<<(*it).first<<std::endl;
        }
        
    

    }
    else if(mod==SKENIRANJE){
        mod=PLACANJE;
        poruka2+=std::to_string(total);
        poruka2+=" KM";
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Zakljucivanje racuna", CENTER_MODE);
        BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)poruka2.c_str(), CENTER_MODE);
        poruka2="Uplatiti ";
    }
    else if(mod==PLACANJE){
        mod=POCETNO;
        kupi();
    }
}

void unesi(){
    if(mod!=UNOS){
        mod=UNOS;
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_MAGENTA);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Unos artikala", CENTER_MODE);
    }
    else{
        pocetno();
    }

}

void obrisi(){
    if(mod!=BRISANJE){
        mod=BRISANJE;
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_RED);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Brisanje artikla", CENTER_MODE);
    }
    else{
        pocetno();
    }
}

void messageArrived_kodKupovina(MQTT::MessageData& md)
{
    if(mod==SKENIRANJE){
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Kupovina", CENTER_MODE);
        MQTT::Message& message = md.message;
        printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
        printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
        ++arrivedcount;
        str=(char*)message.payload;
        str.resize(message.payloadlen);

        it=artikli.find(str);

        if(it!=artikli.end()){
                std::string art=(*it).second.first+", cijena: "+std::to_string((*it).second.second)+" KM";
                BSP_LCD_DisplayStringAt(0, 30, (uint8_t *) art.c_str(), CENTER_MODE);
                total+=(*it).second.second;
                poruka1="Ukupno: " + std::to_string(total)+ " KM";
        }
        else{
            BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)"Nepostojeci artikal", CENTER_MODE);
        }
        BSP_LCD_DisplayStringAt(0, 60,(uint8_t *)poruka1.c_str(), LEFT_MODE);
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_FillRect(0, BSP_LCD_GetYSize()-40, BSP_LCD_GetXSize(), 40);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
        BSP_LCD_SetFont(&Font16);
        BSP_LCD_DisplayStringAt(0, 215, (uint8_t *)"Zakljuci racun", CENTER_MODE);
    }
}

void mqtt_stigao_artikal(MQTT::MessageData& md)
{
    Kasa kasa = Kasa::daj_instancu();
    kasa.aktivnoStanje = Stanje::Pocetno;
    if(mod==UNOS){
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_MAGENTA);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Unos artikala", CENTER_MODE);
        MQTT::Message& message = md.message;
        printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
        printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
        ++arrivedcount;
        str=(char*)message.payload;
        str.resize(message.payloadlen);
        std::string kod;
        std::string naziv;
        int cijena;
        int i=0;
        while(str[i]!=',') kod+=str[i++];
        i++;
        while(str[i]!=',') naziv+=str[i++];
        i++;
        cijena = stoi(str.substr(i));

        BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)"Unijeli ste artikal:", CENTER_MODE);
        BSP_LCD_DisplayStringAt(0, 50, (uint8_t *)naziv.c_str(), CENTER_MODE);
        artikli[kod]=std::pair<std::string,int>(naziv,cijena);

    }
}

void messageArrived_kodBrisanje(MQTT::MessageData& md)
{
    if(mod==BRISANJE){
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_RED);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Brisanje artikla", CENTER_MODE);
        MQTT::Message& message = md.message;
        printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
        printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
        ++arrivedcount;
        str=(char*)message.payload;
        str.resize(message.payloadlen);
        it=artikli.find(str);

        if(it!=artikli.end()){
            BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)"Obrisali ste artikal:", CENTER_MODE);
            BSP_LCD_DisplayStringAt(0, 50, (uint8_t *)(*it).second.first.c_str(), CENTER_MODE);
            artikli.erase(str);
        }
        else{
            BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)"Nepostojeci artikal", CENTER_MODE);
        }
    }

}

int main() {

    Kasa kasa = Kasa::daj_instancu();

    NetworkInterface *network;
    network = NetworkInterface::get_default_instance();

    if (!network) {
        return -1;
    }
    MQTTNetwork mqttNetwork(network);

    BSP_LCD_Init();

    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    const char* hostname = "broker.hivemq.com";
    int port = 1883;
    printf("Connecting to %s:%d\r\n", hostname, port);
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)
        printf("rc from TCP connect is %d\r\n", rc);
    else
        printf("Connected to broker!\r\n");

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = MQTT_CLIENT_NAME;
    data.username.cstring = "";
    data.password.cstring = "";
    if ((rc = client.connect(data)) != 0)
        printf("rc from MQTT connect is %d\r\n", rc);

    if ((rc = client.subscribe(TEMAKUPOVINA, MQTT::QOS0, messageArrived_kodKupovina)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    else
        printf("Subscribed to %s\r\n", TEMAKUPOVINA);


    if ((rc = client.subscribe(TEMAUNOS, MQTT::QOS0, mqtt_stigao_artikal)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    else
        printf("Subscribed to %s\r\n", TEMAUNOS);

    if ((rc = client.subscribe(TEMABRISANJE, MQTT::QOS0, messageArrived_kodBrisanje)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    else
        printf("Subscribed to %s\r\n", TEMABRISANJE);

    pocetno();
    btn0.rise(&pocetno);

    while (1) {
        rc = client.subscribe(TEMAKUPOVINA, MQTT::QOS0, messageArrived_kodKupovina);
        rc = client.subscribe(TEMAUNOS, MQTT::QOS0, mqtt_stigao_artikal);
        rc = client.subscribe(TEMABRISANJE, MQTT::QOS0, messageArrived_kodBrisanje);
        
        if(mod==POCETNO) {
           BSP_TS_GetState(&TS_State);
            if(TS_State.touchDetected) {
                
                uint16_t x1 = TS_State.touchX[0];
                uint16_t y1 = TS_State.touchY[0];
    
                if((unsigned int)y1<=80 && (unsigned int)y1>40) 
                    kupi();
                if((unsigned int)y1>80 && (unsigned int)y1<=120) 
                    obrisi();
                if((unsigned int)y1>120 && (unsigned int)y1<=160) 
                    unesi();
                
            }

        }
        else if(mod==SKENIRANJE) {
            BSP_TS_GetState(&TS_State);
            uint16_t x1 = TS_State.touchX[0];
            uint16_t y1 = TS_State.touchY[0];
            if(TS_State.touchDetected) 
                if((unsigned int)y1>=BSP_LCD_GetYSize()-40 && (unsigned int)y1<BSP_LCD_GetYSize()) 
                    kupi();
        }
        wait_ms(1);
    }
}
