#include "mbed.h"
#include "stm32f413h_discovery_lcd.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"


#include <vector>
#include <string>


#define MQTT_CLIENT_NAME "US_Projekat_Kasa_MBED"
#define TEMAKUPOVINA "projekatkasa/kasa/kod"
#define TEMAUNOS "projekatkasa/kasa/unos"
#define TEMABRISANJE "projekatkasa/kasa/brisanje"

enum Stanja {
    POCETNO = 0,
    SKENIRANJE,
    PLACANJE,
    UNOS,
    BRISANJE
};


TS_StateTypeDef TS_State = { 0 };
InterruptIn btn0(BUTTON1);

float iznosRacuna=0;
char trenutnoStanje=POCETNO;

typedef struct Artikal {
    std::string barkod, naziv;
    float cijena;
    
    Artikal(std::string _barkod = "", std::string _naziv = "", float _cijena = 0.) {
        barkod = _barkod;
        naziv = _naziv;
        cijena = _cijena;
    }
    
} Artikal;

std::vector<Artikal> sviArtikli;

void pocetno_stanje(){
    trenutnoStanje=POCETNO;
    iznosRacuna=0;

    BSP_LCD_Clear(LCD_COLOR_WHITE);

    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 15, (uint8_t *)"Odaberite opciju", CENTER_MODE);
    
    
    
    
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillRect(0, 40, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 55, (uint8_t *)"Kupovina", CENTER_MODE);
    
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DrawHLine(0,40,BSP_LCD_GetXSize());
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_FillRect(0, 80, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_RED);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 95, (uint8_t *)"Brisanje artikla", CENTER_MODE);
    
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DrawHLine(0,80,BSP_LCD_GetXSize());
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    
    BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA);
    BSP_LCD_FillRect(0, 120, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_MAGENTA);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 135, (uint8_t *)"Unos artikla", CENTER_MODE);
    
    
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DrawHLine(0,120,BSP_LCD_GetXSize());
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    
    
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 180, (uint8_t *)"US Projekat", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 210, (uint8_t *)"Elvir - Vedad - Tarik", CENTER_MODE);
}

void kupovina_stanje(){
    if(trenutnoStanje == POCETNO){
        iznosRacuna=0;
        trenutnoStanje=SKENIRANJE;
        std::string temp="Ukupno: " + std::to_string(iznosRacuna)+ " KM";
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Kupovina", CENTER_MODE);
        BSP_LCD_DisplayStringAt(0, 60,(uint8_t *)temp.c_str(), LEFT_MODE);
        printf("Dostupni: \n");
        for(char i = 0; i < sviArtikli.size(); ++i){
            printf("%s\n", sviArtikli.at(i).barkod.c_str());
        }
    }
}

void placanje_stanje() {
    if(trenutnoStanje==SKENIRANJE){
        trenutnoStanje=PLACANJE;
        std::string temp = "Uplatiti: " + std::to_string(iznosRacuna);
        temp += " KM";
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Zakljucivanje racuna", CENTER_MODE);
        BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)temp.c_str(), CENTER_MODE);
    }
}

void unos_stanje(){
    if(trenutnoStanje != UNOS){
        trenutnoStanje=UNOS;
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_MAGENTA);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Unos artikala", CENTER_MODE);
    }
}

void brisanje_stanje(){
    if(trenutnoStanje!=BRISANJE){
        trenutnoStanje=BRISANJE;
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_RED);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Brisanje artikla", CENTER_MODE);
    }
}

void mqtt_stigao_skenirani_artikal(MQTT::MessageData& md)
{
    if(trenutnoStanje==SKENIRANJE){
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Kupovina", CENTER_MODE);
        MQTT::Message& message = md.message;

        std::string payload = (char*)message.payload;
        payload.resize(message.payloadlen);
        
        bool nadjen = false;
        
        std::string ispis;
        
        for (char i = 0; i < sviArtikli.size(); ++i) {
            Artikal temp = sviArtikli.at(i);
            if (payload == temp.barkod) {
                nadjen = true;
                std::string art=temp.naziv+", cijena: "+std::to_string(temp.cijena)+" KM";
                BSP_LCD_DisplayStringAt(0, 30, (uint8_t *) art.c_str(), CENTER_MODE);
                iznosRacuna+=sviArtikli.at(i).cijena;
                ispis = "Ukupno: " + std::to_string(iznosRacuna)+ " KM";
                break;
            }
        }
        
        if (!nadjen) {
            BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)"Nepostojeci artikal", CENTER_MODE);
        }
        
        BSP_LCD_DisplayStringAt(0, 60,(uint8_t *)ispis.c_str(), LEFT_MODE);
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_FillRect(0, BSP_LCD_GetYSize()-40, BSP_LCD_GetXSize(), 40);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
        BSP_LCD_SetFont(&Font16);
        BSP_LCD_DisplayStringAt(0, 215, (uint8_t *)"Zakljuci racun", CENTER_MODE);
    }
}

void mqtt_stigao_novi_artikal(MQTT::MessageData& md)
{
    if(trenutnoStanje==UNOS){
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_MAGENTA);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Unos artikala", CENTER_MODE);
        MQTT::Message& message = md.message;

        std::string payload =(char*)message.payload;
        payload.resize(message.payloadlen);
        std::string kod;
        std::string naziv;
        int cijena;
        int i=0;
        while(payload[i]!=',') kod+=payload[i++];
        i++;
        while(payload[i]!=',') naziv+=payload[i++];
        i++;
        cijena = stof(payload.substr(i));
        
        bool vecPostoji = false;
        
        for (char i = 0; i < sviArtikli.size(); ++i) {
            if (sviArtikli.at(i).barkod == kod) {
                vecPostoji = true;
                break;
            }
        }
        
        if (!vecPostoji) {
            Artikal noviArtikal;
            noviArtikal.barkod = kod;
            noviArtikal.naziv = naziv;
            noviArtikal.cijena = cijena;
            
            sviArtikli.push_back(noviArtikal);
        }
        
        BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)"Unijeli ste artikal:", CENTER_MODE);
        BSP_LCD_DisplayStringAt(0, 50, (uint8_t *)naziv.c_str(), CENTER_MODE);
    }
}

void mqtt_stigao_barkod_za_brisanje(MQTT::MessageData& md)
{
    if(trenutnoStanje==BRISANJE){
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_RED);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Brisanje artikla", CENTER_MODE);
        MQTT::Message& message = md.message;

        std::string payload=(char*)message.payload;
        payload.resize(message.payloadlen);
        
        char offset = 0;
        bool postoji = false;
        
        for (; offset < sviArtikli.size(); ++offset) {
            if (sviArtikli.at(offset).barkod == payload) {
                postoji = true;
                break;
            }
        }
        
        if (postoji) {
            BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)"Obrisali ste artikal:", CENTER_MODE);
            BSP_LCD_DisplayStringAt(0, 50, (uint8_t *)(*(sviArtikli.begin() + offset)).naziv.c_str(), CENTER_MODE);
            sviArtikli.erase(sviArtikli.begin() + offset);
        }
        else{
            BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)"Nepostojeci artikal", CENTER_MODE);
        }
    }

}

int main() {

    NetworkInterface *network;
    network = NetworkInterface::get_default_instance();

    if (!network) {
        return -1;
    }
    MQTTNetwork mqttNetwork(network);

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

    if ((rc = client.subscribe(TEMAKUPOVINA, MQTT::QOS0, mqtt_stigao_skenirani_artikal)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    else
        printf("Subscribed to %s\r\n", TEMAKUPOVINA);


    if ((rc = client.subscribe(TEMAUNOS, MQTT::QOS0, mqtt_stigao_novi_artikal)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    else
        printf("Subscribed to %s\r\n", TEMAUNOS);

    if ((rc = client.subscribe(TEMABRISANJE, MQTT::QOS0, mqtt_stigao_barkod_za_brisanje)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    else
        printf("Subscribed to %s\r\n", TEMABRISANJE);
        
        
        
    BSP_LCD_Init();
    

    pocetno_stanje();
    btn0.fall(&pocetno_stanje);

    while (1) {
        rc = client.subscribe(TEMAKUPOVINA, MQTT::QOS0, mqtt_stigao_skenirani_artikal);
        rc = client.subscribe(TEMAUNOS, MQTT::QOS0, mqtt_stigao_novi_artikal);
        rc = client.subscribe(TEMABRISANJE, MQTT::QOS0, mqtt_stigao_barkod_za_brisanje);
        
        if(trenutnoStanje==POCETNO) {
           BSP_TS_GetState(&TS_State);
            if(TS_State.touchDetected) {
                
                uint16_t x1 = TS_State.touchX[0];
                uint16_t y1 = TS_State.touchY[0];
    
                if((unsigned int)y1<=80 && (unsigned int)y1>40) 
                    kupovina_stanje();
                if((unsigned int)y1>80 && (unsigned int)y1<=120) 
                    brisanje_stanje();
                if((unsigned int)y1>120 && (unsigned int)y1<=160) 
                    unos_stanje();
                
            }

        }
        else if(trenutnoStanje==SKENIRANJE) {
            BSP_TS_GetState(&TS_State);
            uint16_t x1 = TS_State.touchX[0];
            uint16_t y1 = TS_State.touchY[0];
            if(TS_State.touchDetected) 
                if((unsigned int)y1>=BSP_LCD_GetYSize()-40 && (unsigned int)y1<BSP_LCD_GetYSize()) 
                    placanje_stanje();
        }
        wait_ms(1);
    }
}
